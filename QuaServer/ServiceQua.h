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
#ifndef FLUX_QUASERVER_SERVICEQUA_H_
#define FLUX_QUASERVER_SERVICEQUA_H_

#include <map>
#include <sstream>
#include "TimeAPI.h"
#include "syncincl.h"
#include "sppincl.h"
#include "open_app_desc.pb.h"
#include "StringProcess.h"
#include "FileDataMeta.h"

static const time_t s_offline_interval = 60 * 60 * 24 * 1;
static const uint32_t s_offline_begin = 16;
static const time_t s_expire = (60 * 60 * 24 * 3);

enum unify_status {
    UNKNOWN_ERR = -10000000,
    OVER_FLOW = -77777777,
    OVER_LOAD = -88888888,
    NET_ERROR = -99999999
};


//  add for set
static const int32_t s_report_mod = 64283201;
static const int32_t s_report_cmd = 458752;
static const bool    s_report_shutdown = true;


#define win_num 30
const uint32_t s_win_len = 6;
const uint32_t s_threshold = 50;  //  absolute quantity TODO
const time_t s_pasttime = 30;  //  the past time: second
const uint32_t s_flow_threshold = 10;  //  err rate %
const uint32_t s_ratio = 100;

//  quota
const int64_t s_threshold_per_second = 10;
static const uint32_t quotaNorMod = 64283201;
static const uint32_t quotaNorCmd = 393216;

static const uint32_t quotaTesMod = 64283201;
static const uint32_t quotaTesCmd = 327680;

bool HitProba(uint32_t target);

typedef struct {
    time_t ttime;
    int64_t suc;
    int64_t fail;
    void Init() {
        memset(this, 0, sizeof(*this));
        return;
    }
} QUAITEM;

typedef struct {
    QUAITEM item[win_num];
    time_t updatetime;
    void Init() {
        memset(this, 0, sizeof(*this));
        return;
    }
} QUAWIN;

class MTItem {
 public:
    int64_t mtnum;
    int64_t mttotal;
    int64_t mtmax;
    int64_t mtcpumax;
    int64_t mtmemmax;
    //  int64_t aquota;

    int64_t ident;
    MTItem() {
        mtnum  = 0;
        mttotal = 0;
        mtmax = 0;
        //  aquota = 0;
        mtcpumax = 0;
        mtmemmax = 0;

        ident = GetMinute();
    }

    ~MTItem() {
    }
};

//  quota
#define NUM_PER_MIN 6
#define NUM_PER_INDEX (10)
#define REPORT_PERIOD 5  //  seconds
#define QUOTA_BUFF_LEN (1024*4)
#define FINE_INTERVAL (20)

//  #define QUOTA_FLOAT_PERCENT (10)
#define QUOTA_FLOAT_PERCENT (0)
#define QUOTA_ALLOC_MIN 10  //  (500) TODO

int32_t GetRefuse(const open_app_desc::ReportItem& item);
int32_t GetSetRefuse(const open_app_desc::ReportItem& item);

//  processor data
typedef struct {
    int32_t sum;
    int32_t refuse;
    int32_t setrefuse;
    void Init() {
        sum = 0;
        refuse = 0;
        setrefuse = 0;
        return;
    }

    int32_t Init(const open_app_desc::ReportItem& item) {
        sum = item.reportnum();
        refuse = GetRefuse(item);
        setrefuse = GetSetRefuse(item);

        return 0;
    }

    int32_t Total() {
        return sum + refuse;
    }

    int32_t Real() {
        return (sum > setrefuse) ? (sum - setrefuse) : 0;
    }
} PRODATA;

