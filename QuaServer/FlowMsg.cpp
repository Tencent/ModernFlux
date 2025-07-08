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
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdexcept>
#include <sstream>
#include "sync_incl/SyncFrame.h"
#include "SppApi.h"
#include "StringProcess.h"
#include "TypeTransform.h"
#include "ServiceQua.h"
#include "ProtoTemplate.h"
#include "FlowMsg.h"

#define AMS_RSP_BUFF_SIZE (65535)
const unsigned int AMS_LOTTERY_BUFF_SIZE = 32 * 1024;
#define DEBUG_BASE_INFO(ss) (ss << "["__FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << "] ")
time_t g_uptime = 0;

FlowMsg::FlowMsg() {
    m_extinfo = NULL;
    memset(&m_oAddr, 0, sizeof(m_oAddr));
}

FlowMsg::~FlowMsg() {
}

int FlowMsg::ErrorRsp(blob_type& r, open_app_desc::CmdBase& rspHead, int iResult, const char *pMsg,
                     const std::string& strSerialNum) {
    open_app_desc::CmdBaseResp rspBody;
    rspBody.set_ret(iResult);
    if (pMsg && strlen(pMsg)) {
        rspBody.set_err_msg(pMsg);
    }

    std::string sBodyStr;
    rspBody.SerializeToString(&sBodyStr);
    rspHead.set_type(open_app_desc::BODY_RESPONSE);
    rspHead.set_body(sBodyStr);
    int iLen = rspHead.ByteSize();

    if ((iLen + 4) >= r.len) {
        SF_LOG(LOG_ERROR,
        "FlowMsg::ErrorRsp,open_app_desc::CmdBase.SerializeToArray buffer not enough!\n");
        return -1;
    }

    *(reinterpret_cast<uint32_t *>(r.data)) = htonl(iLen);
    r.len = iLen + sizeof(uint32_t);
    if (!rspHead.SerializeToArray((reinterpret_cast<char *>(r.data)) + 4, iLen)) {
        SF_LOG(LOG_ERROR, "FlowMsg::ErrorRsp,open_app_desc::CmdBase.SerializeToArray failed!\n");
        return -2;
    }
    //  echo logic
    SF_LOG(LOG_DEBUG, "==ByteSize:%d, CmdBase==\n%s\n",
        rspHead.ByteSize(), rspHead.DebugString().c_str());
    this->SendToClient(r);
    return 0;
}

int FlowMsg::ErrorRsp(open_app_desc::CmdBase& rspHead, int iResult,
                    const char *pMsg, const std::string& strSerialNum) {
    static char strRspBuf[AMS_RSP_BUFF_SIZE];
    blob_type r;
    r.len = sizeof(strRspBuf);
    r.data = strRspBuf;

    return ErrorRsp(r, rspHead, iResult, pMsg, strSerialNum);
}

int FlowMsg::HandleProcess() {
    blob_type blob;
    const std::string& packet = this->GetReqPkg();
    SetPeerAddr();

    // parse cmd base
    open_app_desc::CmdBase cmd_base;
    bool parse_ret = cmd_base.ParseFromArray(packet.data() + sizeof(int),
                     packet.length() - sizeof(int));
    if (true != parse_ret) {
        open_app_desc::CmdBaseResp resp_body;
        resp_body.set_ret(-4);
        resp_body.set_err_msg("broker parse packet in protobuf fail");

        open_app_desc::CmdBase resp;
        resp.set_type(open_app_desc::BODY_RESPONSE);
        std::string strTmp;
        resp_body.SerializeToString(&strTmp);
        resp.set_body(strTmp);

        int content_length = resp.ByteSize();
        int net_length = htonl(content_length);
        std::string rsp_packet = std::string(reinterpret_cast<const char *>(&net_length),
                                             sizeof(net_length));
        resp.AppendToString(&rsp_packet);

        blob.data = (char *)rsp_packet.data();
        blob.len  = rsp_packet.size();

        this->SendToClient(blob);

        return 0;
    }

    //  SF_LOG(LOG_DEBUG, "cmd_base:'%s'\n", cmd_base.DebugString().c_str());
    m_bodytype = cmd_base.type();
    switch (cmd_base.type()) {
        case open_app_desc::BODY_REQUEST:
            if (cmd_base.cmd_id() == QUOTA_QUERY_CMD) {
                ProQuota(cmd_base);  //  act
            } else if (SECOND_QUOTA_CMD == cmd_base.cmd_id()) {
                ProSecondQuota(cmd_base);  //  set
            } else {
                string msg = "unknown cmd" + TypeTransform::ToString(cmd_base.cmd_id());
                SF_LOG(LOG_DEBUG, "%s", msg.c_str());
            }
            break;

        default:
            break;
    }

    return 0;
}

