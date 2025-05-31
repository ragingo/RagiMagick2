#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iosfwd>
#include <string>
#include <string_view>
#include <type_traits>

namespace RagiMagick2::Common
{
    class BinaryFileReader final
    {
    public:
        enum class SeekOrigin
        {
            Begin,
            Current,
            End
        };

    public:
        BinaryFileReader(std::string_view filename, bool isByteSwap = true) noexcept
        {
            m_Filename = filename;
            m_IsByteSwap = isByteSwap;
        }

        ~BinaryFileReader()
        {
            m_Stream.close();
        }

        bool open() noexcept
        {
            if (!std::filesystem::exists(m_Filename)) {
                return false;
            }

            m_Size = std::filesystem::file_size(m_Filename);
            m_Stream.open(m_Filename, std::ios::binary);

            if (!m_Stream.is_open()) {
                return false;
            }

            return true;
        }

        template <typename T, size_t N>
        void ReadBytes(std::array<T, N>& buffer, size_t count = sizeof(T) * N) noexcept
        {
            m_Stream.read(reinterpret_cast<char*>(buffer.data()), count);
        }

        template <std::ranges::random_access_range T>
        void ReadBytes(T& buffer) noexcept
        {
            m_Stream.read(
                reinterpret_cast<char*>(std::data(buffer)),
                std::size(buffer) * sizeof(std::ranges::range_value_t<T>)
            );
        }

        template <typename T>
            requires (std::is_same_v<T, uint8_t> || (std::is_enum_v<T> && std::is_same_v<std::underlying_type_t<T>, uint8_t>))
        void ReadUInt8(T& buffer) noexcept
        {
            m_Stream.read(reinterpret_cast<char*>(&buffer), sizeof(T));
        }

        template <typename T>
            requires (std::is_same_v<T, uint16_t> || (std::is_enum_v<T> && std::is_same_v<std::underlying_type_t<T>, uint16_t>))
        void ReadUInt16(T& buffer) noexcept
        {
            m_Stream.read(reinterpret_cast<char*>(&buffer), sizeof(T));
            if (m_IsByteSwap) {
                if constexpr (std::is_enum_v<T>) {
                    buffer = static_cast<T>(std::byteswap(static_cast<std::underlying_type_t<T>>(buffer)));
                }
                else {
                    buffer = std::byteswap(buffer);
                }
            }
        }

        template <typename T>
            requires (std::is_same_v<T, uint32_t> || (std::is_enum_v<T> && std::is_same_v<std::underlying_type_t<T>, uint32_t>))
        void ReadUInt32(T& buffer) noexcept
        {
            m_Stream.read(reinterpret_cast<char*>(&buffer), sizeof(T));
            if (m_IsByteSwap) {
                if constexpr (std::is_enum_v<T>) {
                    buffer = static_cast<T>(std::byteswap(static_cast<std::underlying_type_t<T>>(buffer)));
                }
                else {
                    buffer = std::byteswap(buffer);
                }
            }
        }

        size_t GetSize() const noexcept
        {
            return m_Size;
        }

        void Seek(std::streamoff pos, SeekOrigin origin = SeekOrigin::Begin) noexcept
        {
            using enum SeekOrigin;
            switch (origin) {
            case Begin:
                m_Stream.seekg(pos, std::ios::beg);
                break;
            case Current:
                m_Stream.seekg(pos, std::ios::cur);
                break;
            case End:
                m_Stream.seekg(pos, std::ios::end);
                break;
            }
        }

        std::streamoff GetCurrentPosition()
        {
            return m_Stream.tellg();
        }

        bool isEOF()
        {
            return GetCurrentPosition() >= GetSize();
        }

        void clear() noexcept
        {
            m_Stream.clear();
        }

    private:
        std::string m_Filename;
        std::ifstream m_Stream;
        size_t m_Size = 0;
        bool m_IsByteSwap;
    };
} // namespace RagiMagick2::Common
