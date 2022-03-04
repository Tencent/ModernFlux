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
#include <sstream>
#include <iostream>
#include "open_app_desc.pb.h"
#include "SppApi.h"
#include "TypeTransform.h"

#define IntToStr  TypeTransform::IntToString

int32_t SppBinRoute(void *arg1) {
    open_app_desc::CmdBase pack;
    int cmd = 0;
    uint32_t route = 0;

    //  std::cout<<"GetGroupsSum():"<<GetGroupsSum()<<endl;
    if (GetGroupsSum() < 2)
        return route;

    blob_type *blob = reinterpret_cast<blob_type *>(arg1);
    uint32_t iPkgLen = ntohl(*(reinterpret_cast<uint32_t *>(blob->data)));
    if ((blob->len >= 5) && (pack.ParseFromArray(blob->data + 4, iPkgLen))) {
        if (pack.type()>0) {
        }

        if (pack.cmd_id()>0) {
            cmd = pack.cmd_id();
        }

        if ((QUOTA_QUERY_CMD == cmd) && (pack.body().size()>0)) {
            open_app_desc::QuotaReq req;
            if (true == req.ParseFromArray(pack.body().c_str(), pack.body().length())) {
                if (req.key().size() > 0) {
                    uint32_t num = 0;
                    for (size_t i = 0; i < req.key().size(); ++i) {
                        num += req.key()[i];

                        route = num % (GetGroupsSum() - 1) + 2;
                    }
                }
            }
        } else if (SECOND_QUOTA_CMD == cmd) {
            route = 1;  //  shift
        }
    }
    //  std::cout<<"cmd:"<<cmd<<" route:"<<route<<endl;
    return route;
}

void L5TransString(cl5::QOSREQUEST& qos, std::string& l5) {
    stringstream ss;
    if ((qos._modid > 0) && (qos._cmd > 0)) {
        ss << qos._modid << ":" << qos._cmd;
        l5 = ss.str();
    }
    return;
}

int GetL5IpAndPort(cl5::QOSREQUEST& qos_req, int modid, int cmdid,
                   string& destip, int& destport, string& msg,
                   string default_destip, int default_destport) {
    destip = default_destip;  //  "10.157.6.56";
    destport = default_destport;  //  34008;

    //  int modid = 64040257;
    //  int cmdid = 65536;
    float time_out = 0.1;


    int iRet = cl5::ApiInitRoute(modid, cmdid, time_out, msg);
    if (iRet < 0) {
        msg = "ApiInitRoute failed! modid:" + IntToStr(modid) + ", cmdid:" +
                IntToStr(cmdid) + ", iRet: " + IntToStr(
                  iRet) + " " + msg;
        return iRet;
    }

    qos_req._modid = modid;
    qos_req._cmd = cmdid;
    time_out = 1;
    iRet = cl5::ApiGetRoute(qos_req, time_out, msg, NULL);
    if (iRet < 0) {
        msg = "ApiGetRoute failed! modid:" + IntToStr(modid) + ", cmdid:" +
            IntToStr(cmdid) + ", iRet: " + IntToStr(
                  iRet) + " " + msg;
        return iRet;
    } else {
        destip = qos_req._host_ip;
        destport = qos_req._host_port;
    }

    return 0;
}

int GetL5IpAndPort(int modid, int cmdid, string& ip, int& port) {
    cl5::QOSREQUEST qos;
    string msg;
    string defip;
    int defport = 0;
    GetL5IpAndPort(qos, modid, cmdid, ip, port, msg, defip, defport);
    return 0;
}


