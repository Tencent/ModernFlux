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
#include <time.h>

#include <map>
#include <sstream>

#include "qos_client.h"
#include "open_app_desc.pb.h"
#include "CpuInfo.h"
#include "TypeTransform.h"
#include "StringProcess.h"
#include "ProtoTemplate.h"
#include "DataWrap.h"
#include "ServiceQua.h"
#include "SppApi.h"
#include "TimeDelta.h"
#include "UDPAPI.h"

using namespace std;
int g_proxy_port = 0;
const std::string SET_CFG_PATH = "/data/website/appsweb/cfg/ams/set/";
const std::string SET_CFG_FILENAME = "set_config";


bool HitProba(uint32_t target) {
    if (target >= s_ratio) {
        return true;
    }

    __spp_do_update_tv();
    srand(__spp_get_now_ms());
    uint32_t value = rand() % s_ratio;
    if (value < target) {
        return true;
    } else {
        return false;
    }
}

int GetReportIP(const string& procid, string& ip) {
    vector<string> vec;
    SplitString(procid, "_", vec);
    if ((vec.size() >= 2) && (vec[0].size() > 0)) {
        ip = vec[0];
    }

    return ip.size();
}

int ServiceQua::InitKeyQua(const string& key) {
    //  FTRACE
    //  SF_LOG(LOG_DEBUG, "%s\n", __func__);
    map<string, QUAWIN>::iterator it = s_map_qua.find(key);
    if (s_map_qua.end() == it) {
        QUAWIN quawin;
        s_map_qua.insert(make_pair(key, quawin));
        s_map_qua[key].Init();
    }
    //  SF_LOG(LOG_DEBUG, "%s, %d\n", __func__, s_map_qua.size());
    return 0;
}

map<string, QUAWIN> ServiceQua::s_map_qua;
MTItem ServiceQua::s_wi;

//  SET cfg
map<string, string> ServiceQua::mapSETIPCfg;
map<string, SETCfgFirst> ServiceQua::mapSETCfgFirst;
map<string, int> ServiceQua::s_map_fd;

//  quota
//  client
LOCALMAP ServiceQua::s_map_local;
uint32_t ServiceQua::s_now;
uint32_t ServiceQua::s_setsum;

//  first
QDATAMAP ServiceQua::s_map_quota;  //  for act sum every active
QDATAMAP ServiceQua::s_map_proc_quota;  //  for processor data
std::set<PROCQ> ServiceQua::s_setproc;

//  second
QDATAMAP ServiceQua::s_map_setsumdata;   //  for set
QDATAMAP ServiceQua::s_map_setprocdata;   //  for processor
string ServiceQua::s_localhost;

int ServiceQua::ClearKey() {
    time_t ttime = __spp_get_now_s();
    map<string, QUAWIN>::iterator it = s_map_qua.begin();
    //  uint32_t total = s_map_qua.size();
    for (; it != s_map_qua.end();) {
        //  ss<<" "<<it->first; // TODO
        if ((ttime - it->second.updatetime) > s_expire)
            s_map_qua.erase(it++);
        else
            ++it;
    }
    //  uint32_t erase = total - s_map_qua.size();
    return 0;
}

DEFINE_QUOTA_ERASE_FUNC(ClearQData, QDATAMAP, updatetime)
DEFINE_QUOTA_ERASE_FUNC(ClearLocalData, LOCALMAP, updatetime)

int ServiceQua::ClearData() {
    static time_t lasterase = 0;
    static time_t lastshow = 0;
    if ((__spp_get_now_s() - lastshow) > 5) {
        //  SF_LOG(LOG_DEBUG, "ClearData %d %s", ServiceQua::s_map_qua.size(), ShowData().c_str());
        lastshow = __spp_get_now_s();
    }

    if ((__spp_get_now_s() - lasterase) > s_offline_interval) {
        uint32_t hour = GetCurHour();
        if (hour > s_offline_begin) {
            stringstream ss;
            ss << "erasing " << lasterase << " " << __spp_get_now_s()
            << " interval:" << (__spp_get_now_s() - lasterase);
            SF_LOG(LOG_DEBUG, "%s", ss.str().c_str());
            ClearKey();
            ServiceQua::ClearQData(s_map_quota);

            ServiceQua::ClearQData(s_map_proc_quota);
            ServiceQua::ClearQData(s_map_setprocdata);
            ServiceQua::ClearQData(s_map_setsumdata);

            ServiceQua::ClearLocalData(s_map_local);
            lasterase = __spp_get_now_s();
        }
    }
    return 0;
}


int64_t ServiceQua::GetFeaID(int64_t base) {
    return base + GetGroupId();
}

uint32_t ServiceQua::MTEnter(uint32_t num) {
    MTItem& wi = s_wi;
    int64_t ident = GetMinute();  //  ttime/(24*60);
    wi.mttotal += num;
    wi.mtnum += num;

    if (wi.mtnum > wi.mtmax) {
        wi.mtmax = wi.mtnum;
    }

    if (wi.ident != ident) {
        SF_LOG(LOG_DEBUG,
               "FlowEnter: send wi.ident:%llu, ident:%llu, max:%llu, num:%llu, total:%llu",
               wi.ident, ident, wi.mtmax, wi.mtnum, wi.mttotal);
        wi.ident = ident;
        wi.mttotal = 0;
        wi.mtmax = 0;
    }

    SF_LOG(LOG_DEBUG,
           "FlowEnter: wi.ident:%llu, ident:%llu, total:%llu, max:%llu, num:%llu",
           wi.ident, ident, wi.mttotal, wi.mtmax, wi.mtnum);

    return wi.mtnum;
}

uint32_t ServiceQua::MTLeave(uint32_t num) {
    MTItem& wi = s_wi;
    wi.mtnum -= num;

    return wi.mtnum;
}

uint32_t ServiceQua::GetMTNum() {
    MTItem& wi = s_wi;

    return wi.mtnum;
}

uint32_t ServiceQua::ReportAtt(uint32_t cpu, uint32_t mem) {
    MTItem& wi = s_wi;
    int64_t ident = GetMinute();  //  ttime/(24*60);

    if (cpu > wi.mtcpumax) {
        wi.mtcpumax = cpu;
    }

    if (mem > wi.mtmemmax) {
        wi.mtmemmax = mem;
    }

    if (wi.ident != ident) {
        wi.ident = ident;
        wi.mtcpumax = 0;
        wi.mtmemmax = 0;
    }

    return 0;
}

string ServiceQua::ShowData() {
    stringstream ss;
    map<string, QUAWIN>::iterator it = s_map_qua.begin();
    for (; it != s_map_qua.end(); it++) {
        const QUAWIN& quawin = it->second;
        ss << endl << "qoskey:" << it->first << " updatetime:" << quawin.updatetime;
        for (uint32_t i = 0; i < win_num; i++) {
            if (0 == quawin.item[i].ttime)
                continue;

            ss << endl << "num:" << i << " time:" << quawin.item[i].ttime
               << " suc:" << quawin.item[i].suc
               << " fail:" << quawin.item[i].fail;
        }
    }
    return ss.str();
}

uint32_t ServiceQua::GenFlowProba(uint32_t erate) {
    uint32_t rate = s_ratio;
    if (erate <= s_ratio) {
        rate = s_ratio - erate;
    }
    return rate;
}

static time_t s_interval = 3;  //  the interval time: second
static CpuInfo ci;
static time_t get_cpu_info_last_time = 0;


bool ServiceQua::InitSysload() {
    int iRet = 0;
    get_cpu_info_last_time = time(NULL);
    iRet = ci.InitCpuInfo();

    if (0 == iRet) {
        return true;
    }
    return false;
}

bool ServiceQua::SysOverload(const open_app_desc::Flux *flux) {
    uint64_t cpu_thresh = GetCPURate(flux);
    uint64_t mem_thresh = GetMemRate(flux);

    time_t t1 = time(NULL);
    if (t1 - get_cpu_info_last_time >= GetCPUInterval(flux)) {
        get_cpu_info_last_time = time(NULL);
        ci.GetCpuInfo();
    }
    uint64_t cpu_usage = ci.GetCpuUsage();
    uint64_t mem_usage = ci.GetMemUsage();
    ReportAtt(cpu_usage, mem_usage);

    if (cpu_thresh > 0) {
        if (cpu_usage >= cpu_thresh) {
            return true;
        } else {
        }
    } else {
    }

    if (mem_thresh > 0) {
        if (mem_usage >= mem_thresh) {
            return true;
        } else {
        }
    } else {
    }

    return false;
}

uint32_t ServiceQua::GetWinLen(const open_app_desc::Flux *flux) {
    uint32_t ret = s_win_len;
    if (flux != NULL) {
        if (flux->has_winlen()) {
            if ((flux->winlen() > 0) && (flux->winlen() <= 30)) {
                ret = flux->winlen();
            }
        }
    }

    return ret;
}

