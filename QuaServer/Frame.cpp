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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <iostream>
#include "FlowMsg.h"
#include "UDPAPI.h"
#include "SppApi.h"
#include "ServiceQua.h"
#include "tinyxml.h"

const unsigned int MT_STACK_LEN = 128 * 1024;
int g_groups_num = 1;
int g_spp_worker_pid = 0;
string g_host;
extern int g_proxy_port;
static char pathbuff[ 1024 * 10] = {0};
extern time_t g_uptime;
int g_timeout = 5000;
using namespace std;

// Get First Port
int GetFirstPort(const string& filepath) {
    TiXmlDocument doc;
    if (!doc.LoadFile(filepath.c_str())) {
        cerr << doc.ErrorDesc() << endl;
        return -1;
    }
    TiXmlElement *root = doc.FirstChildElement();
    if (root == NULL) {
        cerr << "Failed to load file: No root element." << endl;
        doc.Clear();
        return -2;
    }

    char kname[1024] = "acceptor";
    for (TiXmlElement *elem = root->FirstChildElement(); elem != NULL;
            elem = elem->NextSiblingElement()) {
        if (0 == strncmp(elem->Value(), kname, strlen(kname))) {
            for (TiXmlElement *e1 = elem->FirstChildElement(); e1 != NULL;
                e1 = e1->NextSiblingElement()) {
                return atoi(e1->Attribute("port"));
            }
            break;
        }
    }
    doc.Clear();

    return 0;
}

extern "C" int spp_handle_init(void *arg1, void *arg2) {
    g_groups_num = GetGroupsSum();
    g_spp_worker_pid = getpid();

    CServerBase* base = (CServerBase*)arg2;
    struct rlimit rLmt;
    rLmt.rlim_cur = 100000;
    rLmt.rlim_max = 200000;
    int iRet = setrlimit(RLIMIT_NOFILE, (const struct rlimit *)&rLmt);
    if(iRet){
        cout<<"setrlimit 100000/200000 failed! iRet:"<<iRet<<endl;
        return -1;
    }

    if (base->servertype() == SERVER_TYPE_WORKER){
        mt_set_stack_size(MT_STACK_LEN);
        iRet = CSyncFrame::Instance()->InitFrame(base, 100000);
        if (iRet < 0){
           cout<<"Sync framework init failed, ret:"<<iRet<<endl;
           return -1;
       }
    }
    
    SF_LOG(LOG_ERROR, "spp_worker_pid %d\n", g_spp_worker_pid);

    g_uptime = time(NULL);
    getcwd(pathbuff, sizeof(pathbuff) - 1);
    string proxypath = string(pathbuff) + "/../etc/spp_proxy.xml";
    g_proxy_port = GetFirstPort(proxypath.c_str());
    if ((g_proxy_port < 19100) || (g_proxy_port > 19900)) {
        SF_LOG(LOG_ERROR, "proxy port not:%d", g_proxy_port);
        return -1;
    }

    SF_LOG(LOG_ERROR, "proxy port:%d", g_proxy_port);
    GetLocalNormalIp(g_host);
    SF_LOG(LOG_ERROR, "LocalIP: %s\n", g_host.c_str());
    if (0 == g_host.size())
        return -6;

    return 0;
}

int MyProtocolCheck(void *data, int32_t len) {
    if (NULL == data) {
        return -1;
    }

    if (len < 4) {
        return 0;
    }

    uint32_t h = ntohl(*(reinterpret_cast<uint32_t *>(data)));
    int32_t all = h + sizeof(h);
    if (len >= all) {
        return all;
    }
    return 0;
}

extern "C" int spp_handle_input(unsigned flow, void *arg1, void *arg2) {
    blob_type *blob = reinterpret_cast<blob_type *>(arg1);
    //  TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;
    return MyProtocolCheck(blob->data, blob->len);
}

extern "C" int spp_handle_route(unsigned flow, void *arg1, void *arg2) {
    uint32_t route = SppBinRoute(arg1);
    if (route > 0)
        return route;

    static unsigned int adds = 0;
    int group_id = (adds % g_groups_num) + 1;
    adds++;
    return group_id;
}

extern "C" int spp_handle_process(unsigned flow, void *arg1, void *arg2) {
    blob_type    *blob    = reinterpret_cast<blob_type *>(arg1);
    TConnExtInfo *extinfo = reinterpret_cast<TConnExtInfo *>(blob->extdata);

    CServerBase *base  = reinterpret_cast<CServerBase *>(arg2);
    CTCommu     *commu = reinterpret_cast<CTCommu *>(blob->owner);

    FlowMsg *msg = new FlowMsg;
    if (!msg) {
        blob_type respblob;
        respblob.data  = NULL;
        respblob.len   = 0;
        commu->sendto(flow, &respblob, NULL);
        SF_LOG(LOG_ERROR, "close conn, flow:%u\n", flow);

        return -1;
    }

    msg->SetTConnExtInfo(extinfo);
    msg->SetServerBase(base);
    msg->SetTCommu(commu);
    msg->SetFlow(flow);
    msg->SetMsgTimeout(5000);
    msg->SetReqPkg(blob->data, blob->len);

    msg->m_remoteip = inet_ntoa(*(struct in_addr *)&extinfo->remoteip_);

    CSyncFrame::Instance()->Process(msg);

    return 0;
}

extern "C" void spp_handle_fini(void *arg1, void *arg2) {
    CServerBase *base = reinterpret_cast<CServerBase *>(arg2);
    if (base->servertype() == SERVER_TYPE_WORKER) {
        CSyncFrame::Instance()->Destroy();
    }
}

extern "C" void spp_handle_loop(void *arg) {
    ServiceQua::ClearData();
    ServiceQua::LoadSETInfoWrap();
    ServiceQua::LoopLeave();
}

