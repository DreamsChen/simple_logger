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

#include "Formatter.h"

namespace simple_logger
{
#ifndef HAS_STD_FORMAT
    void FormatExpand(std::stringstream& ss, const char* fmt)
    {
        while (*fmt) {
            if (*fmt == '{' && *(++fmt) == '}') {
#ifndef NDEBUG
                throw std::runtime_error("Invalid format string: missing arguments");
#else
                ++fmt;
                ++fmt;
#endif
            }
            ss << *fmt++;
        }
    }
#endif
}