int FlowMsg::ProQuota(open_app_desc::CmdBase& base) {
    open_app_desc::QuotaReq req;
    if (false == req.ParseFromArray(base.body().c_str(), base.body().length())) {
        // aqota
        SF_LOG(LOG_DEBUG, "open_app_desc::QuotaReq.ParseFromArray failed!");
        int iErrCode = -1;
        string strErrMsg = "Invalid reqbody";
        ErrorRsp(base, iErrCode, strErrMsg.c_str(), "");
        stringstream ss;
        DEBUG_BASE_INFO(ss);
        ss << " parse QuotaReq failed" << endl;
        return iErrCode;
    }
    open_app_desc::QuotaRsp rsp;
    QEXTINFO info;
    info.Init(g_uptime, m_remoteip);
    ServiceQua::GetQueryQuota(info, req, rsp);
    open_app_desc::CmdBase baseresp;
    if (open_app_desc::BODY_REQUEST == m_bodytype) {
        std::string strrsp;
        rsp.SerializeToString(&strrsp);  //  aquota
        baseresp.set_type(open_app_desc::BODY_RESPONSE);
        baseresp.set_body(strrsp);
    } else {
        open_app_desc::CmdBaseResp bodyresp;
        bodyresp.set_ret(0);
        std::string strrsp;
        rsp.SerializeToString(&strrsp);  //  aquota

        bodyresp.set_resp(strrsp);
        //  SF_LOG(LOG_DEBUG, "bodyresp:'%s'\n", bodyresp.DebugString().c_str());

        std::string strbody;
        bodyresp.SerializeToString(&strbody);

        baseresp.set_type(open_app_desc::BODY_RESPONSE);
        baseresp.set_body(strbody);
    }
    //  SF_LOG(LOG_DEBUG, "baseresp:'%s'\n", baseresp.DebugString().c_str());

    char strRspBuf[AMS_LOTTERY_BUFF_SIZE];
    blob_type r;
    r.len = sizeof(strRspBuf);
    r.data = strRspBuf;
    if (prototemplate::SetBinaryPack(baseresp, strRspBuf, r.len)) {
        this->SendToClient(r);
    }

    return 0;
}

int FlowMsg::ProSecondQuota(open_app_desc::CmdBase& base) {
    open_app_desc::SetQuotaReq req;
    if (false == req.ParseFromArray(base.body().c_str(), base.body().length())) {
        // aqota
        SF_LOG(LOG_DEBUG, "open_app_desc::SetQuotaReq.ParseFromArray failed!");
        int iErrCode = -1;
        string strErrMsg = "Invalid reqbody";
        ErrorRsp(base, iErrCode, strErrMsg.c_str(), "");
        stringstream ss;
        DEBUG_BASE_INFO(ss);
        ss << " parse SetQuotaReq failed" << endl;
        return iErrCode;
    }
    SF_LOG(LOG_DEBUG, "\nrsp:\n'%s'\n", req.DebugString().c_str());

    open_app_desc::SetQuotaRsp rsp;
    ServiceQua::ProSETQuota(req, m_remoteip, rsp);
    //  SF_LOG(LOG_DEBUG, "\nrsp:\n%s\n%s\n", rsp.DebugString().c_str(), m_remoteip.c_str());

    open_app_desc::CmdBase baseresp;
    if (open_app_desc::BODY_REQUEST == m_bodytype) {
        std::string strrsp;
        rsp.SerializeToString(&strrsp);  //  aquota
        baseresp.set_type(open_app_desc::BODY_RESPONSE);
        baseresp.set_body(strrsp);
    }
    char strRspBuf[AMS_LOTTERY_BUFF_SIZE];
    blob_type r;
    r.len = sizeof(strRspBuf);
    r.data = strRspBuf;
    if (prototemplate::SetBinaryPack(baseresp, strRspBuf, r.len)) {
        this->SendToClient(r);
    }
    return 0;
}


