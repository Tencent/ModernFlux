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
#include <stdio.h>
#include <iostream>
#include "TimeAPI.h"
#include "StringProcess.h"

using namespace std;

int GetTM(struct tm& _tm_t, int delta) {
    struct timeval _tval;
    gettimeofday(&_tval, NULL);
    if ((_tval.tv_sec + delta) > 0)
        _tval.tv_sec += delta;
    localtime_r(&_tval.tv_sec, &_tm_t);
    return 0;
}

int GetWeekIndex() {
    struct tm _tm_t;
    GetTM(_tm_t);

    int index = _tm_t.tm_wday;
    if (0 == index)
        index = 7;

    return (index - 1);
}

double GetCurTime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

int64_t GetMSTime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

time_t GetCurTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec;
}
string GetCurUSecTimeString() {
    struct timeval tv;
    gettimeofday(&tv, 0);

    char szSec[64] = {0};
    char szUSec[64] = {0};
    snprintf(szSec, sizeof(szSec) - 1, "%lu ", tv.tv_sec);
    snprintf(szUSec, sizeof(szUSec) - 1, "%lu", tv.tv_usec);
    string result = string(szSec) + string(szUSec);
    return result;
}

int GetCurTimeString(string& strTime, char szFormat[], int delta) {
    struct tm _tm_t;
    GetTM(_tm_t, delta);

    char _times[128] = {0};
    strftime(_times, sizeof(_times), szFormat, &_tm_t);
    strTime = _times;
    return 0;
}

int GetColorCurTime(char szTime[128], int iLen, char szFormat[]) {
    struct tm _tm_t;
    GetTM(_tm_t);

    strftime(szTime, iLen, "\033[1;33m [%Y-%m-%d|%H:%M:%S]\033[0m", &_tm_t);

    return 0;
}

int GetCurDayTime() {
    struct tm _tm_t;
    GetTM(_tm_t);

    return _tm_t.tm_hour * 60 * 60 + _tm_t.tm_min * 60 + _tm_t.tm_sec;
}


int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y) {
    int iDelta = (y->tv_sec - x->tv_sec) * TIME_EXPONENT + (y->tv_usec - x->tv_usec);
    if (iDelta > 0) {
        result->tv_sec = iDelta / TIME_EXPONENT;
        result->tv_usec = iDelta % TIME_EXPONENT;
        return 1;
    }

    return   0;
}

string Get5MinTime() {
    struct tm t;
    GetTM(t);
    char tmp[64] = {0};
    strftime(tmp, sizeof(tmp), "%Y%m%d%H", &t);
    string nowTime_5(tmp);

    int min = t.tm_min - (t.tm_min % 5);
    char szMin[64] = {0};
    snprintf(szMin, sizeof(szMin) - 1, "%02d", min);

    nowTime_5 += szMin;

    return nowTime_5;
}

int32_t GetNMinTime(const string& strTime, string& sliceTime,
                    const string& format, const int32_t slice) {
    time_t sliceTimeStamp = strtotime(strTime.c_str(), format.c_str());
    sliceTimeStamp = sliceTimeStamp - sliceTimeStamp % slice;
    timetodate(sliceTimeStamp, sliceTime, format);

    return 0;
}

int32_t GetMinTime(const string& strTime, string& sliceTime, const int32_t slice) {
    vector<string> vecTime;
    SplitString(strTime, " ", vecTime);
    if (2 == vecTime.size()) {
        vector<string> vecSec;
        SplitString(vecTime[1], ":", vecSec);
        if (3 == vecSec.size()) {
            int32_t min = atoi(vecSec[1].c_str());
            min = min - min % slice;
            char smin[16] = {0};
            snprintf(smin, sizeof(smin) - 1, "%02d", min);

            sliceTime = vecTime[0] + " " + vecSec[0] + ":" + string(smin) + ":" + "00";
            //  cout<<"sliceTime:"<<sliceTime<<endl;
        }
    }


    return 0;
}

int GetExpiredSecond_5() {
    struct tm t;
    GetTM(t);
    return (5 - t.tm_min % 5) * 60 - t.tm_sec;
}

int GetExpiredSecond_1day() {
    struct tm t;
    GetTM(t);
    return 86400 - t.tm_hour * 3600 - t.tm_min * 60 - t.tm_sec;
}

string Get1DayTime() {
    struct tm t;
    GetTM(t);
    char tmp[64] = {0};
    strftime(tmp, sizeof(tmp) - 1, "%Y%m%d", &t);
    string nowDay_1(tmp);
    return nowDay_1;
}

time_t strtotime(const char *const date, const char *const format) {
    struct tm tm;
    strptime(date, format, &tm);
    time_t ft = mktime(&tm);
    return ft;
}

time_t daytotime(const string& day, const string& format) {
    string theday = day + " 00:00:00";
    return strtotime(theday.c_str(), format.c_str());
}

int datetodate(const string& src, string& des, int delta, const string& format) {
    string theday = src + " 00:00:00";
    time_t thetime = strtotime(theday.c_str(), format.c_str());
    des = timetodate(thetime + delta);
    return 0;
}

int datetodate(const string& src, string& des, int delta,
               const string& format1, const string& format2) {
    string theday = src + " 00:00:00";
    time_t thetime = strtotime(theday.c_str(), format1.c_str());
    timetodate(thetime + delta, des, format2);
    return 0;
}

int32_t DayToDay(const string& src, string& des, int delta, const string& format) {
    string outFormat;
    if (TIME_FORMAT_Ymd == format)
        outFormat = TIME_FORMAT_YmdHMS2;

    if (TIME_FORMAT_YYYYMMDD == format)
        outFormat = TIME_FORMAT_YmdHMS1;

    datetodate(src, des, delta * ONE_DAY_SEC, outFormat, format);
    return 0;
}

int32_t timetodate(const time_t timer, string& strTime, const string& format) {
    struct tm l;
    localtime_r(&timer, &l);

    char tmp[64] = {0};
    strftime(tmp, sizeof(tmp) - 1, format.c_str(), &l);
    strTime = tmp;

    return 0;
}

string timetodate(time_t const timer) {
    struct tm l;
    localtime_r(&timer, &l);

    char buf[128] = {0};
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", l.tm_year + 1900, l.tm_mon + 1, l.tm_mday);
    string s(buf);
    return s;
}

//  Same 5 minutes
bool In5Minutes(uint32_t t1, uint32_t t2) {
    return ((t1 - t1 % 300) == (t2 - t2 % 300));
}

//   Same day
bool InOneDay(uint32_t t1, uint32_t t2) {
    return ((t1 - t1 % 86400) == (t2 - t2 % 86400));
}

//  Same hour
bool InOneHour(uint32_t t1, uint32_t t2) {
    return ((t1 - t1 % 3600) == (t2 - t2 % 3600));
}

uint32_t GetCurHour() {
    struct tm tm_t;
    struct timeval tval;
    gettimeofday(&tval, NULL);
    localtime_r(&tval.tv_sec, &tm_t);

    return tm_t.tm_hour;
}

int GetMinute() {
    struct tm _tm_t;
    GetTM(_tm_t);

    return _tm_t.tm_min;
}