//  server
typedef struct {
    time_t timekey;
    int32_t sum;
    int32_t refuse;
    int32_t setrefuse;
    map<string, PRODATA> mapidnum;  //  ip_group:num
    //  map<string, PRODATA> lastmapidnum;
    //  time_t lasttimekey;
    void Init() {
        timekey = 0;
        sum = 0;
        refuse = 0;
        setrefuse = 0;
        //  lasttimekey = 0;
    }

    /*
    void Init(const open_app_desc::ReportItem &item){
    	Init();
    	if(item.has_timekey())
    		timekey = item.timekey();
    	if(item.has_reportnum())
    		sum = item.reportnum();
    	if(item.has_refuse())
    		refuse = item.refuse();
    	if(item.has_setrefuse())
    		setrefuse = item.setrefuse();
    	return;
    }
    */

    void GetInfo(std::stringstream& ss) {
        ss << std::endl << "timekey:" << timekey
           << std::endl << "sum:" << sum;
        map<string, PRODATA>::iterator it = mapidnum.begin();
        for (; it != mapidnum.end(); it++) {
            ss << std::endl << "id:" << it->first << " sum:"
            << it->second.sum << " refuse:" << it->second.refuse;
        }
        ss << std::endl;
        return;
    }

    int32_t Total() {
        return sum + refuse;
    }

    int32_t Real() {
        return (sum > setrefuse) ? (sum - setrefuse) : 0;
    }
} TIMEITEM;

typedef struct {
    uint32_t aquota;
    time_t updatetime;  //  for report id
    time_t firsttime;   //  for report id
    void Init() {
        aquota = 0;
        updatetime = 0;
        firsttime = 0;
        return;
    }
    void GetInfo(std::stringstream& ss) {
        ss << "aquota:" << aquota
           << " updatetime:" << updatetime
           << " firsttime:" << firsttime;
        return;
    }
} IDINFO;

typedef struct {
    int     workernum;
    time_t  updatetime;
    void Init() {
        workernum = 0;
        updatetime = 0;
        return;
    }
} WorkerInfo;

typedef struct sPROCQ {
    std::string procid;
    uint32_t procvalid;
    std::string setkey;
    bool operator<(const struct sPROCQ& right)const {
        return this->procid < right.procid;
    }
} PROCQ;

typedef struct {
    TIMEITEM titem[NUM_PER_MIN];  //  0:0-9 1:10-19 ....... 5:50-59
    map<string, map<string, IDINFO> > mapalloq;  //  ip  procid  quota
    map<string, WorkerInfo> mapworker;  //  report ip/workernum, for default
    time_t updatetime;  //  heartbeat for clean this
    uint32_t setvalid;  //  for SET
    uint32_t setquota;  //  for SET
    uint32_t setrefuse;  //  for SET
    uint32_t setnowsum;  //  for SET
    uint32_t workernum;  //  for SET
    bool setfirstvalid;  //  for SET first normal
    void Init() {
        size_t len = sizeof(titem) / sizeof(TIMEITEM);
        for (size_t i = 0; i < len; i++) {
            titem[i].Init();
        }
        setvalid = 0;
        setquota = 0;
        setrefuse = 0;
        setnowsum = 0;
        workernum = 0;
        setfirstvalid = false;
        return;
    }

    void CleanReportId(const string& ip) {
        map<string, map<string, IDINFO> >::iterator it = mapalloq.find(ip);
        if (mapalloq.end() != it) {
            map<string, IDINFO>& tmap = it->second;
            if (tmap.size() > 500) {
                tmap.clear();
            }

            if (tmap.size() > 300) {
                time_t now = time(NULL);
                map<string, IDINFO>::iterator it1 = tmap.begin();
                for (; it1 != tmap.end();) {
                    time_t ttime = it1->second.updatetime;
                    if ((ttime < now) && ((now - ttime) > (60 * 30))) {
                        tmap.erase(it1++);
                    } else {
                        ++it1;
                    }
                }
            }
        }
    }


    int GetWorkerNum() {
        int num = 0;
        time_t now = time(NULL);
        map<string, WorkerInfo>::iterator it = mapworker.begin();
        for (; it != mapworker.end(); it++) {
            WorkerInfo& info = it->second;
            if (((now >= info.updatetime) && ((now - info.updatetime) <= (60 * 30)))
                || (now < info.updatetime)) {
                num += info.workernum;
            }
        }
        return num;
    }

    void AddWorkerItem(const string& ip, int workernum, const time_t& ttime) {
        map<string, WorkerInfo>::iterator it = mapworker.find(ip);
        if (mapworker.end() == it) {
            WorkerInfo workerinfo;
            workerinfo.Init();
            mapworker.insert(make_pair(ip, workerinfo));
        }
        if (workernum > mapworker[ip].workernum)
            mapworker[ip].workernum = workernum;

        mapworker[ip].updatetime = ttime;
        return;
    }

    int32_t GetProcQ(int32_t procvalid) {
        int32_t procquota = 0;
        if ((setvalid > 0 && procvalid > 0) && setfirstvalid) {
            procquota = setquota * procvalid / setvalid;
        }
        return procquota;
    }

    int32_t GetSetRealNum() {
        return (setvalid > setrefuse) ? (setvalid - setrefuse) : 0;
    }

    int32_t GetAverQ() {
        return (workernum > 0) ? (setquota / workernum) : setquota;
    }

    std::stringstream& ShowAll(std::stringstream& ss) {
        ss << endl << "///////////////////////////////" << endl;
        for (int i = 0; i < NUM_PER_MIN; i++) {
            titem[i].GetInfo(ss);
        }

        ss << std::endl;
        ss << "iplist:";
        map<string, WorkerInfo>::iterator it2 = mapworker.begin();
        for (; it2 != mapworker.end(); it2++) {
            ss << it2->second.workernum << " ";
        }

        ss << std::endl << "updatetime:" << updatetime;
        ss << endl << "///////////////////////////////" << endl;
        return ss;
    }
} QDATA;

