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

                if (values[0] == "REM") {
                    cue.remarks.emplace_back(parseRemark(values));
                }
            }

        }

    private:
        CueRemark parseRemark(std::vector<std::string> values) noexcept
        {
            assert(values.size() == 3);
            assert(values[0] == "REM");
            const auto& type = values[1];
            const auto& value = values[2];
            if (type == "GENRE") {
                return { .type = CueRemarkType::GENRE, .value = value };
            }
            else if (type == "DATE") {
                return { .type = CueRemarkType::DATE, .value = value };
            }
            else if (type == "DISCID") {
                return { .type = CueRemarkType::DISCID, .value = value };
            }
            else if (type == "COMMENT") {
                return { .type = CueRemarkType::COMMENT, .value = value };
            }
            else if (type == "COMPOSER") {
                return { .type = CueRemarkType::COMPOSER, .value = value };
            }
            return { .type = CueRemarkType::UNKNOWN, .value = value };
        }

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