uint32_t ServiceQua::GetErrNum(const open_app_desc::Flux *flux) {
    uint32_t ret = s_threshold;
    if (flux != NULL) {
        if (flux->has_errnum()) {
            if (flux->errnum() > 0) {
                ret = flux->errnum();
            }
        }
    }

    return ret;
}

uint32_t ServiceQua::GetErrNumRate(const open_app_desc::Flux *flux) {
    uint32_t ret = s_flow_threshold;
    if (flux != NULL) {
        if (flux->has_errnumrate()) {
            if ((flux->errnumrate() > 0) && (flux->errnumrate() < 100)) {
                ret = flux->errnumrate();
            }
        }
    }

    return ret;
}

uint32_t ServiceQua::GetCPURate(const open_app_desc::Flux *flux) {
    uint32_t ret = 0;  //  s_cpu_threshold;
    if (flux != NULL) {
        if (flux->has_cpurate()) {
            if ((flux->cpurate() > 0) && (flux->cpurate() < 100)) {
                ret = flux->cpurate();
            }
        }
    }

    return ret;
}

time_t ServiceQua::GetCPUInterval(const open_app_desc::Flux *flux) {
    time_t ret = s_interval;
    if (flux != NULL) {
        if (flux->has_cpuinterval()) {
            if ((flux->cpuinterval() > 0) && (flux->cpuinterval() < 10)) {
                ret = flux->cpuinterval();
            }
        }
    }

    return ret;
}

uint32_t ServiceQua::GetMemRate(const open_app_desc::Flux *flux) {
    uint32_t ret = 0;  //  s_mem_threshold;
    if (flux != NULL) {
        if (flux->has_memrate()) {
            if ((flux->memrate() > 0) && (flux->memrate() < 100)) {
                ret = flux->memrate();
            }
        }
    }

    return ret;
}

string ServiceQua::GetAllPara(const open_app_desc::Flux *flux) {
    stringstream ss;
    ss << " winlen:" << GetWinLen(flux)
       << " ErrNum:" << GetErrNum(flux)
       << " ErrNumRate:" << GetErrNumRate(flux)
       << " GetCPURate:" << GetCPURate(flux)
       << " GetCPUInterval:" << GetCPUInterval(flux)
       << " GetMemRate:" << GetMemRate(flux);

    return ss.str();
}

//  quota
QDATA& ServiceQua::InitKeyQuaAlloc(const string& key, QDATAMAP& qmap) {
    QDATAMAP::iterator it = qmap.find(key);
    if (qmap.end() == it) {
        QDATA qdata;
        qmap.insert(make_pair(key, qdata));
        qmap[key].Init();
    }
    return qmap[key];
}

static PRODATA& InitStrIntMap(map<string, PRODATA>& ipmap, const string& id) {
    map<string, PRODATA>::iterator it = ipmap.find(id);
    if (ipmap.end() == it) {
        PRODATA prodata = {0, 0, 0};
        ipmap.insert(make_pair(id, prodata));
    }

    return ipmap[id];
}

static IDINFO& InitIdInfoMap(map<string, map<string, IDINFO> >& idmap,
                            const string& ip, const string& id) {
    map<string, map<string, IDINFO> >::iterator it = idmap.find(ip);
    if (idmap.end() == it) {
        map<string, IDINFO> tmap;
        idmap.insert(make_pair(ip, tmap));
    }

    map<string, IDINFO>& tmap1 = idmap[ip];
    map<string, IDINFO>::iterator it1 = tmap1.find(id);
    if (tmap1.end() == it1) {
        IDINFO idinfo;
        idinfo.Init();
        tmap1.insert(make_pair(id, idinfo));
    }

    return idmap[ip][id];
}

//  aquota judge id
static bool ValidID(map<string, map<string, IDINFO> >& idmap,
                    const string& ip, const string& id, time_t now,
                    int linenum) {
    //  for test
    //  return true;
    bool ret = false;
    IDINFO& idinfo = InitIdInfoMap(idmap, ip, id);
    map<string, IDINFO>& ipmap = idmap[ip];
    if ((now >= ipmap[id].updatetime) && ((now - ipmap[id].updatetime) < 30)) {
        //  there's data update last 20s
        if ((now > idinfo.firsttime) && ((now - idinfo.firsttime) > (60 * 2))) {
            //  update  over 2 minute
            ret = true;
        } else {
            SF_LOG(LOG_DEBUG, " :%d s_quota_data_time_not_enough:%s, delta:%d",
                linenum, id.c_str(), now - idinfo.firsttime);
            ret = false;
        }
    } else {
        idinfo.firsttime = now;
        SF_LOG(LOG_DEBUG, " :%d s_quota_data_reset:%s", linenum, id.c_str());
        ret = false;
    }
    idinfo.updatetime = now;

    return ret;
}

int ServiceQua::GetTimeKey(time_t ttime, time_t& timekey, int& index) {
    int left = ttime % 60;
    index = left / NUM_PER_INDEX;
    time_t base = ttime - left;
    timekey = base + index * NUM_PER_INDEX;
    return 0;
}

int ServiceQua::InitLocalMap(LOCALMAP& local, const string& id) {
    LOCALMAP::iterator it = local.find(id);
    if (local.end() == it) {
        LOCALDATA data;
        data.Init();
        local.insert(make_pair(id, data));
        //  local[id].Init();
    }
    return 0;
}

int ServiceQua::GetQuotaL5(int env, const open_app_desc::Flux *flux, cl5::QOSREQUEST& aquol5) {
    if (1 == env) {
        aquol5._modid = quotaNorMod;
        aquol5._cmd = quotaNorCmd;
        if ((flux != NULL) && (flux->has_normalenv())) {
            aquol5._modid = strtoul(flux->normalenv().modid().c_str(), NULL, 10);
            aquol5._cmd = strtoul(flux->normalenv().cmdid().c_str(), NULL, 10);
        }
    } else {
        aquol5._modid = quotaTesMod;
        aquol5._cmd = quotaTesCmd;
        if ((flux != NULL) && (flux->has_testenv())) {
            aquol5._modid = strtoul(flux->testenv().modid().c_str(), NULL, 10);
            aquol5._cmd = strtoul(flux->testenv().cmdid().c_str(), NULL, 10);
        }
    }

    return 0;
}

bool ServiceQua::ProcessByRefer(LOCALDATA& local, const string& referid,
                                int32_t& ret, int& deltaquota) {
    bool flag = false;
    int32_t referquota = 0;
    time_t now = time(NULL);
    if (referid.size() > 0) {
        InitLocalMap(s_map_local, referid);
        LOCALDATA& referlocal = s_map_local[referid];
        if ((referlocal.restrainquota > 0)
                && (referlocal.updatetime <= now) && ((now - referlocal.updatetime) < 60 * 5)
                && (referlocal.localquota > 0)) {
            referquota = referlocal.restrainquota * local.totalquota / referlocal.totalquota;
        }
    }

    if (referquota > s_threshold_per_second) {
        flag = true;
        deltaquota = referquota - local.nowsum;  //  for report
        if (local.nowsum > referquota) {
            ret = 2;
        } else {
            ret = 0;
        }
    }

    return flag;
}

bool ServiceQua::ProcessByAver(LOCALDATA& local, int64_t totalquota,
                                int32_t& ret, int& deltaquota) {
    bool flag = false;
    int32_t averquota = 0;
    if (local.totalworkernum > 0) {
        //  averquota = (totalquota / local.totalworkernum)*(100 + QUOTA_FLOAT_PERCENT)/100;
        averquota = totalquota / local.totalworkernum;
    }

    if (averquota > s_threshold_per_second) {
        flag = true;
        deltaquota = averquota - local.nowsum;  //  for report

        int percent = local.nowsum * 100 / averquota;
        if (percent >= (100 + QUOTA_FLOAT_PERCENT)) {
            ret = 2;
        } else {
            if (percent >= 100) {
                int delta = percent - 100;
                if (HitProba(delta)) {
                    ret = 2;
                } else {
                    ret = 0;
                }
            } else {
                ret = 0;
            }
        }
    }

    return flag;
}