////////////////////////////////////////////////////////////////////////////

typedef std::map<std::string, QDATA > QDATAMAP;  //  key : quota

typedef struct {  //  client
    time_t timekey;
    int32_t sum;
    int32_t refuse;
    int32_t setrefuse;
    void Init() {
        timekey = 0;
        sum = 0;
        refuse = 0;
        setrefuse = 0;
        return;
    }
} TIMESUM;

#define MIN_HARD_CODE (100*10000)
#define QUOTA_ARRAY_NUM (12)
typedef struct {  //  client
    TIMESUM timesum[NUM_PER_MIN];
    int64_t totalquota;  //  total quota

    //
    int64_t localquota;  //
    int64_t localquotamax;
    int64_t localquotamin;
    int64_t localquotavary;

    //
    int64_t localquotaver;
    int64_t localquotamaxaver;
    int64_t localquotaminaver;
    int64_t localquotavervary;

    //
    int64_t nowsum;  //
    int32_t maxper;  //   per min maxvalue
    int32_t minper;  //  per min minvalue
    int32_t flowvary;

    //
    //  int32_t flowavervary;

    int32_t leftminq;  //  left quota
    int32_t overmaxq;  //  overq

    int32_t reqnum;  //  per min
    int32_t totalreqtime;  //  ms total min
    int32_t maxtime;  //  ms per min
    time_t lastreport;

    int64_t restrainquota;  // 20% float
    time_t updatetime;   // last update time
    time_t now;

    uint32_t index;
    int64_t qarray[QUOTA_ARRAY_NUM];
    uint32_t lock;
    uint32_t totalworkernum;

    //  set logic
    int32_t setquota;
    int32_t setsum;
    int32_t procquota;
    int32_t averq;

    void Init() {
        memset(this, 0, sizeof(*this));
        minper = MIN_HARD_CODE;
        leftminq = MIN_HARD_CODE;
        localquotamin = MIN_HARD_CODE;
        localquotaminaver = MIN_HARD_CODE;
        lock = 0;
        totalworkernum = 0;
        setquota = 0;
        setsum = 0;
        procquota = 0;
        averq = 0;
        return;
    }

    int64_t GetAverQ() {
        int64_t ret = 0;
        uint32_t num = 0;
        int64_t quota = 0;
        for (int i = 0; i < QUOTA_ARRAY_NUM; i++) {
            if (qarray[i] > 0) {
                num++;
                quota += qarray[i];
            }
        }

        if (num > 0)
            ret = quota / num;

        return ret;
    }

    void ClearQuota() {
        memset(qarray, 0, sizeof(qarray));

        return;
    }

    bool QSetValid() {
        SF_LOG(LOG_DEBUG, "QSetValid setquota:%d, setsum:%d\n", setquota, setsum);
        if (setquota > 0 && setsum > 0) {
            if (0 == procquota) {
                if (totalworkernum > 0)
                    averq = setquota / totalworkernum;
            }
            if (procquota > 0 || averq > 0)
                return true;
        }
        return false;
    }

    bool QSetOverLoad(int32_t nowsum) {  //  0: suc1 :mid 2:over
        if (QSetValid()) {
            int32_t valid = nowsum;
            int32_t tmp = (procquota > 0) ? procquota : averq;
            int percent = valid * 100 / tmp;
            SF_LOG(LOG_DEBUG, "QSetValid percent:%d, nowsum:%d, procquota:%d\n",
            percent, valid, tmp);
            if (percent >= (100 + QUOTA_FLOAT_PERCENT)) {
                return true;
            }

            if (percent >= 100) {
                int delta = percent - 100;
                return HitProba(delta);
            }
            return false;
        }

        return false;
    }
} LOCALDATA;

