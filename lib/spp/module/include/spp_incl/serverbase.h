#ifndef _SPP_COMM_SERVERBASE_H_
#define _SPP_COMM_SERVERBASE_H_
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#include "tlog.h"			//日志
#include "tstat.h"			//统计
#include "tprocmon.h"		//监控

#if !__GLIBC_PREREQ(2, 3)
#define __builtin_expect(x, expected_value) (x)
#endif
#ifndef likely
#define likely(x)  __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)  __builtin_expect(!!(x), 0)
#endif

#define RUN_FLAG_QUIT		0x01
#define RUN_FLAG_RELOAD		0x02

#define SERVER_TYPE_UNKNOWN 0x00
#define SERVER_TYPE_PROXY   0x01
#define SERVER_TYPE_WORKER  0x02
#define SERVER_TYPE_CTRL	0x04

using namespace tbase::tlog;
using namespace tbase::tstat;
using namespace tbase::tprocmon;

namespace spp
{
namespace comm
{
//服务器程序基础类，包含运行环境初始化、日志、统计、监控对象
class CServerBase
{
public:
    CServerBase();
    virtual ~CServerBase();
    virtual void run(int argc, char* argv[]);
    virtual void startup(bool bg_run = true);
    //实际的运行函数
    virtual void realrun(int argc, char* argv[]) {}
    //返回服务容器的类型
    virtual int servertype()
    {
        return SERVER_TYPE_UNKNOWN;
    }

    //业务日志
    CTLog log_;
    //统计
    CTStat stat_;
    //监控
    CTProcMonCli moncli_;
    //内存分配器。。。等等其它设施

protected:
    struct TInternal
    {
        //main参数值
        int argc_;
        char** argv_;
        //监控信息上报间隔时间
        int moni_inter_time_;
        //框架统计信息打印间隔时间
        int fstat_inter_time_;
        //业务统计信息打印间隔时间,unused
        int stat_inter_time_;
    };
    struct TInternal* ix_;
public:
    ///////////////////////////////////////////////////////////////////////
    static bool reload();
    static bool quit();
    static void sigusr1_handle(int signo);
    static void sigusr2_handle(int signo);
    static unsigned char flag_;
    static char version_desc[64]; // unused
};
}
}
#endif
