//必须包含spp的头文件
#include "sppincl.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "msg.h"
#include "GetState.h"
#include "GetState2.h"
#include "UpdateState.h"

#define MODULE_PROC_NUM	"module_proc_num"

//global backend seqno
int32_t seq = 0;

char *format_time( time_t tm);

int Init(CAsyncFrame* pFrame, CMsgBase* pMsg)
{
    //CAsyncFrame *pFrame = (CAsyncFrame *)arg1;
    //CMsg *msg = (CMsg *) arg2;
    return STATE_ID_GET;
    //return STATE_ID_GET2;   // test L5Route Action
}

typedef struct rsp_pkg
{
    int uin;
    int level;
    int coin;
} rsp_pkg_t;

int Fini(CAsyncFrame* pFrame, CMsgBase* pMsg)
{
    //CAsyncFrame *pFrame = (CAsyncFrame *)arg1;
    CMsg *msg = (CMsg *) pMsg;

    pFrame->FRAME_LOG( LOG_DEBUG,
                       "uin: %d, level: %d, coin: %d, seed_num: %d, gain_num: %d, result: %d\n",
                       msg->uin, msg->level, msg->coin, msg->seed_num, msg->gain_num, msg->result);
    std::string info;
    pMsg->GetDetailInfo(info);
    pFrame->FRAME_LOG( LOG_DEBUG, "%s\n", info.c_str());

    rsp_pkg_t pkg;
    pkg.uin = msg->uin;
    pkg.level = msg->level;
    pkg.coin = msg->coin;

    blob_type rspblob;
    rspblob.data = (char *)&pkg;
    rspblob.len = sizeof(pkg);
    pMsg->SendToClient(rspblob);

    return 0;
}

int OverloadProcess(CAsyncFrame* pFrame, CMsgBase* pMsg)
{
    //CAsyncFrame *pFrame = (CAsyncFrame *)arg1;
    //CMsg *msg = (CMsg *) pMsg;

    pFrame->FRAME_LOG( LOG_DEBUG, "Overload.\n" );

    rsp_pkg_t pkg;
    pkg.uin = -1000;
    pkg.level = -999;
    pkg.coin = -888;

    blob_type rspblob;
    rspblob.data = (char *)&pkg;
    rspblob.len = sizeof(pkg);
    pMsg->SendToClient(rspblob);

    return 0;
}
//初始化方法（可选实现）
//arg1:	配置文件
//arg2:	服务器容器对象
//返回0成功，非0失败
extern "C" int spp_handle_init(void* arg1, void* arg2)
{
    //插件自身的配置文件
    const char* etc = (const char*)arg1;
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P(LOG_DEBUG, "spp_handle_init, %s, %d\n", etc, base->servertype());
    //建立一个统计项, 统计策略为累加
//    base->stat_.init_statobj(MODULE_PROC_NUM, STAT_TYPE_SUM);

    if ( base->servertype() == SERVER_TYPE_WORKER )
    {
        /*
        CPollerUnit* pPollerUnit = SPP_ASYNC::GetPollerUnit();
        CTimerUnit* pTimerUnit = SPP_ASYNC::GetTimerUnit();

        base->log_.LOG_P(LOG_DEBUG, "init AsyncFrame, PollerUnit: %p, TimerUnit: %p\n",
                pPollerUnit, pTimerUnit);

        CAsyncFrame::Instance()->InitFrame(base, pPollerUnit, pTimerUnit, 100);
        */
        CAsyncFrame::Instance()->InitFrame2(base, 100, 0);

        CAsyncFrame::Instance()->RegCallback(CAsyncFrame::CBType_Init, Init);
        CAsyncFrame::Instance()->RegCallback(CAsyncFrame::CBType_Fini, Fini);
        CAsyncFrame::Instance()->RegCallback(CAsyncFrame::CBType_Overload, OverloadProcess);

        CAsyncFrame::Instance()->AddState(STATE_ID_GET, new CGetState);
        CAsyncFrame::Instance()->AddState(STATE_ID_UPDATE, new CUpdateState);
        CAsyncFrame::Instance()->AddState(STATE_ID_GET2, new CGetState2);

        //仅worker 可以基于命令字时延 统计和上报
        //必须先初始化相应的命令字
        //INIT_STAT_ITEM(0);

        // if use parallel, onnections number limit
        //InitConnNumLimit("10.0.0.1", 10000, 2); // No route, just ip:port
        //InitConnNumLimit(/*modid*/123, /*cmdid*/456, 2); // Route: NonState or AntiParal
        //InitConnNumLimit(/*modid*/123, /*cmdid*/-1, 2); // Route: State 
    }

    return 0;
}
//数据接收（必须实现）
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回正数表示数据已经接收完整且该值表示数据包的长度，0值表示数据包还未接收完整，负数表示出错
extern "C" int spp_handle_input(unsigned flow, void* arg1, void* arg2)
{
    //数据块对象，结构请参考tcommu.h
    blob_type* blob = (blob_type*)arg1;
    //extinfo有扩展信息
    TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P(LOG_DEBUG, "spp_handle_input, %d, %d, %s, %s\n",
                     flow,
                     blob->len,
                     inet_ntoa(*(struct in_addr*)&extinfo->remoteip_),
                     format_time(extinfo->recvtime_));

    if ( blob->len >= (int)sizeof(int) )
    {
        return (int)sizeof(int);
    }
    return 0;
}
//路由选择（可选实现）
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回值表示worker的组号