// ret, 0: < 100%, 1: >100%, <120% 2: > 120%; set refuse: 3
int32_t ServiceQua::FluxQuota(string& keyid, int32_t groupid,
                                int32_t totalquota, cl5::QOSREQUEST& aquol5) {
    open_app_desc::CmdBase base;
    base.set_type(open_app_desc::BODY_REQUEST);
    base.set_cmd_id(QUOTA_QUERY_CMD);
    open_app_desc::QuotaReq req;
    req.set_key(keyid);
    req.set_totalquota(totalquota);
    req.set_id(0);
    req.set_workernum(1);
    string strreq;
    if (req.SerializeToString(&strreq)) {
        base.set_body(strreq);
        char sendbuff[QUOTA_BUFF_LEN];
        uint32_t total = sizeof(sendbuff);
        if (prototemplate::SetBinaryPack(base, sendbuff, total)) {  //  2
            SppApi::L5ROUTER route;
            route.Init();
            //  route.timeout = 5;

            cl5::QOSREQUEST& qos = route.qos;
            qos._modid = aquol5._modid;
            qos._cmd = aquol5._cmd;
            route.ip = aquol5._host_ip;
            route.port = aquol5._host_port;

            WrapMemo wrapmemo(QUOTA_BUFF_LEN);
            char *buf = wrapmemo.GetMem();
            int buf_size = wrapmemo.GetLen();

            int iRet = SppApi::mt_tcpsendrcv_route(sendbuff, total, buf, buf_size,
                                                    prototemplate::ProtocolCheck, route);
            prototemplate::ProtoPack< open_app_desc::QuotaReq, open_app_desc::QuotaRsp> cmd;
            SF_LOG(LOG_DEBUG, "FluxQuota ip:%s, port:%d, iRet:%d",
                    qos._host_ip.c_str(), qos._host_port, iRet);
            if (0 == iRet) {  //  3
                int ret1 = cmd.ParseRspTmplt(buf + 4, buf_size - 4);
                if (0 == ret1) {
                    const open_app_desc::QuotaRsp& rsp = cmd.spe_rsp;
                    if ((rsp.has_localq()) && (0 == rsp.localq())) {
                        return 2;
                    }

                    if ((rsp.procquota()) && (0 == rsp.procquota())) {
                        return 3;
                    }
                }
            } else {
                SF_LOG(LOG_ERROR, "FluxQuota ip:%s, port:%d, iRet:%d",
                    qos._host_ip.c_str(), qos._host_port, iRet);
            }
        }
    }
    return 0;
}


// ret, 0: < 100%, 1: >100%, <120% 2: > 120%; set refuse: 3
int32_t ServiceQua::CheckQuota(string& keyid, int32_t groupid, int32_t totalquota,
                                cl5::QOSREQUEST& aquol5,
                               string referid, const string& serial) {
    int32_t ret = 0;
    int32_t setret = 0;
    time_t now = time(NULL);
    int sendnum = 0;
    int perdelta = 0;  //  ms
    InitLocalMap(s_map_local, keyid);
    LOCALDATA& local = s_map_local[keyid];
    int deltaquota = 0;

    if (s_now != now) {
        s_now = now;
        s_setsum = 0;
    }

    if (now >= local.updatetime) {   //  general condition
        //  process overflow logic
        //  check total quota vary

        if (local.totalquota != totalquota) {
            //  if (local.totalquota != 0) {
            //  }
            local.totalquota = totalquota;
        }

        if (now != local.now) {  //  reset
            local.now = now;
            if (local.nowsum > local.maxper) {
                local.maxper = local.nowsum;
            }

            if (local.nowsum < local.minper) {
                local.minper = local.nowsum;
            }

            local.nowsum = 1;
            ret = 0;
        } else {
            local.nowsum += 1;
            //  no quota
            //  process heartbeat: the quota service is not available
            bool heartbeat = (now > local.updatetime) && ((now - local.updatetime) > 60 * 5);
            bool threshold = local.localquotaver < s_threshold_per_second;
            if (heartbeat || threshold) {
                if (heartbeat) {
                    local.ClearQuota();
                }
                bool result = ServiceQua::ProcessByRefer(local, referid, ret, deltaquota);
                if (!result) {
                    result = ServiceQua::ProcessByAver(local, local.totalquota, ret, deltaquota);
                    if (!result) {
                        ret = 0;
                    }
                }
            } else {  //  heartbeat ok
                deltaquota = local.localquotaver - local.nowsum;  //  for report
                if ((local.restrainquota > s_threshold_per_second)
                    && (local.nowsum > local.restrainquota)) {
                    ret = 2;
                } else {
                    if ((local.localquotaver > 0) && (local.nowsum > local.localquotaver)) {
                        ret = 1;
                    } else {
                        ret = 0;
                    }
                }
            }
        }

        //  SET modify
        if (ret < 2) {
            if (local.QSetOverLoad(s_setsum))
                setret = 2;
        }

        ///////////////////////////////
        //  update local cache
        ///////////////////////////////
        time_t timekey = 0;
        int index = 0;
        GetTimeKey(now, timekey, index);
        index = index % NUM_PER_MIN;

        if (local.timesum[index].timekey != timekey) {
            local.timesum[index].timekey = timekey;
            local.timesum[index].sum = 0;
            local.timesum[index].refuse = 0;
            local.timesum[index].setrefuse = 0;
            if (ret < 2)
                local.timesum[index].sum = 1;
            else
                local.timesum[index].refuse = 1;

            if (2 == setret)
                local.timesum[index].setrefuse = 1;
        } else {
            if (ret < 2)
                local.timesum[index].sum += 1;
            else
                local.timesum[index].refuse += 1;

            if (2 == setret)
                local.timesum[index].setrefuse += 1;
        }

        // networks --------------------------------------------------------
        time_t delta = now - local.updatetime;
        if ((delta >= REPORT_PERIOD) && (0 == local.lock)) {
            // access quota server
            local.lock = 1;
            open_app_desc::CmdBase base;
            base.set_type(open_app_desc::BODY_REQUEST);
            base.set_cmd_id(QUOTA_QUERY_CMD);
            open_app_desc::QuotaReq req;
            req.set_key(keyid);
            req.set_totalquota(totalquota);
            req.set_localquota(local.localquota);
            req.set_id(groupid);
            req.set_nowsum(local.maxper);
            req.set_workernum(GetGroupsSum());
            if (serial.size() > 0) {
                req.set_serial(serial);
            }

            for (int n = 0; n < NUM_PER_MIN; n++) {
                if (local.timesum[n].timekey > 0) {
                    open_app_desc::ReportItem *item = req.add_reportitem();
                    item->set_timekey(local.timesum[n].timekey);
                    item->set_reportnum(local.timesum[n].sum);
                    item->set_refuse(local.timesum[n].refuse);
                    item->set_setrefuse(local.timesum[n].setrefuse);
                }
            }

            //  SF_LOG(LOG_DEBUG, "ServiceQua::CheckQuota req:%s\n", req.DebugString().c_str());
            string strreq;
            if (req.SerializeToString(&strreq)) {
                base.set_body(strreq);
                char sendbuff[QUOTA_BUFF_LEN];
                uint32_t total = sizeof(sendbuff);
                if (prototemplate::SetBinaryPack(base, sendbuff, total)) {  //  2
                    SppApi::L5ROUTER route;
                    route.Init();

                    cl5::QOSREQUEST& qos = route.qos;
                    qos._modid = aquol5._modid;
                    qos._cmd = aquol5._cmd;

                    WrapMemo wrapmemo(QUOTA_BUFF_LEN);
                    char *buf = wrapmemo.GetMem();
                    int buf_size = wrapmemo.GetLen();

                    int iRet = SppApi::mt_tcpsendrcv_route(sendbuff, total, buf,
                                    buf_size, prototemplate::ProtocolCheck, route);
                    sendnum++;
                    perdelta = route.sendinfo.senditem[0].perdelta;
                    prototemplate::ProtoPack< open_app_desc::QuotaReq, open_app_desc::QuotaRsp> cmd;
                    if (0 == iRet) {  //  3
                        int ret1 = cmd.ParseRspTmplt(buf + 4, buf_size - 4);
                        if (0 == ret1) {
                            local.updatetime = now;
                            const open_app_desc::QuotaRsp& rsp = cmd.spe_rsp;
                            SF_LOG(LOG_DEBUG, "ServiceQua::CheckQuota rsp:%s\n",
                                    rsp.DebugString().c_str());
                            if ((rsp.has_key()) && (rsp.key() == keyid)) {
                                if (rsp.has_localq()) {
                                    uint32_t arrind = (local.index++) % QUOTA_ARRAY_NUM;
                                    local.qarray[arrind] = rsp.localq();
                                    local.localquotaver = local.GetAverQ();
                                    if (local.localquotaver > local.localquotamaxaver) {
                                        local.localquotamaxaver = local.localquotaver;
                                    }
                                    if (local.localquotaver < local.localquotaminaver)
                                        local.localquotaminaver = local.localquotaver;

                                    local.restrainquota = (local.localquotaver *
                                                        (100 + QUOTA_FLOAT_PERCENT)) / 100;

                                    if (local.localquota != rsp.localq()) {
                                        local.localquota = rsp.localq();
                                    }

                                    if (rsp.has_totalworkernum()) {
                                        local.totalworkernum = rsp.totalworkernum();
                                    }

                                    // add set logic
                                    if (rsp.has_setquota()) {
                                        local.setquota = rsp.setquota();
                                    }

                                    if (rsp.has_setsum()) {
                                        local.setsum = rsp.setsum();
                                    }

                                    if (rsp.has_procquota()) {
                                        local.procquota = rsp.procquota();
                                    }
                                    SF_LOG(LOG_DEBUG,
                                    "ServiceQua::CheckQuota local.setquota:%d, local.setsum:%d\n",
                                    local.setquota, local.setsum);
                                }
                            }
                        }
                    } else {
                        if (-9 == iRet) {
                        } else if (-3 == iRet) {
                        }
                    }
                }  //  2
            }
            local.lock = 0;
        }
    }

    //  tnm2 report
    if (sendnum > 0) {
        if (perdelta > local.maxtime) {
            local.maxtime = perdelta;
        }
        local.totalreqtime += perdelta;
        local.reqnum += 1;
    }

    if (deltaquota < 0) {  //  over
        int32_t overq = 0 - deltaquota;
        if (overq > local.overmaxq) {
            local.overmaxq = overq;
        }
    }

    if (deltaquota > 0) {  //  left
        if (deltaquota < local.leftminq)
            local.leftminq = deltaquota;
    }

    //  minute period report
    if (local.localquota > local.localquotamax)
        local.localquotamax = local.localquota;

    if (local.localquota < local.localquotamin)
        local.localquotamin = local.localquota;

    int32_t quotavervary = local.localquotamaxaver - local.localquotaminaver;
    if (quotavervary > local.localquotavervary)
        local.localquotavervary = quotavervary;

    int32_t quotavary = local.localquotamax - local.localquotamin;
    if (quotavary > local.localquotavary)
        local.localquotavary = quotavary;

    int32_t flowvary = local.maxper - local.minper;
    if (flowvary > local.flowvary) {
        local.flowvary = flowvary;
    }

    int64_t nowmin = GetMinute();
    if (nowmin != local.lastreport) {
        //  if ((local.minper > 0) && (local.minper != MIN_HARD_CODE)) {
        //  }
        //  if (local.localquotamax > 0) {
        //  }
        //  if ((local.localquotamin > 0) && (local.localquotamin != MIN_HARD_CODE)) {
        //  }

        local.reqnum = 0;
        local.totalreqtime = 0;
        local.maxtime = 0;

        local.maxper = 0;
        local.minper = MIN_HARD_CODE;
        local.flowvary = 0;

        local.localquotamax = 0;
        local.localquotamin = MIN_HARD_CODE;
        local.localquotavary = 0;

        local.localquotamaxaver = 0;
        local.localquotaminaver = MIN_HARD_CODE;
        local.localquotavervary = 0;

        //  if ((local.leftminq != 0) && (local.leftminq != MIN_HARD_CODE)) {
        //  }
        //  if (local.overmaxq > 0) {
        //  }

        local.leftminq = MIN_HARD_CODE;
        local.overmaxq = 0;
        local.lastreport = nowmin;

        //  if (local.reqnum > 0) {
        //  }
    }


    //  SF_LOG(LOG_DEBUG, "ServiceQua::CheckQuota keyid:%s,
    //  aquota:%llu\n", keyid.c_str(), local.localquota);
    if (2 == setret)
        return 3;

    if (ret < 2)
        s_setsum += 1;

    return ret;
}

