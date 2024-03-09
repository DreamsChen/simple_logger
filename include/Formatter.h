// Copyright(c) 2023-present, Dreams Chen.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef FORMATTER_H
#define FORMATTER_H

#include <type_traits>

#if __has_include(<format>)
#include <format>
#include <concepts>

#define HAS_STD_FORMAT
#define FORMAT std::format
#else
#include <string>
#include <sstream>

#define FORMAT simple_logger::format
#endif

namespace simple_logger
{
#ifdef HAS_STD_FORMAT
    template <typename T>
    concept EnumType = std::is_enum_v<T>;

    template <EnumType EnumValue, typename CharType>
    struct std::formatter<EnumValue, CharType> : std::formatter<int, CharType>
    {
        template <typename FormatContext>
        typename FormatContext::iterator format(const EnumValue& v, FormatContext& formatContext)
        {
            typename FormatContext::iterator itr = std::formatter<int, CharType>().format(static_cast<int>(v), formatContext);
            return itr;
        }
    };
#else
    void FormatExpand(std::stringstream& s, const char* fmt);

    template<typename T, typename = typename std::enable_if<std::is_enum<T>::value>::type>
    inline std::stringstream& operator<<(std::stringstream& ss, T& value)
    {
        ss << static_cast<int>(value);
        return ss;
    }

    template<typename T, typename... Args>
    void FormatExpand(std::stringstream& ss, const char* fmt, T& value, Args... args)
    {
        while (*fmt) {
            if (*fmt == '{' && *(++fmt) == '}') {
                ss << value;
                FormatExpand(ss, ++fmt, args...);
                return;
            }
            ss << *fmt++;
        }
#ifndef NDEBUG
        throw std::logic_error("Invalid formatting: too much arguments are provided to format");
#endif
    }

    template<typename ... Args>
    std::string format(const char* fmt, Args ... args)
    {
        std::stringstream ss;
        FormatExpand(ss, fmt, args...);
        return ss.str();
    }
#endif
}
#endif // !FORMATTER_H
