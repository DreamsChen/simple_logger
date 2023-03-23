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

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#ifdef HAS_STD_FORMAT
#include <format>
#else
#include <string>
#include <sstream>
#endif

namespace simple_logger
{
#ifndef HAS_STD_FORMAT
    void FormatExpand(std::stringstream& s, const char* fmt);

    template<typename T, typename... Args>
    void FormatExpand(std::stringstream& ss, const char* fmt, T value, Args... args)
    {
        while (*fmt) {
            if (*fmt == '{' && *(++fmt) == '}') {
                ss << value;
                FormatExpand(ss, ++fmt, args...);
                return;
            }
            ss << *fmt++;
        }

        throw std::logic_error("extra arguments provided to format");
    }

    template<typename ... Args>
    std::string format(const char* fmt, Args ... args)
    {
        std::stringstream s;
        FormatExpand(s, fmt, args...);
        return s.str();
    }
#endif
}
#endif // !FORMATTER_H
