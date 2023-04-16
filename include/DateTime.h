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

#ifndef DATA_TIME_H
#define DATA_TIME_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>
#include <chrono>

namespace simple_logger 
{
#ifdef linux
    using errno_t = error_t;
#endif

    using Now = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

    time_t GetTime();
    Now GetCurrentTime();

    errno_t GetUtcTm(tm* utcTm);
    errno_t GetLocalTm(tm* localTm);

    std::string GetUtcDateTime();
    std::string GetUtcDate();
    std::string GetUtcTime();
    std::string GetUtcDateTimeWithMilliSecond();
    std::string GetUtcDateTimeWithMilliSecond(const Now& now);

    std::string ToUtcDateTime(const time_t* time);
    std::string ToUtcDate(const time_t* time);
    std::string ToUtcTime(const time_t* time);

    std::string GetLocalDateTime();
    std::string GetLocalDate();
    std::string GetLocalTime();
    std::string GetLocalDateTimeWithMilliSecond();
    std::string GetLocalDateTimeWithMilliSecond(const Now& now);
    std::string GetLocalDateFromUnixTimeStamp(long long timeStamp);
    std::string GetLocalTimeFromUnixTimeStamp(long long timeStamp);

    std::string ToLocalDateTime(const time_t* time);
    std::string ToLocalDate(const time_t* time);
    std::string ToLocalTime(const time_t* time);

    time_t GetTimeFromString(std::string dateTime, std::string fmt = "%04d%02d%02d-%02d:%02d:%02d");
}

#endif // !DATA_TIME_H