static const time_t s_service_runing_time = 60 *  5;

int32_t GetRefuse(const open_app_desc::ReportItem& item) {
    if (item.has_refuse())
        return item.refuse();
    else
        return 0;
}

int32_t GetSetRefuse(const open_app_desc::ReportItem& item) {
    if (item.has_setrefuse())
        return item.setrefuse();
    else
        return 0;
}

int32_t ServiceQua::RealReport(int32_t keytype, const string& keyname,
                                int64_t timekey, const string& setname,
                               TIMEITEM& titem, int32_t quota) {
    //  return 0;
    if (s_report_shutdown) {
        return 0;
    }
    static uint64_t num = 0;
    static string ip;
    static int port = 0;
    num++;
    num = num % 100;

    open_app_desc::CmdBase base;
    base.set_cmd_id(REAL_REPORT_CMD);
    open_app_desc::RealReport req;
    req.set_keytype(keytype);
    req.set_keyname(keyname);
    req.set_timekey(timekey);
    req.set_setname(setname);
    req.set_total(titem.Total());
    req.set_refuse(titem.refuse);
    req.set_setrefuse(titem.setrefuse);
    req.set_real(titem.Real());
    req.set_quota(quota);

    string strreq;
    if (req.SerializeToString(&strreq)) {
        base.set_body(strreq);
        string total;
        base.SerializeToString(&total);
        if (total.size() > 0) {
            if ((0 == port) || (0 == num)) {
                string tmpip = ip;
                int tmpport = port;
                GetL5IpAndPort(s_report_mod, s_report_cmd, ip, port);
                //  if ((tmpip != ip) || (tmpport != port)) {
                //  }
            }
            if ((ip.size() > 0) && (port > 0)) {
                int len = MapSendUDP(ip, port, total, s_map_fd);
                SF_LOG(LOG_DEBUG, "ServiceQua::RealReport len:%d, report:%s\n",
                len, req.DebugString().c_str());
            }
        }
    }
    return 0;
}


int32_t ServiceQua::QuotaReport(const string& actkey, const string& ip,
                                int32_t id, const string& setkey,
                                int32_t aquota, int32_t squota,
                                PRODATA& report, int64_t timekey) {
    if (s_report_shutdown) {
        return 0;
    }

    static uint64_t num = 0;
    static string reportip;
    static int port = 0;
    //  num++;
    num = ++num % 100;

    open_app_desc::CmdBase base;
    base.set_cmd_id(QUOTA_REPORT_CMD);
    open_app_desc::QReport req;
    req.set_actid(actkey);
    req.set_ip(ip);
    req.set_procid(id);
    req.set_timekey(timekey);
    req.set_setname(setkey);
    req.set_actquota(aquota);
    req.set_setquota(squota);
    req.set_total(report.sum + report.refuse);
    req.set_refuse(report.refuse);
    req.set_setrefuse(report.setrefuse);
    req.set_real((report.sum > report.setrefuse) ? (report.sum - report.setrefuse) : 0);


    string strreq;
    if (req.SerializeToString(&strreq)) {
        base.set_body(strreq);
        string total;
        base.SerializeToString(&total);
        if (total.size() > 0) {
            if ((0 == port) || (0 == num)) {
                string tmpip = reportip;
                int tmpport = port;
                GetL5IpAndPort(s_report_mod, s_report_cmd, reportip, port);
                //  if ((tmpip != reportip) || (tmpport != port)) {
                //  }
            }
            if ((reportip.size() > 0) && (port > 0)) {
                int len = MapSendUDP(reportip, port, total, s_map_fd);
                SF_LOG(LOG_DEBUG, "ServiceQua::QuotaReport len:%d, timekey:%llu, report:%s\n",
                        len, timekey, req.DebugString().c_str());
            }
        }
    }
    return 0;
}


int32_t InitAll(TIMEITEM& titem, const open_app_desc::ReportItem& item, time_t timekey) {
    titem.sum = item.reportnum();
    titem.refuse = GetRefuse(item);
    titem.setrefuse = GetSetRefuse(item);
    titem.timekey = timekey;

    return 0;
}

int32_t NormalAddSum(TIMEITEM& titem, const PRODATA& last, const open_app_desc::ReportItem& item,
                     PRODATA *prociddata = NULL) {
    int32_t sum = item.reportnum();
    titem.sum -= last.sum;
    titem.sum += sum;
    if (NULL != prociddata) {
        prociddata->sum = sum;
    }
    return 0;
}

int32_t NormalAddRefuse(TIMEITEM& titem, const PRODATA& last, const open_app_desc::ReportItem& item,
                        PRODATA *prociddata = NULL) {
    int32_t refuse = GetRefuse(item);
    titem.refuse -= last.refuse;
    titem.refuse += refuse;
    if (NULL != prociddata) {
        prociddata->refuse = refuse;
    }
    return 0;
}

int32_t NormalAddSetRefuse(TIMEITEM& titem, const PRODATA& last,
                            const open_app_desc::ReportItem& item,
                           PRODATA *prociddata = NULL) {
    int32_t setrefuse = GetSetRefuse(item);
    titem.setrefuse -= last.setrefuse;
    titem.setrefuse += setrefuse;
    if (prociddata != NULL) {
        prociddata->setrefuse = setrefuse;
    }
    return 0;
}

int32_t NormalAddAll(TIMEITEM& titem, const PRODATA& last, const open_app_desc::ReportItem& item,
                     PRODATA *prociddata = NULL) {
    NormalAddSum(titem, last, item, prociddata);
    NormalAddRefuse(titem, last, item, prociddata);
    NormalAddSetRefuse(titem, last, item, prociddata);
    return 0;
}

