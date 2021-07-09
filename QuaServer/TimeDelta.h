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
#ifndef FLUX_QUASERVER_TIMEDELTA_H_
#define FLUX_QUASERVER_TIMEDELTA_H_

#include <time.h>
#include "syncincl.h"

inline int64_t GetMicroSecond() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}

class ActTimeDelta {
 public:
    int64_t& delta_;
    int64_t begin_;
    int64_t end_;


 public:
    explicit ActTimeDelta(int64_t& delta): delta_(delta) {
        //  delta_ = 0;
        begin_ = GetMicroSecond();
    }

    ~ActTimeDelta() {
        end_ = GetMicroSecond();
        delta_ += (end_ - begin_);
    }
};

#endif  //  FLUX_QUASERVER_TIMEDELTA_H_