typedef map<string, LOCALDATA> LOCALMAP;

typedef struct {
    string remoteip;
    time_t uptime;
    void Init(time_t ttime, string& ip) {
        remoteip = ip;
        uptime = ttime;

        return;
    }
} QEXTINFO;

class QLocalPara {
 public:
    int64_t keyid;
    int32_t groupid;
    int32_t totalquota;
    string serial;
    string referkey;

    QLocalPara() {
        keyid = 0;
        groupid = 0;
        totalquota = 0;
    }

    ~QLocalPara() {
    }
};

//  add set struct
typedef struct {
    string key;
    int32_t setquota;
    int64_t setsum;   //  percent seconds
    uint32_t feaid;
    uint32_t refuseid;
    uint32_t setrefuseid;
    uint32_t realid;
    TIMEITEM timesum[NUM_PER_MIN];
    uint32_t lock;
    time_t updatetime;
    uint32_t workernum;

    void Init() {
        setquota = 0;
        setsum = 0;
        feaid = 0;
        refuseid = 0;
        setrefuseid = 0;
        realid = 0;
        for (int i = 0; i < NUM_PER_MIN; i++) {
            timesum[i].Init();
        }
        lock = 0;
        updatetime = 0;
        workernum = 0;

        return;
    }
} SETCfgFirst;

//  SET FILENAME
TMPLT_DEFINE_ITEM(OneSetInfo, open_app_desc::SetMacInfo)
TMPLT_DEFINE_ARR(SET_ARR, OneSetInfo)
typedef std::map<std::string, SET_ARR> SET_MAP;

//  SET FILE META
TMPLT_DEFINE_ITEM(OneSetFileInfo, open_app_desc::SetFileInfo)
TMPLT_DEFINE_ARR(SETFILE_ARR, OneSetFileInfo)
typedef std::map<std::string, SETFILE_ARR> SETFILE_MAP;



/////////////////////////////////////////////////////

