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
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include "CpuInfo.h"

using namespace std;

int CpuInfo::InitCpuInfo() {
    uint64_t worktime;
    uint64_t totaltime;
    uint64_t mem_data[4];
    int ret = GetWorkTimeInfo(worktime, totaltime);
    if (ret != 0) {
        return ret;
    }
    cpu_time[0] = worktime;
    cpu_time[1] = totaltime;


    cpu_usage = 0;

    ret = GetMEMInfo(mem_data);
    for (i = 0; i < 4; i++) {
        memory[i] = mem_data[i];
    }
    if (memory[0] > 0) {
        mem_usage = ((memory[0] - memory[1] - memory[2] - memory[3]) * 100) / memory[0];
    } else {
        mem_usage = 0;
    }

    lock = 0;

    return 0;
}

int CpuInfo::GetCpuInfo() {
    uint64_t worktime;
    uint64_t totaltime;
    uint64_t mem_data[4];
    int ret = GetWorkTimeInfo(worktime, totaltime);
    if (ret != 0) {
        return ret;
    }

    if ((worktime - cpu_time[0] > 0) && (totaltime - cpu_time[1] > 0)) {
        cpu_usage = ((worktime - cpu_time[0]) * 100) / (totaltime - cpu_time[1]);
        cpu_time[0] = worktime;
        cpu_time[1] = totaltime;
    } else {
        return 1;
    }

    GetMEMInfo(mem_data);

    if (mem_data[0] > 0 && mem_data[0] - mem_data[1] > 0) {
        for (i = 0; i < 4; i++) {
            memory[i] = mem_data[i];
        }
        mem_usage = ((memory[0] - memory[1] - memory[2] - memory[3]) * 100) / memory[0];
    }

    return 0;
}

int CpuInfo::GetMEMInfo(uint64_t mem_data[4]) {
    FILE *fd;
    char meminfo[6][50];
    char meminfo_num[4][50];

    int i = 0, j = 0;
    uint32_t k = 0;
    char s0[] = "MemTotal", s1[] = "MemFree", s2[] = "Buffers", s3[] = "Cached";

    fclose(fd);
    fd = fopen("/proc/meminfo", "r");
    /*
    MemTotal:        xxxxxxx kB
    MemFree:         xxxxxxx kB
    Buffers:         xxxxxxx kB
    Cached:          xxxxxxx kB
    */
    if (fd == NULL) {
        return 1;
    } else {
        for (i = 0; i < 5; i++) {
            fgets(meminfo[5], 50, fd);
            strcpy(meminfo[i], meminfo[5]);
        }
    }
    fclose(fd);

    memset(meminfo_num, 0, sizeof(meminfo_num));

    for (i = 0; i < 5; i++) {
        j = 0;

        int index = 0;
        if (strlen(meminfo[i]) > 0) {
            if (strncmp(meminfo[i], s0, 8) == 0) {
                index = 0;
            } else if (strncmp(meminfo[i], s1, 7) == 0) {
                index = 1;
            } else if (strncmp(meminfo[i], s2, 7) == 0) {
                index = 2;
            }  else if (strncmp(meminfo[i], s3, 6) == 0) {
                index = 3;
            } else {
                continue;
            }
        }


        mem_data[index] = 0;
        for (k = 0; k < strlen(meminfo[i]); k++) {
            if (isdigit(meminfo[i][k]))
                meminfo_num[i][j++] = meminfo[i][k];
        }
        mem_data[index] = strtol(meminfo_num[i], reinterpret_cast<char **>(NULL), 10);
    }
    return 0;
}


int CpuInfo::GetWorkTimeInfo(uint64_t& worktime , uint64_t& totaltime) {
    FILE *fd;
    char buff[1024];
    char name[32];
    uint64_t data[7];
    int i = 0, j = 0;
    size_t k = 0;

    fd = fopen("/proc/stat", "r");
    /*
    user: normal processes executing in user mode
    nice: niced processes executing in user mode
    system: processes executing in kernel mode
    idle: twiddling thumbs
    iowait: waiting for I/O to complete
    irq: servicing interrupts
    softirq: servicing softirqs
    */
    if (fd == NULL) {
        return 1;
    } else {
        fgets(buff, sizeof(buff), fd);
        sscanf(buff, "%s %ju %ju %ju %ju %ju %ju %ju", name, data, data + 1,
               data + 2, data + 3, data + 4, data + 5, data + 6);
        // worktime doesn't count idle and iowait time
        worktime = data[0] + data[1] + data[2] + data[5] + data[6];
        totaltime = worktime + data[3];
    }

    fclose(fd);

    if ((worktime - cpu_time[0] <= 0) || (totaltime - cpu_time[1] <= 0)) {
        return 1;
    }
    return 0;
}