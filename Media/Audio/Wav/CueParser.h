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
            auto lastCommand = CueCommand::UNKNOWN;
            bool isHeaderParsed = false;

            while (std::getline(m_Reader, line)) {
                auto&& values = parseLine(line);

                if (values.size() < 2) {
                    continue;
                }

                const auto& command = parseCommand(values[0]);

               switch (command) {
               case CueCommand::REM:
                   if (isHeaderParsed) {
                       assert(cue.tracks.size() > 0);
                       auto& lastTrack = cue.tracks[cue.tracks.size() - 1];
                       lastTrack.remarks.emplace_back(parseRemark(values));
                   }
                   else {
                       cue.remarks.emplace_back(parseRemark(values));
                   }
                   break;
               case CueCommand::TITLE:
                   if (isHeaderParsed) {
                       auto& lastTrack = cue.tracks[cue.tracks.size() - 1];
                       lastTrack.title = values[1];
                   }
                   else {
                       cue.title = values[1];
                   }
                   break;
               case CueCommand::PERFORMER:
                   if (isHeaderParsed) {
                       auto& lastTrack = cue.tracks[cue.tracks.size() - 1];
                       lastTrack.performer = values[1];
                   }
                   else {
                       cue.performer = values[1];
                   }
                   break;
               case CueCommand::FILE:
                   isHeaderParsed = true;
                   cue.wavFileName = values[1];
                   break;
               case CueCommand::TRACK:
                   cue.tracks.emplace_back(parseTrack(values));
                   break;
               case CueCommand::INDEX:
                   {
                       auto& lastTrack = cue.tracks[cue.tracks.size() - 1];
                       lastTrack.indices.emplace_back(parseTrackIndex(values));
                   }
                   break;
               default:
                   break;
                }
            }

        }

    private:
        CueCommand parseCommand(std::string_view value) noexcept
        {
            if (value == "REM") {
                return CueCommand::REM;
            }
            else if (value == "PERFORMER") {
                return CueCommand::PERFORMER;
            }
            else if (value == "TITLE") {
                return CueCommand::TITLE;
            }
            else if (value == "FILE") {
                return CueCommand::FILE;
            }
            else if (value == "TRACK") {
                return CueCommand::TRACK;
            }
            else if (value == "INDEX") {
                return CueCommand::INDEX;
            }
            return CueCommand::UNKNOWN;
        }

        CueTrack parseTrack(std::vector<std::string> values) noexcept
        {
            assert(values.size() == 3);
            assert(values[0] == "TRACK");
            const auto& id = values[1];
            const auto& type = values[2];
            CueTrack track{};
            track.id = std::stoi(id);
            track.type = type;
            return track;
        }

        CueTrackIndex parseTrackIndex(std::vector<std::string> values) noexcept
        {
            assert(values.size() == 3);
            assert(values[0] == "INDEX");
            const auto& index = values[1];
            const auto& time = values[2];
            CueTrackIndex trackIndex{};
            trackIndex.index = std::stoi(index);
            trackIndex.time = time;
            return trackIndex;
        }

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
                        startPtr = ptr + 1;
                    }
                    else {
                        endPtr = ptr - 1;
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