class ServiceQua {
 public:
    //  for report
#define FILE_LOAD_NUM 5
    typedef struct {
        int64_t mtnum;
        int64_t mttotal;
        int64_t mtmax;

        int64_t looplast;  //  loop last time
        int64_t loopdelta;  //  loop delta time
        int64_t loopdeltamax;  //   loop delta max time

        int64_t totaltime;  //  total req time
        int64_t reqtimemax;  //  one req max time

        int64_t loopfunbegin;  //  loop func begin
        int64_t loopfunend;  //  loop func end
        int64_t loopfunmax;  //  loop func time

        int64_t l5delta;
        int64_t l5deltamax;

        int64_t loopl5delta;
        int64_t loopl5deltamax;

        int64_t filedelta;
        int64_t filedeltamax;

        int64_t fileloadmax[FILE_LOAD_NUM];

        int64_t ident;

        void Init() {
            mtnum  = 0;
            totaltime = 0;
            mttotal = 0;
            mtmax = 0;

            looplast = 0;
            loopdeltamax = 0;
            loopdelta = 0;

            reqtimemax = 0;
            loopfunmax = 0;
            l5delta = 0;
            l5deltamax = 0;
            loopl5delta = 0;
            loopl5deltamax = 0;
            filedelta = 0;
            filedeltamax = 0;
            memset(fileloadmax, 0, sizeof(fileloadmax));

            ident = GetMinute();

            return;
        }

        void CleanArr() {
            memset(fileloadmax, 0, sizeof(fileloadmax));
            return;
        }
    } WindItem;
    static WindItem s_winditem;
    static void LoopEnter(/*WindItem& wi*/);
    static void LoopLeave(/*WindItem& wi*/);
    static void FlowEnter(/*WindItem& wi, time_t ttime*/);
    static void FlowLeave(int64_t deltatime);


 private:
    static map<string, QUAWIN> s_map_qua;
    static MTItem s_wi;
    static int InitKeyQua(const string& key);

    // quota
    static QDATAMAP s_map_quota;
    static QDATAMAP s_map_proc_quota;
    static LOCALMAP s_map_local;
    static QDATA& InitKeyQuaAlloc(const string& key, QDATAMAP& qmap);

 public:
    // add set logic
    // set cfg
    static std::map<std::string, std::string> mapSETIPCfg;
    static std::map<std::string, SETCfgFirst> mapSETCfgFirst;

    static QDATAMAP s_map_setsumdata;
    static QDATAMAP s_map_setprocdata;

    //
    static std::string s_localhost;
    static uint32_t s_now;
    static uint32_t s_setsum;
    static std::set<PROCQ> s_setproc;

    // Real Report
    static std::map<string, int> s_map_fd;

 public:
    template<typename T>
    static bool CheckQos(const T& keyname, const open_app_desc::Flux *flux = NULL) {
        stringstream ss;
        ss << keyname;

        //  FTRACE
        //  SF_LOG(LOG_DEBUG, "%s\n", __func__);
        bool ret = true;
        static bool s_qos_status = true;  // down line true;

        const string& key = ss.str();
        InitKeyQua(key);
        QUAWIN& quawin = s_map_qua[key];
        time_t ttime = __spp_get_now_s();
        uint32_t suc = 0;
        uint32_t fail = 0;

        uint32_t winlen = GetWinLen(flux);
        int index = ttime % s_pasttime;
        for (uint32_t i = 0; i < win_num; i++) {
            time_t pasttime = quawin.item[index].ttime;

            if ((ttime > pasttime) && ((ttime - pasttime) <= s_pasttime)) {
                suc += quawin.item[index].suc;
                fail += quawin.item[index].fail;
            }

            --index;
            index = (index + win_num) % win_num;
            if ((i + 1) >= winlen)
                break;
        }

        uint32_t err_num_threshold = GetErrNum(flux);
        uint32_t err_rate = GetErrNumRate(flux);
        SF_LOG(LOG_DEBUG, "CheckQos:threshold:%d, s_ratio:%d, time:%d, id:%s, suc: %d, fail:%d \n",
               err_num_threshold, s_ratio, ttime, key.c_str(), suc, fail);
        if (fail > err_num_threshold) {
            if (fail * (s_ratio - err_rate) > err_rate * suc) {
                //  send warning
                uint32_t proba = GenFlowProba(err_rate);
                if (HitProba(proba)) {
                    ret = true;
                } else {
                    ret = false;
                }
            }
        }

        if (ret != s_qos_status) {
            s_qos_status = ret;
        }

        return ret;
    }

