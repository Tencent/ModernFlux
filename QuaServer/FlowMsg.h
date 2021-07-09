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
#ifndef FLUX_QUASERVER_FLOWMSG_H_
#define FLUX_QUASERVER_FLOWMSG_H_

#include "syncincl.h"
#include "open_app_desc.pb.h"
class FlowMsg : public CSyncMsg {
 public:
    FlowMsg();
    ~FlowMsg();
    virtual int HandleProcess();

 public:
    string m_remoteip;
    int64_t m_bodytype;
    int ErrorRsp(blob_type& r, ::open_app_desc::CmdBase& rsp,
                int iResult, const char *pMsg, const std::string& strSerialNum);
    int ErrorRsp(::open_app_desc::CmdBase& rspHead, int iResult,
                const char *pMsg, const std::string& strSerialNum);
    void SetTConnExtInfo(const TConnExtInfo *v) {
        m_extinfo = (v);
    }
    const TConnExtInfo *GetTConnExtInfo(void) {
        return m_extinfo;
    }
    void SetPeerAddr() {
        memset(&m_oAddr, 0, sizeof(m_oAddr));
        m_oAddr.sin_family = AF_INET;
        m_oAddr.sin_port = GetTConnExtInfo()->remoteport_;
        m_oAddr.sin_addr.s_addr = GetTConnExtInfo()->remoteip_;
    }

    const struct sockaddr_in *GetPeerAddr(void) {
        return &m_oAddr;
    }

    // quota
    int ProQuota(open_app_desc::CmdBase& base);
    int ProSecondQuota(open_app_desc::CmdBase& base);

 private:
    const TConnExtInfo *m_extinfo;
    struct sockaddr_in m_oAddr;
};

#endif  //  FLUX_QUASERVER_FLOWMSG_H_
