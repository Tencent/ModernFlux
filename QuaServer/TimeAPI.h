//  Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the BSD 3-Clause License (the "License"); you may not use this file
//  except in compliance with the License. You may obtain a copy of the License at
//
//  https://opensource.org/licenses/BSD-3-Clause
//
//  Unless required by applicable law or agreed to in writing, software distributed
//  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
//  language governing permissions and limitations under the License.
//
#ifndef FLUX_QUASERVER_TIMEAPI_H_
#define FLUX_QUASERVER_TIMEAPI_H_
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>


#define TIME_EXPONENT 1000000
#define TIME_FORMAT_YmdHMS  "[%Y-%m-%d|%H:%M:%S]"
#define TIME_FORMAT_YmdHMS_Color "\033[1;33m[%Y-%m-%d|%H:%M:%S]\033[0m"
#define TIME_FORMAT_Ymd          "%Y%m%d"
#define TIME_FORMAT_YmdH         "%Y%m%d%H"
#define TIME_FORMAT_YmdHMS_NUM   "%Y%m%d%H%M%S"
#define TIME_FORMAT_YmdHM        "%Y%m%d%H%M"
#define TIME_FORMAT_YYYYMMDD     "%Y-%m-%d"
#define TIME_FORMAT_YmdHMS1      "%Y-%m-%d %H:%M:%S"
#define TIME_FORMAT_YmdHMS2      "%Y%m%d %H:%M:%S"
#define TIME_FORMAT_mdHMS2       "%m-%d %H:%M:%S"
#define ONE_DAY_SEC              (24*60*60)

double GetCurTime();
int64_t GetMSTime();
time_t GetCurTimeStamp();
std::string GetCurUSecTimeString();
int GetTM(struct tm& _tm_t, int delta = 0);
int GetWeekIndex();
int GetCurTimeString(std::string& strTime, char szFormat[], int delta = 0);
int GetColorCurTime(char szTime[128], int iLen, char szFormat[]);
int GetCurDayTime();
int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y);
std::string Get5MinTime();
int32_t GetNMinTime(const std::string& strTime, std::string& sliceTime, const std::string& format,
                    const int32_t slice = 5 * 60);
int32_t GetMinTime(const std::string& strTime, std::string& sliceTime, const int32_t slice = 5);
int GetExpiredSecond_5();
int GetExpiredSecond_1day();
std::string Get1DayTime();
time_t strtotime(const char *const date, const char *const format = TIME_FORMAT_YmdHMS1);
time_t daytotime(const std::string& day, const std::string& format = TIME_FORMAT_YmdHMS1);
int datetodate(const std::string& src, std::string& des, int delta = -ONE_DAY_SEC,
               const std::string& format = TIME_FORMAT_YmdHMS1);
int datetodate(const std::string& src, std::string& des, int delta, const std::string& format1,
               const std::string& format2);
int32_t timetodate(const time_t timer, std::string& strTime, const std::string& format);
std::string timetodate(time_t const timer);
int32_t DayToDay(const std::string& src, std::string& des, int delta, const std::string& format);
int32_t ConvertDate(std::string& src, std::string& desc, uint32_t delta);
bool In5Minutes(uint32_t t1, uint32_t t2);
bool InOneDay(uint32_t t1, uint32_t t2);
bool InOneHour(uint32_t t1, uint32_t t2);
uint32_t GetCurHour();
int GetMinute();

#endif  //  FLUX_QUASERVER_TIMEAPI_H_