    template<typename T>
    static int UpdateQos(const T& keyname, int32_t ret/* 0:suc other:fail  -3:timeout .......*/,
                        uint64_t ms = 0 /*ms*/) {
        stringstream ss;
        ss << keyname;

        //  FTRACE
        //  SF_LOG(LOG_DEBUG, "%s\n", __func__);
        const string& key = ss.str();
        InitKeyQua(key);
        QUAWIN& quawin = s_map_qua[key];
        time_t ttime = __spp_get_now_s();
        int index = ttime % s_pasttime;

        if ((0 == quawin.item[index].ttime)
                || ((ttime > quawin.item[index].ttime) &&
                ((ttime - quawin.item[index].ttime) >= s_pasttime))) {
            SF_LOG(LOG_DEBUG, "retry time:%d, index:%d, id:%s, suc: %llu, fail:%llu \n",
                   quawin.item[index].ttime, index, key.c_str(),
                   quawin.item[index].suc, quawin.item[index].fail);
            quawin.item[index].ttime = ttime;
            quawin.item[index].suc = 0;
            quawin.item[index].fail = 0;
        }

        if (0 == ret)
            quawin.item[index].suc += 1;
        else
            quawin.item[index].fail += 1;

        quawin.updatetime = ttime;
        SF_LOG(LOG_DEBUG, "UpdateQos:%d, index:%d, id:%s, suc: %llu, fail:%llu \n",
               ttime, index, key.c_str(), quawin.item[index].suc, quawin.item[index].fail);

        return 0;
    }

    static int ClearKey();
    static int ClearData();
    ///////////////////////////////////////////////
    static int64_t GetFeaID(int64_t base);

    //  mt num control
    static uint32_t MTEnter(uint32_t num = 1);
    static uint32_t MTLeave(uint32_t num = 1);
    static uint32_t GetMTNum();
    ///////////////////////////////////////////////////
    static uint32_t GenFlowProba(uint32_t erate);
    //  static bool HitProba(uint32_t target);
    static bool InitSysload();
    static bool SysOverload(const open_app_desc::Flux *flux = NULL);
    static uint32_t ReportAtt(uint32_t cpu, uint32_t mem);
    static std::string ShowData();

    template <typename T>
    static int32_t ReportQuery(const T& act) {
        if (act.has_flux()) {
            const open_app_desc::Flux& flux = act.flux();
            if (flux.queryid() > 0) {
            } else {
            }

            if (flux.totalflowid() > 0) {
            } else {
            }
        } else {
        }

        return 0;
    }


    template <typename T>
    static int32_t ReportAttend(const T& act) {
        if (act.has_flux()) {
            const open_app_desc::Flux& flux = act.flux();
            if (flux.attendid() > 0) {
            } else {
            }

            if (flux.totalflowid() > 0) {
            } else {
            }
        } else {
        }
        return 0;
    }

    template <typename T>
    static bool ManagerQueryLimit(const T& act) {
        bool ret = false;
        if (act.has_flux()) {
            const open_app_desc::Flux& flux = act.flux();
            if (flux.queryrate()>0) {
                uint32_t instance_rate = flux.queryrate();
                if ((instance_rate > 0) && (instance_rate < s_ratio)) {
                    if ( !HitProba(instance_rate)) {
                        ret = true;
                    } else {
                        ret = false;
                    }
                }
            }
        }

        return ret;
    }

    template <typename T>
    static bool FluxSwitch(const T& act, uint32_t& querytype, uint32_t& attendtype) {
        bool ret = false;
        querytype = 0;
        attendtype = 0;
        if (act.has_flux()) {
            const open_app_desc::Flux& flux = act.flux();
            if (flux.flowswitch() == 0) {
                ret = false;
            } else {
                ret = true;
            }
            
            uint32_t type = 1;
            if (2 == flux.type()) {
                type = 2;
            }

            if (flux.queryquota() > 0) {
                querytype = type;
            }

            if (flux.attendquota() > 0) {
                attendtype = type;
            }
        }



        return ret;
    }
    static uint32_t GetWinLen(const open_app_desc::Flux *flux);
    static uint32_t GetErrNum(const open_app_desc::Flux *flux);
    static uint32_t GetErrNumRate(const open_app_desc::Flux *flux);
    static uint32_t GetCPURate(const open_app_desc::Flux *flux);
    static time_t GetCPUInterval(const open_app_desc::Flux *flux);
    static uint32_t GetMemRate(const open_app_desc::Flux *flux);
    static string GetAllPara(const open_app_desc::Flux *flux);

