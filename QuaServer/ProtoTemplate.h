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
#ifndef FLUX_QUASERVER_PROTOTEMPLATE_H_
#define FLUX_QUASERVER_PROTOTEMPLATE_H_

#include <arpa/inet.h>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include "open_app_desc.pb.h"

#define SIM_RSP_PARSE 869162
#define ORIGIN_RSP_PARSE 869161

namespace prototemplate {

template <typename T>
bool SetBinaryPack(T& value, char *buf, uint32_t& total) {
    uint32_t tlen = total;
    uint32_t len = value.ByteSize();
    *(reinterpret_cast<uint32_t *>(buf)) = htonl(len);
    total = len + sizeof(uint32_t);

    if (total >= tlen) {
        return false;
    }

    value.SerializeToArray(buf + 4, len);
    return true;
}

template <typename T>
bool SetBinaryPack(T& value, char *buf, int& total) {
    int tlen = total;
    int len = value.ByteSize();
    *(reinterpret_cast<int *>(buf)) = htonl(len);
    total = len + sizeof(int);

    if (total >= tlen) {
        return false;
    }

    value.SerializeToArray(buf + 4, len);
    return true;
}

template<typename PRO_REQ, typename PRO_RSP>
class ProtoPack {
 public:
    ProtoPack() {}
    ~ProtoPack() {}
    int ParseReqTmplt(char *req, int req_len) {
        //  FTRACE

        if ((NULL == req) || (0 == req_len)) {
            return -5;
        }

        if (!cmd_base.ParseFromArray(req, req_len)) {
            return -1;
        }

        if (!spe_req.ParseFromArray(cmd_base.body().c_str(), cmd_base.body().length())) {
            return -2;
        }

        return 0;
    }

    int ParseRspTmplt(char *rsp, int rsp_len) {
        //  FTRACE

        if ((NULL == rsp) || (0 == rsp_len)) {
            return -5;
        }

        if (!cmd_rsp_base.ParseFromArray(rsp, rsp_len)) {
            return -1;
        }

        bool flag = open_app_desc::BODY_RESPONSE == cmd_rsp_base.type();

        if (flag) {
            if (!spe_rsp.ParseFromArray(cmd_rsp_base.body().c_str(),
                    cmd_rsp_base.body().length())) {
                return -5;
            }
        } else {
            if (!code_rsp.ParseFromArray(cmd_rsp_base.body().c_str(),
                    cmd_rsp_base.body().length())) {
                return -2;
            }

            if (!spe_rsp.ParseFromArray(code_rsp.resp().c_str(), code_rsp.resp().length())) {
                return -3;
            }
        }
        return 0;
    }

    void GetValue(std::string& req_base, std::string& req_req,
                  std::string& rsp_base, std::string& rsp_base_rsp,
                  std::string& rsp_rsp) {
        req_base = cmd_base.DebugString();
        req_req = spe_req.DebugString();

        rsp_base = cmd_rsp_base.DebugString();
        rsp_base_rsp = code_rsp.DebugString();
        rsp_rsp = spe_rsp.DebugString();
        return;
    }

 public:
    open_app_desc::CmdBase cmd_base;
    PRO_REQ spe_req;

    open_app_desc::CmdBase cmd_rsp_base;
    open_app_desc::CmdBaseResp code_rsp;
    PRO_RSP spe_rsp;
};

inline int ProtocolCheck(void *data, int32_t len) {
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
}  //  namespace prototemplate

#define GET_PRO_VAL(item, name) (item.name()>0)?(item.name()):0;
#define GET_PRO_STR(item, name) (item.name().size()>0)?(item.name()):string("");

#endif   //  FLUX_QUASERVER_PROTOTEMPLATE_H_

