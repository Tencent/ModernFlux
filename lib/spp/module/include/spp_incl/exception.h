/**
 *  * spp exception signal handle
 *   **/
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__
#include "serverbase.h"
#include <string>

using std::string;

namespace spp
{
namespace exception
{

#define EXCEP_NOPATITION      (-1)
#define EXCEP_NOENOUGHSPACE   (-2)

typedef const std::string (*GET_MSG_FUN)();
typedef bool (*CHECK_STACK_FUN)(long);

typedef enum tagPackType
{
    PACK_TYPE_FRONT_RECV = 0, //front-end recive 
    PACK_TYPE_BACK_RECV,      //back-end recive
    PACK_TYPE_BUTT
}PackType;

struct PacketData
{
    int64_t sustime_;
    int     srcip_;
    int     dstip_;
    int     srcport_;
    int     dstport_;
    int     packlen_;
    void   *packetbuff_;
    PacketData()
    {
        sustime_ = 0;
        srcip_   = 0;
        dstip_   = 0;
        srcport_ = 0;
        dstport_ = 0;
        packlen_ = 0;
        packetbuff_ = NULL;
    }
    PacketData(void *packet, int packlen, int srcip, int dstip, int srcport, int dstport, int64_t sustime)
    {
        srcip_   = srcip;
        dstip_   = dstip;
        srcport_ = srcport;
        dstport_ = dstport;
        sustime_ = sustime;
        packlen_ = packlen;
        packetbuff_  = packet;
    }
};

extern bool _spp_g_exceptionpacketdump;
extern bool _spp_g_exceptioncoredump;
extern bool _spp_g_exceptionrestart;
extern bool _spp_g_exceptionmonitor;
extern bool _spp_g_exceptionrealtime;

extern GET_MSG_FUN _spp_g_get_msg_fun_ptr;	//获取微线程当前消息的函数指针，在work  里面会给他赋值
extern CHECK_STACK_FUN _spp_g_check_stack_fun_ptr;	//检查微线程当前栈是否安全，在work  里面会给他赋值

extern void SavePackage(PackType enPacktype, const PacketData *pstPacketData);
extern char *GetFileName(int iFileType);
extern void InitHanleException(void);
extern void set_serverbase(CServerBase *base);
extern void deal_workexception(long lESP, int signo);
extern void GstackLog(int pid);

}
}

#endif