extern "C" int spp_handle_route(unsigned flow, void* arg1, void* arg2)
{
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P(LOG_DEBUG, "spp_handle_route, %d\n", flow);
    //int route_no = 2;
    //return GROUPID(route_no);
    return 1;
}

//数据处理（必须实现）
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回0表示成功，非0失败
extern "C" int spp_handle_process(unsigned flow, void* arg1, void* arg2)
{
    //数据块对象，结构请参考tcommu.h
    blob_type* blob = (blob_type*)arg1;
    TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;

    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    //数据来源的通讯组件对象
    CTCommu* commu = (CTCommu*)blob->owner;

    //int ret = 0;

    base->log_.LOG_P_PID(LOG_DEBUG, "spp_handle_process, %d, %d, %s, %s\n",
                         flow,
                         blob->len,
                         inet_ntoa(*(struct in_addr*)&extinfo->remoteip_),
                         format_time(extinfo->recvtime_));

    /*
    rsp_pkg_t pkg;
    pkg.uin = 111;
    pkg.level = 10;
    pkg.coin = 1000;

    blob_type rspblob;
    rspblob.data = (char *)&pkg;
    rspblob.len = sizeof(pkg);

    commu->sendto( flow, &rspblob, arg2 );
    */
    CMsg *msg = new CMsg;
    msg->SetServerBase(base);
    msg->SetTCommu(commu);
    msg->SetFlow(flow);
    msg->SetInfoFlag(true);
    msg->SetMsgTimeout(0);
    msg->uin = *(int*)blob->data;
    msg->level = 0;
    msg->coin = 0;
    //初始化后 可以调用宏上报
    //STEP_STAT_ITEM(0, 5, 101);
    CAsyncFrame::Instance()->Process( msg );

    return 0;
}
//析构资源（可选实现）
//arg1:	保留参数
//arg2:	服务器容器对象
extern "C" void spp_handle_fini(void* arg1, void* arg2)
{
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P(LOG_DEBUG, "spp_handle_fini\n");

    if ( base->servertype() == SERVER_TYPE_WORKER )
    {
        CAsyncFrame::Instance()->FiniFrame();
        CAsyncFrame::Destroy();
    }
}

char *format_time( time_t tm)
{
    static char str_tm[1024];
    struct tm tmm;
    memset(&tmm, 0, sizeof(tmm) );
    localtime_r((time_t *)&tm, &tmm);

    snprintf(str_tm, sizeof(str_tm), "[%04d-%02d-%02d %02d:%02d:%02d]",
             tmm.tm_year + 1900, tmm.tm_mon + 1, tmm.tm_mday,
             tmm.tm_hour, tmm.tm_min, tmm.tm_sec);

    return str_tm;
}