int32_t ReSetAddSum(TIMEITEM& titem, const open_app_desc::ReportItem& item,
                    PRODATA *prociddata = NULL,
                    const string *key = NULL) {
    titem.sum += item.reportnum();
    if (NULL != prociddata) {
        //  prociddata->sum += item.reportnum();
        if ((prociddata->sum != 0) && (key != NULL)) {
            stringstream ss;
            ss << "ReSetAddSum timekey:" << item.timekey() << " key:"
            << *key << " his sum:" << prociddata->sum << " report sum:" <<
               item.reportnum();
            SF_LOG(LOG_ERROR, "%s", ss.str().c_str());
        }
        prociddata->sum = item.reportnum();
    }
    return 0;
}

int32_t ReSetAddRefuse(TIMEITEM& titem, const open_app_desc::ReportItem& item,
                        PRODATA *prociddata = NULL,
                       const string *key = NULL) {
    titem.refuse += GetRefuse(item);
    if (NULL != prociddata) {
        //  prociddata->refuse += GetRefuse(item);
        prociddata->refuse = GetRefuse(item);
    }
    return 0;
}

int32_t ReSetAddSetRefuse(TIMEITEM& titem, const open_app_desc::ReportItem& item,
                        PRODATA *prociddata = NULL,
                          const string *key = NULL) {
    titem.setrefuse += GetSetRefuse(item);
    if (NULL != prociddata) {
        //  prociddata->setrefuse += GetSetRefuse(item);
        prociddata->setrefuse = GetSetRefuse(item);
    }
    return 0;
}

int32_t ReSetAddAll(TIMEITEM& titem, const open_app_desc::ReportItem& item,
                    PRODATA *prociddata = NULL,
                    const string *key = NULL) {
    ReSetAddSum(titem, item, prociddata, key);
    ReSetAddRefuse(titem, item, prociddata, key);
    ReSetAddSetRefuse(titem, item, prociddata, key);
    return 0;
}


void ShowFirstReq(const string& ip, const open_app_desc::QuotaReq& req) {
    return;
    bool flag = (req.has_key()) && ("ame_167145" == req.key());
    if (!flag) {
        return;
    }

    stringstream ss;
    ss << "ShowFirstReq IP:" << ip << " key:" << req.key();
    for (int i = 0; i < req.reportitem_size(); i++) {
        const open_app_desc::ReportItem& item = req.reportitem(i);
        //  ss<<endl<<"ShowFirstReq "<<item.reportnum()<<" "<<item.refuse()<<" "<<item.setrefuse();
        ss << endl << "ShowFirstReq " << item.timekey()
        << " num:" << item.reportnum() << " id:" << req.id() << " time:" <<
           __spp_get_now_ms();
    }
    SF_LOG(LOG_ERROR, "%s", ss.str().c_str());
    //  SF_LOG(LOG_ERROR, "%s", req.DebugString().c_str());
    return;
}