    //  Quota
    static int GetTimeKey(time_t ttime, time_t& timekey, int& index);
    static int InitLocalMap(LOCALMAP& local, const string& id);

    //
    static int GetQuotaL5(int env, const open_app_desc::Flux *flux, cl5::QOSREQUEST& aquol5);

    static bool ProcessByRefer(LOCALDATA& local, const string& referid,
                                int32_t& ret, int& deltaquota);
    static bool ProcessByAver(LOCALDATA& local, int64_t totalquota,
                                int32_t& ret, int& deltaquota);


    static int32_t FluxQuota(string& keyid, int32_t groupid, int32_t totalquota,
                            cl5::QOSREQUEST& aquol5);
    //  ret, 0: < 100%, 1: >100%, <120% 2: > 120%
    static int32_t CheckQuota(string& keyid, int32_t groupid, int32_t totalquota,
                              cl5::QOSREQUEST& aquol5, string referid = "",
                              const string& serial = "");
    static int32_t RealReport(int32_t keytype, const string& keyname, int64_t timekey,
                                const string& setname,
                              TIMEITEM& titem, int32_t quota = 0);
    static int32_t QuotaReport(const string& actkey, const string& ip, int32_t id,
                                const string& setkey, int32_t aquota,
                               int32_t squota, PRODATA& report, int64_t timekey);
    static int32_t GetLastQuota(QDATA& procdata, const string& ip, const string& procid);
    static uint32_t GetQueryQuota(const QEXTINFO& info, const open_app_desc::QuotaReq& req,
                                  open_app_desc::QuotaRsp& rsp,
                                  const open_app_desc::Flux *flux = NULL);

    static uint32_t ProSETQuota(const open_app_desc::SetQuotaReq& req,
                        const string& firstip, open_app_desc::SetQuotaRsp&);

    template <typename T>
    static uint32_t GetSETCfgFirst(const T& req, const std::string& remoteip,
                            string& setkey, SETCfgFirst *&psetcfgfirst) {
        if (req.setkey().size() > 0) {
            setkey = req.setkey();
        } else {
            if (remoteip.size() > 0) {
                map<string, string>::iterator it = mapSETIPCfg.find(remoteip);
                if (it != mapSETIPCfg.end()) {
                    setkey = it->second;
                }
            }
        }

        if (setkey.size() > 0) {
            map<string, SETCfgFirst>::iterator it = mapSETCfgFirst.find(setkey);
            if (it != mapSETCfgFirst.end()) {
                psetcfgfirst = &(it->second);
                //  SF_LOG(LOG_DEBUG, "GetSETCfgFirst: find set config");
            }
        }
        return 0;
    }

    static uint32_t GetSetKey(const std::string& remoteip, string& setkey) {
        if (remoteip.size() > 0) {
            map<string, string>::iterator it = mapSETIPCfg.find(remoteip);
            if (it != mapSETIPCfg.end()) {
                setkey = it->second;
            }
        }

        return 0;
    }

#define DECLARE_QUOTA_ERASE_FUNC(funcname, maptype)\
    static int funcname(maptype &themap);

#define DEFINE_QUOTA_ERASE_FUNC(funcname, maptype, endname)\
    int ServiceQua::funcname(maptype &themap) {\
        time_t ttime = __spp_get_now_s();\
        maptype::iterator it = themap.begin();\
        for (; it != themap.end();) {\
            if ((ttime - it->second.updatetime) > s_expire) \
                themap.erase(it++);\
            else\
                ++it;\
        }\
        return 0;\
    }

    DECLARE_QUOTA_ERASE_FUNC(ClearQData, QDATAMAP)
    DECLARE_QUOTA_ERASE_FUNC(ClearLocalData, LOCALMAP)

    //  for set config
    static int LoadSETInfoWrap();
};

#endif  //  FLUX_QUASERVER_SERVICEQUA_H_
