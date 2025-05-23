#pragma once
#include <array>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include "Cue.h"

namespace RagiMagick2::Audio::Wav
{
    class CueParser final
    {
    public:
        CueParser(std::string_view fileName) noexcept
            : m_FileName(fileName)
        {
            m_Reader = std::ifstream(fileName.data(), std::ios_base::binary);
        }

        void parse() noexcept
        {
            assert(m_Reader);

            std::array<uint8_t, 3> bom;
            m_Reader.read(reinterpret_cast<char*>(bom.data()), bom.size());
            if (bom != std::array<uint8_t, 3>{ 0xef, 0xbb, 0xbf }) {
                m_Reader.seekg(0, std::ios_base::beg);
            }

            Cue cue{};
            std::string line;
            while (std::getline(m_Reader, line)) {
                auto&& values = parseLine(line);

                if (values.size() < 2) {
                    continue;
                }

                auto&& value0 = values[0];
                auto&& value1 = values[1];

                if (value0 == "REM") {
                    if (value1 == "GENRE" && values.size() == 3) {
                        cue.genre = values[2];
                    }
                    else if (value1 == "DATE" && values.size() == 3) {
                        cue.date = values[2];
                    }
                    else if (value1 == "DISCID" && values.size() == 3) {
                        cue.discID = values[2];
                    }
                    else if (value1 == "COMMENT" && values.size() == 3) {
                        cue.comment = values[2];
                    }
                }
            }

            std::cout << cue.genre << std::endl;
            std::cout << cue.date << std::endl;
            std::cout << cue.discID << std::endl;
            std::cout << cue.comment << std::endl;
        }

    private:
        std::vector<std::string> parseLine(std::string_view line) noexcept
        {
            const std::string_view::const_pointer head = line.data();
            const std::string_view::const_pointer tail = head + line.size();
            std::string_view::const_pointer ptr = head;
            std::string_view::const_pointer startPtr = head;
            std::string_view::const_pointer endPtr = nullptr;

            std::vector<std::string> fields;
            bool beginTrim = true;
            bool isQuotedValue = false;

            while (ptr < tail) {
                if (*ptr == ' ' && beginTrim) {
                    ++ptr;
                    startPtr = ptr;
                    continue;
                }
                else {
                    beginTrim = false;
                }

                if (*ptr == '\r') {
                    if (auto next = ptr + 1; next < tail && *next == '\n') {
                        ++ptr;
                        continue;
                    }
                }

                switch (*ptr) {
                case '\r':
                case '\n':
                    if (ptr != startPtr) {
                        endPtr = ptr - 1;
                    }
                    break;
                case '\"':
                    isQuotedValue = !isQuotedValue;
                    if (isQuotedValue) {
                        startPtr = ptr;
                    }
                    else {
                        endPtr = ptr;
                    }
                    break;
                case ' ':
                    if (!isQuotedValue) {
                        endPtr = ptr - 1;
                    }
                    break;
                default:
                    break;
                }

                if (startPtr && endPtr) {
                    fields.emplace_back(line.substr(startPtr - head, (endPtr - startPtr) + 1));
                    startPtr = ptr + 1;
                    endPtr = nullptr;
                    beginTrim = true;
                }

                ++ptr;
            }

            return fields;
        }

    private:
        std::string_view m_FileName;
        std::ifstream m_Reader;
    };

}
