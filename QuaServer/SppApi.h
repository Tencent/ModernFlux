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
#ifndef FLUX_QUASERVER_SPPAPI_H_
#define FLUX_QUASERVER_SPPAPI_H_

#include <sstream>
#include "syncincl.h"
#include "qos_client.h"

#define QUOTA_QUERY_CMD  150
#define HEALTH_PROBE_CMD 151
#define SECOND_QUOTA_CMD 152

// SET Report
#define REAL_REPORT_CMD 153
#define QUOTA_REPORT_CMD 154

void L5TransString(cl5::QOSREQUEST& qos, std::string& l5);
int GetL5IpAndPort(cl5::QOSREQUEST& qos_req, int modid, int cmdid,
                   string& destip, int& destport, string& msg,
                   string default_destip,
                   int default_destport);
int GetL5IpAndPort(int modid, int cmdid, string& ip, int& port);

int32_t SppBinRoute(void *arg1);

namespace SppApi {

#define SEND_TRY_NUM 2
#define L5_TIMEOUT (0.2)  //  seconds
typedef struct {
    int ret;  //  output
    int64_t send;  //  output
    int64_t recv;  //  output
    struct sockaddr_in addr;  //  output
    int64_t modid;  //  output
    int64_t cmd;  //  output
    string ip;  //  output
    int port;  //  output
    int l5ret;
    string sl5;
    int perdelta;  //  (recv - send)

    void Init() {
        ret = 0;
        send = recv = 0;
        port = 0;
        modid = cmd = 0;
        l5ret = 0;
        perdelta = 0;
        memset(&addr, 0, sizeof(addr));
        return;
    }
} SendItem;

typedef struct {
    SendItem senditem[SEND_TRY_NUM];
    int times;
    int timeout_num;
    int suc;
    int fail;
    int64_t delta;
    std::stringstream ss;
    void Init() {
        times = 0;
        suc = fail = timeout_num = 0;
        delta = 0;

        for (int i = 0; i < SEND_TRY_NUM; i++) {
            senditem[i].Init();
        }

        return;
    }
} SendInfo;

typedef struct {
    cl5::QOSREQUEST qos;  //  input
    std::string ip;     //  input
    unsigned short port;  //  input
    int timeout;    //  input ms
    SendInfo sendinfo;  //  output
    int num;  //  input
    void Init() {
        qos._modid = 0;
        qos._cmd = 0;
        port = 0;
        timeout = 80;  //  ms aquota
        sendinfo.Init();
        num = 1;
        return;
    }
} L5ROUTER;

int mt_tcpsendrcv_route(void *pkg, int len, void *rcv_buf, int& buf_size,
                        MtFuncTcpMsgLen func, SppApi::L5ROUTER& router);
}  //  namespace SppApi

#endif  //  FLUX_QUASERVER_SPPAPI_H_