int32_t ServiceQua::GetLastQuota(QDATA& procdata, const string& ip, const string& procid) {
    const map<string, map<string, IDINFO> >::iterator it = procdata.mapalloq.find(ip);
    if (it != procdata.mapalloq.end()) {
        map<string, IDINFO>::iterator it1 = it->second.find(procid);
        if (it1 != it->second.end()) {
            return it1->second.aquota;
        }
    }

    return 0;
}
//  act
uint32_t ServiceQua::GetQueryQuota(const QEXTINFO& info, const open_app_desc::QuotaReq& req,
                                   open_app_desc::QuotaRsp& rsp, const open_app_desc::Flux *flux) {
    uint32_t aquota = 0;
    size_t partnernum = 0;
    size_t totalworkernum = 0;
    size_t ipnum = 0;
    int32_t alloctype = 0;
    //  alloctype 0:undo 1:average 2:dynamic 3:heart beat reset 4:history total not enough
    int32_t allocpartnum = 0;
    time_t alloctime = 0;
    int64_t self = 0;
    int64_t total = 0;
    int32_t groupid = 0;
    bool quotavalid = false;

    stringstream ssid;
    ssid << info.remoteip;
    if (req.has_id()) {
        ssid << "_" << req.id();
        groupid = req.id();
    }
    const string& procid = ssid.str();  //  ip_group id

    ShowFirstReq(info.remoteip, req);

    string setkey;
    SETCfgFirst *psetcfgfirst = NULL;
    GetSETCfgFirst(req, info.remoteip, setkey, psetcfgfirst);

    if (s_localhost.size() == 0) {
        GetLocalNormalIp(s_localhost);
    }

    //  get quota
    uint32_t actquota = 0;
    if (flux != NULL) {
        if (flux->has_queryquota()) {
            if (flux->queryquota() > 0) {
                actquota = flux->queryquota();
            }
        }
    } else {
        if ((req.has_totalquota()) && (req.totalquota() > 0)) {
            actquota = req.totalquota();
        }
    }

    int32_t localquota = 0;
    if ((req.has_localquota()) && (req.localquota() > 0)) {
        localquota = req.localquota();
    }

    int hostworkernum = 0;
    if ((req.has_workernum()) && (req.workernum() > 0)) {
        hostworkernum = req.workernum();
    }

    const time_t now = __spp_get_now_s();
    time_t nowkey = 0;
    int nowindex = 0;
    GetTimeKey(now, nowkey, nowindex);

    //  gen key
    if (actquota > 0) {
        string actkey;
        if ((req.has_key()) && (req.key().size() > 0)) {
            actkey = req.key();
        } else if ((req.has_referkey()) && (req.referkey().size() > 0)) {
            actkey = req.referkey();
        }

        if (actkey.size() > 0) {
            rsp.set_key(actkey);

            if (setkey.size() > 0) {
                PROCQ tmp = {procid, 0, setkey};
                s_setproc.insert(tmp);
            }
            QDATA& qdata = InitKeyQuaAlloc(actkey, s_map_quota);  //  act data
            QDATA& procdata = InitKeyQuaAlloc(procid, s_map_proc_quota);  //  proc data

            //  update sum
            for (int i = 0; i < req.reportitem_size(); i++) {
                const open_app_desc::ReportItem& item = req.reportitem(i);
                if ((item.has_timekey()) && (item.timekey() > 0) &&
                    (item.has_reportnum()) && (item.reportnum() > 0)) {
                    int index = 0;
                    time_t timekey = 0;
                    GetTimeKey(item.timekey(), timekey, index);
                    TIMEITEM& titem = qdata.titem[index];  //  total, server
                    if (timekey < titem.timekey) {
                        //  continue;
                    }

                    map<string, PRODATA>& mapidnum = titem.mapidnum;
                    PRODATA& prociddata = InitStrIntMap(mapidnum, procid);  //  history
                    PRODATA last = prociddata;
                    PRODATA reportpro;
                    reportpro.Init(item);
                    if ((0 == titem.timekey) || (timekey > titem.timekey)) {
                        //  for report
                        map<string, PRODATA>::iterator itl = titem.mapidnum.begin();
                        for (; itl != titem.mapidnum.end(); itl++) {
                            const string& leftid = itl->first;
                            PRODATA& lastpro = itl->second;
                            vector<string> vecid;
                            SplitString(leftid, "_", vecid);
                            if (vecid.size() >= 2) {
                                uint32_t tactquota = GetLastQuota(qdata, vecid[0], leftid);
                                QDATA& leftprocdata = InitKeyQuaAlloc(leftid,
                                s_map_proc_quota);  //  proc data
                                uint32_t tsetquota = GetLastQuota(leftprocdata, vecid[0], leftid);
                                string tmpsetkey;
                                GetSetKey(vecid[0], tmpsetkey);
                                QuotaReport(actkey, vecid[0], atoi(vecid[1].c_str()),
                                tmpsetkey, tactquota, tsetquota, lastpro, titem.timekey);
                            }
                        }

                        RealReport(1, actkey, timekey, setkey, titem, actquota);
                        RealReport(1, actkey, timekey, "all", titem, actquota);
                        InitAll(titem, item, timekey);
                        //  history data clear, if peer reset and pid changes, no problem
                        mapidnum.clear();
                        mapidnum.insert(make_pair(procid, reportpro));
                    } else if (timekey == titem.timekey) {
                        if (0 == last.sum || reportpro.sum < last.sum) {  //  start? reset
                            ReSetAddAll(titem, item, &prociddata, &procid);
                        } else if (reportpro.sum >= last.sum) {  //  normal
                            NormalAddAll(titem, last, item, &prociddata);
                        }
                    }

                    //  set data
                    if (psetcfgfirst) {
                        TIMEITEM& stitem = psetcfgfirst->timesum[index];
                        if ((0 == stitem.timekey) || (timekey > stitem.timekey)) {
                            InitAll(stitem, item, timekey);
                        } else if (timekey == stitem.timekey) {
                            if (0 == last.sum || reportpro.sum < last.sum) {  //  start? reset
                                ReSetAddAll(psetcfgfirst->timesum[index], item);
                            } else if (reportpro.sum >= last.sum) {  //  normal
                                NormalAddAll(psetcfgfirst->timesum[index], last, item);
                            }
                        }
                    }

                    //  for set alloc logic
                    TIMEITEM& pitem = procdata.titem[index];
                    if ((0 == pitem.timekey) || (timekey > pitem.timekey)) {
                        InitAll(pitem, item, timekey);
                        //  history data clear, if peer reset and pid changes, no problem
                    } else if (timekey == pitem.timekey) {
                        if (0 == last.sum || reportpro.sum < last.sum) {  //  start? reset?
                            ReSetAddAll(pitem, item);
                        } else if (reportpro.sum >= last.sum) {  //  normal
                            NormalAddAll(pitem, last, item);
                        }
                    }
                }
            }

            //  prevent remote change pid
            qdata.CleanReportId(info.remoteip);
            procdata.CleanReportId(info.remoteip);
            procdata.updatetime = now;

            //  to clear

            //  alloc act quota
            quotavalid = ValidID(qdata.mapalloq, info.remoteip, procid, now, __LINE__);
            if (quotavalid) {
                int32_t historyself = 0;
                int32_t historytotal = 0;
                int32_t parternum = 0;
                int32_t validnum = 0;
                int beindex = nowindex;
                for (size_t l = 0; l < sizeof(qdata.titem) / sizeof(TIMEITEM); l++) {
                    TIMEITEM& targetitem = qdata.titem[beindex];
                    if ((targetitem.timekey < nowkey) &&
                        (nowkey - targetitem.timekey) >= FINE_INTERVAL) {  //  20 seconds ago
                        PRODATA& tdata = InitStrIntMap(targetitem.mapidnum, procid);
                        if (tdata.sum > 0) {
                            historyself += tdata.sum;
                            historytotal += targetitem.sum;
                            parternum = targetitem.mapidnum.size();
                            validnum++;
                        }
                    }
                    beindex = (beindex - 1 + NUM_PER_MIN) % NUM_PER_MIN;
                }

                if ((validnum >= 2) && (historytotal > QUOTA_ALLOC_MIN)) {  //   need report data
                    //  SF_LOG(LOG_DEBUG, "historytotal:%llu, aver:%llu,
                    //  validnum:%d", historytotal, historytotal/(validnum*10), validnum);
                    alloctype = 1;
                    aquota = (actquota / parternum);  // average
                    allocpartnum = parternum;
                    alloctime = qdata.titem[nowindex].timekey;
                    //  if (0 == localquota) {
                    //  }

                    if (historyself > 0) {
                        alloctype = 2;
                        self = historyself;
                        total = historytotal;
                        aquota = actquota * (historyself) / historytotal;
                    }
                } else {
                    alloctype = 4;
                    aquota = 0;
                    allocpartnum = parternum;
                    alloctime = qdata.titem[nowindex].timekey;
                }
            }

            //  aquota
            //  if (qdata.mapalloq[info.remoteip][procid].aquota != aquota) {
            //  }
            qdata.mapalloq[info.remoteip][procid].aquota = aquota;

            qdata.AddWorkerItem(info.remoteip, hostworkernum, now);
            qdata.updatetime = now;
            partnernum = qdata.mapalloq.size();  //  peer number
            totalworkernum = qdata.GetWorkerNum();
            ipnum = qdata.mapworker.size();

            if (psetcfgfirst)
                psetcfgfirst->workernum = qdata.GetWorkerNum();

            //  stringstream ss;
            //  SF_LOG(LOG_DEBUG, "\nkey:%s,
            //  GetQueryQuota:%s\n", key.c_str(), qdata.ShowAll(ss).str().c_str());
        }
    }

    if (req.has_serial() && (req.serial().size() > 0)) {
        rsp.set_serial(req.serial());
    }

    if ((now > info.uptime) && ((now - info.uptime) > s_service_runing_time)) {
    } else {
        alloctype = 3;
        aquota = 0;
    }

    //  access second
    if (psetcfgfirst) {
        time_t delta = now - psetcfgfirst->updatetime;
        if ((delta >= 1 || s_setproc.size() > 30) && 0 == psetcfgfirst->lock) {
            psetcfgfirst->updatetime = now;
            psetcfgfirst->lock = 1;

            //  set
            open_app_desc::CmdBase base;
            base.set_type(open_app_desc::BODY_REQUEST);
            base.set_cmd_id(SECOND_QUOTA_CMD);
            open_app_desc::SetQuotaReq secreq;
            secreq.set_setkey(setkey);
            //  secreq.set_id(GetGroupId());// modify pid
            secreq.set_id(g_proxy_port * 1000 + GetGroupId());
            secreq.set_workernum(psetcfgfirst->workernum);

            //  SET
            for (size_t i = 0; i < sizeof(psetcfgfirst->timesum) / sizeof(TIMEITEM); i++) {
                open_app_desc::ReportItem *item = secreq.add_itemlist();
                const TIMEITEM& tmp = psetcfgfirst->timesum[i];
                item->set_timekey(tmp.timekey);
                item->set_reportnum(tmp.sum);
                item->set_refuse(tmp.refuse);
                item->set_setrefuse(tmp.setrefuse);
            }

            //  SET Proc
            std::set<PROCQ>::iterator it = s_setproc.begin();
            for (; it != s_setproc.end(); it++) {
                open_app_desc::ProcReport *pr = secreq.add_procitem();
                pr->set_procid(it->procid);
                pr->set_setkey(it->setkey);
                QDATA& pd = s_map_proc_quota[it->procid];
                for (int h = 0; h < NUM_PER_MIN; h++) {
                    TIMEITEM& pitem = pd.titem[h];
                    open_app_desc::ReportItem *item = pr->add_proclist();
                    item->set_timekey(pitem.timekey);
                    item->set_reportnum(pitem.sum);
                    item->set_refuse(pitem.refuse);
                    item->set_setrefuse(pitem.setrefuse);
                    //  SF_LOG(LOG_DEBUG, "first proc titem: %d\n", pitem.sum);
                }
            }
            s_setproc.clear();

            //  SF_LOG(LOG_DEBUG, "ServiceQua::SetQuotaReq req:%s\n", req.DebugString().c_str());
            string strreq;
            if (secreq.SerializeToString(&strreq)) {
                base.set_body(strreq);
                char sendbuff[QUOTA_BUFF_LEN] = {0};
                uint32_t total = sizeof(sendbuff);
                if (prototemplate::SetBinaryPack(base, sendbuff, total)) {  //  2
                    SppApi::L5ROUTER route;
                    route.Init();

                    route.ip = s_localhost;
                    route.port = 19106;
                    //  cl5::QOSREQUEST &qos = route.qos;
                    //  qos._modid = 64366017;
                    //  qos._cmd = 131072;

                    WrapMemo wrapmemo(QUOTA_BUFF_LEN);
                    char *buf = wrapmemo.GetMem();
                    int buf_size = wrapmemo.GetLen();

                    int iRet = SppApi::mt_tcpsendrcv_route(sendbuff, total,
                                buf, buf_size, prototemplate::ProtocolCheck, route);
                    prototemplate::ProtoPack<open_app_desc::SetQuotaReq,
                    open_app_desc::SetQuotaRsp> cmd;
                    if (0 == iRet) {  //  3
                        int ret1 = cmd.ParseRspTmplt(buf + 4, buf_size - 4);
                        if (0 == ret1) {
                            psetcfgfirst->updatetime = now;
                            const open_app_desc::SetQuotaRsp& rsp = cmd.spe_rsp;
                            if (rsp.has_setkey() && rsp.setkey() == setkey) {
                                if (rsp.has_setsum() && rsp.setsum() > 0)
                                    psetcfgfirst->setsum = rsp.setsum();

                                for (int f = 0; f < rsp.procq_size(); f++) {
                                    const open_app_desc::ProcQ& procq = rsp.procq(f);
                                    if (procq.has_procid() > 0 && procq.procid().size() > 0
                                        && procq.has_procquota() && procq.procquota() > 0) {
                                        QDATA& pd = s_map_proc_quota[procq.procid()];
                                        string procip;
                                        if (GetReportIP(procq.procid(), procip) > 0) {
                                            if (ValidID(pd.mapalloq, procip, procq.procid(),
                                            now,  __LINE__)) {
                                                pd.mapalloq[procip][procq.procid()].aquota =
                                                procq.procquota();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                    }
                }
            }
            psetcfgfirst->lock = 0;
        }
    }

    rsp.set_localq(aquota);
    rsp.set_totalq(actquota);
    rsp.set_partnernum(partnernum);
    rsp.set_ipnum(ipnum);
    rsp.set_alloctype(alloctype);
    rsp.set_allocpartnum(allocpartnum);
    rsp.set_alloctime(alloctime);
    rsp.set_self(self);
    rsp.set_total(total);
    rsp.set_totalworkernum(totalworkernum);
    if (req.has_nowsum()) {
        rsp.set_nowsum(req.nowsum());
    }

    //  find result
    QDATA& resprocdata = InitKeyQuaAlloc(procid, s_map_proc_quota);
    uint32_t thequota = GetLastQuota(resprocdata, info.remoteip, procid);
    if (!quotavalid) {
        thequota = 0;
    }
    rsp.set_procquota(thequota);

    //  add set logic
    if (psetcfgfirst) {
        if (psetcfgfirst->setquota > 0) {
            rsp.set_setquota(psetcfgfirst->setquota);
        }
        if (psetcfgfirst->setsum > 0) {
            rsp.set_setsum(psetcfgfirst->setsum);
        }
    }
    //  SF_LOG(LOG_DEBUG, "ServiceQua::GetQueryQuota QuotaRsp:%s\n", rsp.DebugString().c_str());

    return aquota;
}

void ShowSecondReq(const open_app_desc::SetQuotaReq& req) {
    stringstream ss;
    ss << "SecondSet setkey:" << req.setkey()
    << " workernum:" << req.workernum() << " id:" << req.id();
    for (int i = 0; i < req.itemlist_size(); i++) {
        const open_app_desc::ReportItem& item = req.itemlist(i);
        ss << endl << "SecondSet:" << i << " "
        << item.reportnum() << " " << item.refuse() << " " << item.setrefuse();
    }

    for (int m = 0; m < req.procitem_size(); m++) {
        ss << endl << "SecondProc ";
        const open_app_desc::ProcReport& pr = req.procitem(m);
        ss << pr.procid() << " " << pr.setkey();
        for (int n = 0; n < pr.proclist_size(); n++) {
            const open_app_desc::ReportItem& item = pr.proclist(n);
            ss << endl << "SecondProc " << item.reportnum()
            << " " << item.refuse() << " " << item.setrefuse();
        }
    }
    SF_LOG(LOG_DEBUG, "%s", ss.str().c_str());
    return;
}


///////////////////////////////////////////////////////////////////////////////////
uint32_t ServiceQua::ProSETQuota(const open_app_desc::SetQuotaReq& req, const string& firstip,
                                 open_app_desc::SetQuotaRsp& rsp) {
    //  gen key
    string setkey;
    SETCfgFirst *psetcfgfirst = NULL;
    GetSETCfgFirst(req, "", setkey, psetcfgfirst);
    uint32_t feaid = 0;
    uint32_t refuseid = 0;
    uint32_t setrefuseid = 0;
    uint32_t realid = 0;
    if (psetcfgfirst) {
        feaid = psetcfgfirst->feaid;
        refuseid = psetcfgfirst->refuseid;
        setrefuseid = psetcfgfirst->setrefuseid;
        realid = psetcfgfirst->realid;
    }

    if (s_localhost.size() == 0) {
        GetLocalNormalIp(s_localhost);
    }

    stringstream ssid;
    ssid << firstip;  //  first ip
    if (req.has_id()) {
        ssid << "_" << req.id();  //  group id
    }
    const string& firstid = ssid.str();
    const time_t now = __spp_get_now_s();
    time_t nowkey = 0;
    int nowindex = 0;
    GetTimeKey(now, nowkey, nowindex);
    ShowSecondReq(req);

    //  for procid
    vector<PROCQ> vecpq;
    for (int u = 0; u < req.procitem_size(); u++) {
        const open_app_desc::ProcReport& procitem = req.procitem(u);
        const string& procid = procitem.procid();
        if (0 == procid.size())
            continue;

        string tmpsetkey;
        if (procitem.has_setkey()) {
            tmpsetkey = procitem.setkey();
        }

        string reportid = procid + "_" + firstid;
        QDATA& setprocdata = InitKeyQuaAlloc(procid, s_map_setprocdata);
        for (int i = 0; i < procitem.proclist_size(); i++) {
            const open_app_desc::ReportItem& item = procitem.proclist(i);
            if ((item.has_timekey()) && (item.timekey() > 0) &&
                (item.has_reportnum()) && (item.reportnum() > 0)) {
                int index = 0;
                time_t timekey = 0;
                GetTimeKey(item.timekey(), timekey, index);
                PRODATA reportpro;
                reportpro.Init(item);
                TIMEITEM& titem = setprocdata.titem[index];
                if (timekey < titem.timekey) {
                    continue;
                }
                map<string, PRODATA>& mapidnum = titem.mapidnum;
                PRODATA& tmpprocdata = InitStrIntMap(titem.mapidnum, reportid);
                if ((0 == titem.timekey) || (timekey > titem.timekey)) {
                    RealReport(2, procid, timekey, tmpsetkey, titem);
                    InitAll(titem, item, timekey);
                    //  history data clear, if peer reset and pid changes, no problem
                    mapidnum.clear();
                    mapidnum.insert(make_pair(reportid, reportpro));
                } else if (timekey == titem.timekey) {
                    if (0 == tmpprocdata.sum || reportpro.sum < tmpprocdata.sum) {  //  start? reset
                        ReSetAddAll(titem, item, &tmpprocdata, &reportid);
                    } else if (reportpro.sum >= tmpprocdata.sum) {  //  normal
                        NormalAddAll(titem, tmpprocdata, item, &tmpprocdata);
                    }
                }
            }
        }

        //  prepare to alloc quota
        string procip;
        if (GetReportIP(procid, procip) > 0) {
            uint32_t procvalid = 0;
            if (ValidID(setprocdata.mapalloq, procip, procid, now,  __LINE__)) {
                int32_t historyvalid = 0;
                int32_t validnum = 0;
                int beindex = nowindex;
                for (size_t l = 0; l < sizeof(setprocdata.titem) / sizeof(TIMEITEM); l++) {
                    TIMEITEM& targetitem = setprocdata.titem[beindex];
                    if ((targetitem.timekey < nowkey) &&
                    (nowkey - targetitem.timekey) >= FINE_INTERVAL) {  //  20 seconds ago
                        if (targetitem.sum > 0) {
                            historyvalid += targetitem.sum;
                            validnum++;
                            // break;
                        }
                    }
                    beindex = (beindex - 1 + NUM_PER_MIN) % NUM_PER_MIN;
                }

                if ((validnum >= 1) && (historyvalid > QUOTA_ALLOC_MIN)) {  //  need report data
                    procvalid = historyvalid / (10 * validnum);
                }
            }

            setprocdata.updatetime = now;
            string tmpsetkey;
            if (procitem.has_setkey()) {
                tmpsetkey = procitem.setkey();
            }
            PROCQ tmpPQ = {procid, procvalid, tmpsetkey};
            vecpq.push_back(tmpPQ);
        }
        setprocdata.CleanReportId(procip);
    }

    //  set
    if (setkey.size() > 0) {
        rsp.set_setkey(setkey);
        QDATA& setdata = InitKeyQuaAlloc(setkey, s_map_setsumdata);
        setdata.setfirstvalid = false;
        if (req.has_workernum())
            setdata.workernum = req.workernum();

        //  update sum
        for (int i = 0; i < req.itemlist_size(); i++) {
            const open_app_desc::ReportItem& item = req.itemlist(i);
            if ((item.has_timekey()) && (item.timekey() > 0)
                && (item.has_reportnum()) && (item.reportnum() > 0)) {
                int index = 0;
                time_t timekey = 0;
                GetTimeKey(item.timekey(), timekey, index);
                TIMEITEM& titem = setdata.titem[index];
                if (timekey < titem.timekey) {
                    continue;
                }

                PRODATA reportpro;
                reportpro.Init(item);
                map<string, PRODATA>& mapidnum = titem.mapidnum;
                PRODATA& tmpprocdata = InitStrIntMap(titem.mapidnum, firstid);
                if ((0 == titem.timekey) || (timekey > titem.timekey)) {
                    //  add report
                    RealReport(3, setkey, timekey, setkey, titem);
                    //  uint32_t real = (titem.sum>titem.setrefuse)?(titem.sum-titem.setrefuse):0;

                    //  history data clear, if peer reset and pid changes, no problem
                    InitAll(titem, item, timekey);
                    mapidnum.clear();
                    mapidnum.insert(make_pair(firstid, reportpro));
                } else if (timekey == titem.timekey) {
                    if (0 == tmpprocdata.sum || reportpro.sum < tmpprocdata.sum) {
                        //  start ? reset
                        ReSetAddAll(titem, item, &tmpprocdata, &firstid);
                    } else if (reportpro.sum >= tmpprocdata.sum) {  //  normal
                        NormalAddAll(titem, tmpprocdata, item, &tmpprocdata);
                    }
                }
            }
        }

        //  alloc quota
        int32_t validnum = 0;
        int32_t setrealnum = 0;
        int32_t actrefuse = 0;
        int32_t historyvalid = 0;
        int32_t historyrefuse = 0;
        int32_t historysetrefuse = 0;
        if (ValidID(setdata.mapalloq, firstip, firstid, now,  __LINE__)) {
            int beindex = nowindex;
            for (size_t l = 0; l < sizeof(setdata.titem) / sizeof(TIMEITEM); l++) {
                TIMEITEM& targetitem = setdata.titem[beindex];
                if ((targetitem.timekey < nowkey)
                && (nowkey - targetitem.timekey) >= FINE_INTERVAL) {  //  20 seconds ago
                    PRODATA& tmpprocdata = InitStrIntMap(targetitem.mapidnum, firstid);
                    if (tmpprocdata.sum > 0) {
                        historyvalid += targetitem.sum;
                        historyrefuse += targetitem.refuse;
                        historysetrefuse += targetitem.setrefuse;
                        validnum++;
                        //  break;
                    }
                }
                beindex = (beindex - 1 + NUM_PER_MIN) % NUM_PER_MIN;
            }
            int32_t historytotal = historyvalid + historyrefuse;
            if ((validnum >= 1) && (historytotal > QUOTA_ALLOC_MIN)) {  //  need report data
                setdata.setfirstvalid = true;
                setdata.setnowsum = historytotal / (10 * validnum);
                setdata.setrefuse = historysetrefuse / (10 * validnum);
                setdata.setvalid = historyvalid / (10 * validnum);
                setrealnum = (setdata.setvalid > setdata.setrefuse)
                        ? (setdata.setvalid - setdata.setrefuse) : 0;
                actrefuse = historyrefuse / (10 * validnum);
                if (psetcfgfirst)
                    setdata.setquota = psetcfgfirst->setquota;
            }
        } else {
        }
        setdata.updatetime = now;
        //  setdata.CleanReportId(firstip);

        rsp.set_setsum(setdata.GetSetRealNum());
        SF_LOG(LOG_DEBUG,
               "AVERSUM setname:%s, total:%d, procnum:%d, realnum:%d,
               setvalid:%d, refuse:%d, setrefuse:%d, setquota:%d\n",
               setkey.c_str(), setdata.setnowsum, setdata.workernum,
               setrealnum, setdata.setvalid, actrefuse, setdata.setrefuse,
               setdata.setquota);
    }

    for (size_t m = 0; m < vecpq.size(); m++) {
        PROCQ& procq = vecpq[m];
        const string& tmpsetkey = procq.setkey;
        if (tmpsetkey.size() == 0)
            continue;

        if (s_map_setsumdata.find(tmpsetkey) != s_map_setsumdata.end()) {
            QDATA& tmpqdata = s_map_setsumdata[tmpsetkey];
            open_app_desc::ProcQ *pq = rsp.add_procq();
            pq->set_procid(procq.procid);
            int32_t procquota = tmpqdata.GetProcQ(procq.procvalid);
            pq->set_procquota(procquota > 0 ? procquota : (tmpqdata.GetAverQ()));
            SF_LOG(LOG_DEBUG,
                   "PROCSUM setname:%s, procnum:%d, procid:%s,
                   procquota:%d, averq:%d, procvalid:%d, setvalid:%d, setquota:%d\n",
                   tmpsetkey.c_str(), tmpqdata.workernum, procq.procid.c_str(),
                   procquota, tmpqdata.GetAverQ(), procq.procvalid,
                   tmpqdata.setvalid, tmpqdata.setquota);
        }
    }

    return 0;
}

//   clear key

ServiceQua::WindItem ServiceQua::s_winditem;
void ServiceQua::LoopEnter() {
    return;
}

void ServiceQua::LoopLeave() {
    WindItem& wi = ServiceQua::s_winditem;
    int64_t ttime = GetMSTime();

    if (0 == wi.looplast) {
        wi.loopdelta = 0;
    } else {
        wi.loopdelta = ttime - wi.looplast;
    }

    if (wi.loopdelta > wi.loopdeltamax) {
        wi.loopdeltamax = wi.loopdelta;
    }

    wi.looplast = ttime;
    return;
}

void ServiceQua::FlowEnter() {
    WindItem& wi = ServiceQua::s_winditem;
    int64_t ident = GetMinute();  //  ttime/(24*60);

    if (wi.ident != ident) {
        wi.ident = ident;
        wi.loopdeltamax = 0;
        wi.reqtimemax = 0;
    }
    return;
}

void ServiceQua::FlowLeave(int64_t deltatime) {
    WindItem& wi = ServiceQua::s_winditem;
    if (deltatime > wi.reqtimemax) {
        wi.reqtimemax = deltatime;
    }
    return;
}

//  Add SET
#define INDEX_DIFF_TIME (60*3)
const char *tmplt_flag_err[] = { "init value",  //   0
                                 "the first success",
                                 "update success",
                                 "update too fre",
                                 "in memory, no load",
                                 "update fail",
                                 "the first load error"
                               };
uint32_t tmplt_flag_err_len = sizeof(tmplt_flag_err) / sizeof(char *);

//  SET
SET_MAP g_mapSET;   //  clean ?
TMPLT_DEFINE_FUNC(LoadSETInfo, std::string, open_app_desc::SetMacInfo, SET_ARR)

//  SET FILE
SETFILE_MAP g_mapSETFile;
TMPLT_DEFINE_FUNC(LoadSETFileInfo, std::string, open_app_desc::SetFileInfo, SETFILE_ARR)

int ServiceQua::LoadSETInfoWrap() {
    static time_t last = 0;
    time_t now = __spp_get_now_s();

    if (now - last < 1 * 60) {
        return 0;
    } else {
        last = now;
    }

    open_app_desc::SetFileInfo *pfdata = NULL;
    int64_t fdelta = 0;
    string fkeyname = SET_CFG_FILENAME;
    string fpath = SET_CFG_PATH + fkeyname;
    int findex;
    uint32_t fflag  = 0;
    int fret = LoadSETFileInfo(fpath, fkeyname, pfdata, findex, g_mapSETFile, fflag, fdelta);
    if (0 != fret) {
        return 0;
    }

    if (4 == fflag) {
        return 0;
    }

    if (0 == fret  && (1 == fflag || 2 == fflag)) {
        SF_LOG(LOG_ERROR, "LoadSETFileInfo fpath:%s, ret:%d flag:%d, delta:%llu, %s\n",
        fpath.c_str(), fret, fflag, fdelta,
               pfdata->DebugString().c_str());
    }

    const open_app_desc::SetFileInfo& setfileinfo = *pfdata;
    string filepath = SET_CFG_PATH;
    if (setfileinfo.has_fpath() && setfileinfo.fpath().size() > 0)
        filepath = setfileinfo.fpath();

    for (int i = 0; i < setfileinfo.fname_size(); i++) {
        const string& keyname = setfileinfo.fname(i);
        if (0 == keyname.size()) {
            continue;
        }

        open_app_desc::SetMacInfo *pdata = NULL;
        int64_t delta = 0;
        string path = filepath + keyname;
        int index;
        uint32_t flag  = 0;
        int ret = LoadSETInfo(path, keyname, pdata, index, g_mapSET, flag, delta);

        if (0 == ret  && (1 == flag || 2 == flag)) {
            SF_LOG(LOG_ERROR, "LoadSETInfoWrap path:%s, ret:%d flag:%d, delta:%llu, %s\n",
            path.c_str(), ret, flag, delta,
                   pdata->DebugString().c_str());
            const open_app_desc::SetMacInfo& setmacinfo = *pdata;
            for (int i = 0; i < setmacinfo.itemlist_size(); i++) {
                const open_app_desc::SetItem& item = setmacinfo.itemlist(i);
                if (item.setkey().size() > 0) {
                    map<string, SETCfgFirst>::iterator it =
                    ServiceQua::mapSETCfgFirst.find(item.setkey());
                    if (ServiceQua::mapSETCfgFirst.end() == it) {
                        SETCfgFirst tmpsetinfo;
                        tmpsetinfo.Init();
                        ServiceQua::mapSETCfgFirst.insert(make_pair(item.setkey(), tmpsetinfo));
                    }

                    SETCfgFirst& setcfg = ServiceQua::mapSETCfgFirst[item.setkey()];
                    setcfg.key = GET_PRO_STR(item, setkey);
                    setcfg.setquota = GET_PRO_VAL(item, setquota);
                    setcfg.feaid = GET_PRO_VAL(item, feaid);
                    setcfg.refuseid = GET_PRO_VAL(item, refuseid);
                    setcfg.setrefuseid = GET_PRO_VAL(item, setrefuseid);
                    setcfg.realid = GET_PRO_VAL(item, realid);

                    for (int n = 0; n < item.iplist_size(); n++) {
                        if (item.iplist(n).size() > 0) {
                            ServiceQua::mapSETIPCfg[item.iplist(n)] = item.setkey();
                        }
                    }
                }
            }
        }
    }
    return 0;
}