namespace SppApi {

int mt_tcpsendrcv_route(void *pkg, int len, void *rcv_buf, int& buf_size,
                        MtFuncTcpMsgLen func, SppApi::L5ROUTER& router) {
    int iRet = 0;
    int the_buf_len = buf_size;
    int64_t begin = __spp_get_now_ms();

    const int& num = router.num;
    const int& timeout = router.timeout;
    cl5::QOSREQUEST& qos = router.qos;
    SppApi::SendInfo& sendinfo = router.sendinfo;
    const string& ip = router.ip;
    const unsigned short& port = router.port;

    for (int i = 0; i < num; i++) {
        std::string err_msg;
        struct sockaddr_in stPeerServerAddr;
        memset(&stPeerServerAddr, 0, sizeof(stPeerServerAddr));
        stPeerServerAddr.sin_family = AF_INET;

        int l5ret = 0;
        if (qos._modid > 0  && qos._cmd > 0)
            l5ret = cl5::ApiGetRoute(qos, L5_TIMEOUT, err_msg, NULL);
        else
            l5ret = -1;

        sendinfo.senditem[i].modid = qos._modid;
        sendinfo.senditem[i].cmd = qos._cmd;
        sendinfo.senditem[i].l5ret = l5ret;
        L5TransString(qos, sendinfo.senditem[i].sl5);

        if (l5ret) {  //  default ip
            SF_LOG(LOG_DEBUG, "LocalHost ip:%s, port:%u\n", ip.c_str(), port);
            stPeerServerAddr.sin_addr.s_addr = inet_addr(ip.c_str());
            stPeerServerAddr.sin_port = htons(port);
            sendinfo.senditem[i].ip = ip;
            sendinfo.senditem[i].port = port;

            //  aquota
            if ((ip.size() == 0) || (0 == port)) {
                iRet = -9;
                break;
            }

        } else {
            stPeerServerAddr.sin_addr.s_addr = inet_addr(qos._host_ip.c_str());
            stPeerServerAddr.sin_port = htons(qos._host_port);
            sendinfo.senditem[i].ip = qos._host_ip;
            sendinfo.senditem[i].port = qos._host_port;
        }

        SF_LOG(LOG_DEBUG, "qos._modid:%u, qos._cmd:%u, %s:%u\n",
            qos._modid, qos._cmd, qos._host_ip.c_str(), qos._host_port);

        sendinfo.ss << (i + 1);

        if ((sendinfo.senditem[i].modid > 0) && (sendinfo.senditem[i].cmd > 0)) {
            sendinfo.ss << " modid:" << sendinfo.senditem[i].modid << " cmd:"
                        << sendinfo.senditem[i].cmd
                        << " ret:" << sendinfo.senditem[i].l5ret << " ";
        }

        sendinfo.ss << " ip:" << sendinfo.senditem[i].ip << " port:" << sendinfo.senditem[i].port;

        buf_size = the_buf_len;
        sendinfo.times++;
        sendinfo.senditem[i].send = __spp_get_now_ms();
        sendinfo.senditem[i].addr = stPeerServerAddr;
        SF_LOG(LOG_DEBUG, "mt_tcpsendrcv_route timeout:%d", timeout);
        iRet = mt_tcpsendrcv(&stPeerServerAddr, pkg, len, rcv_buf, buf_size, timeout, func);
        sendinfo.senditem[i].recv = __spp_get_now_ms();
        sendinfo.senditem[i].ret = iRet;
        if (sendinfo.senditem[i].recv > sendinfo.senditem[i].send) {
            sendinfo.senditem[i].perdelta = sendinfo.senditem[i].recv - sendinfo.senditem[i].send;
        }

        uint64_t msTimeUsed = sendinfo.senditem[i].recv - sendinfo.senditem[i].send;
        sendinfo.ss << " ret:" << iRet << " ";

        if (0 == iRet) {
            sendinfo.suc++;
        }

        if (0 != iRet) {
            sendinfo.fail++;
        }

        if (-3 == iRet) {
            sendinfo.timeout_num++;
        }

        if (iRet != 0) {
            SF_LOG(LOG_ERROR, "qos._modid:%u, qos._cmd:%u, %s:%u\n",
                qos._modid, qos._cmd, qos._host_ip.c_str(), qos._host_port);
        }

        if (0 == l5ret) {
            cl5::ApiRouteResultUpdate(qos, iRet, msTimeUsed, err_msg);
        }

        if ((0 == iRet) || (-1 == iRet) || (-6 == iRet) || (-3 == iRet)) {
            break;
        }
    }

    sendinfo.delta = __spp_get_now_ms() - begin;
    sendinfo.ss << " performance:" << sendinfo.delta << " times:"
                << sendinfo.times << " timeout times:" <<
                sendinfo.timeout_num;
    return iRet;
}
}  //  namespace SppApi

