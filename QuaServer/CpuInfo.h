//  Copyright (C) 2019 Tencent. All rights reserved.
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
#ifndef FLUX_QUASERVER_CPUINFO_H_
#define FLUX_QUASERVER_CPUINFO_H_
#include <time.h>
//  #include "Attr_API.h"
//  typedef struct shm_data_str {
//  uint64_t  cpu_time[2]; // [0]:worktime [1]:totaltime
//  uint64_t  memory[4]; // [0]:MemTotal [1]:MemFree [2]:Buffers [3]:Cached
//  uint64_t  cpu_usage;
//  uint64_t  mem_usage;
//  time_t    last_time;
//  uint32_t  lock;
// } SHM_DATA;



static const uint32_t s_get_proc_stat = 868170;
static const uint32_t s_get_proc_meminfo = 868171;
static const uint32_t s_calculate_cpu = 868172;
static const uint32_t s_calculate_mem = 868173;



class CpuInfo {
 public:
    int InitCpuInfo();

    int GetCpuInfo();

    int GetMEMInfo(uint64_t mem_data[4]);

    int GetWorkTimeInfo(uint64_t& worktime , uint64_t& totaltime);

    uint64_t GetCpuUsage() {
        return cpu_usage;
    }

    uint64_t GetMemUsage() {
        return mem_usage;
    }
 private:
    uint64_t  cpu_time[2]; //  [0]:worktime [1]:totaltime
    uint64_t  memory[4]; //  [0]:MemTotal [1]:MemFree [2]:Buffers [3]:Cached
    uint64_t  cpu_usage;
    uint64_t  mem_usage;
    uint32_t  lock;
};





#endif //  FLUX_QUASERVER_CPUINFO_H_
