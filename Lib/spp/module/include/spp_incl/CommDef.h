/*
 * =====================================================================================
 *
 *       Filename:  commdef.h
 *
 *    Description:  
 *
 *
 *        Version:  1.0
 *        Created:  06/22/2010 09:56:33 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __COMMDEF_H__
#define __COMMDEF_H__

#define BEGIN_ASYNCFRAME_NS namespace SPP_ASYNCFRAME {
#define END_ASYNCFRAME_NS }
#define USING_ASYNCFRAME_NS using namespace SPP_ASYNCFRAME

BEGIN_ASYNCFRAME_NS

#define IP_BUF_SIZE 16
#define MIN_BUF_SIZE 32

/**
 * 路由配置类型
 *
 */
typedef enum
{
    NoGetRoute,             // 无需框架获取路由
    L5NonStateRoute,        // L5无状态路由
    L5StateRoute,           // L5有状态路由
    L5AntiParalRoute,       // L5防并发路由
}RouteType;



typedef unsigned short PortType;

// 协议类型
typedef enum enumProtoType
{
    ProtoType_Invalid = -1,
    ProtoType_TCP,              // tcp协议
    ProtoType_UDP,              // udp协议
}ProtoType;

// 连接类型
typedef enum enumConnType
{
    ConnType_Short = 0,         // 短连接
    ConnType_Long,              // 长连接，在连接上没有等待请求
    ConnType_LongWithPending,   // 长连接，在连接上可以有等待请求
}ConnType;

// Action类型
typedef enum enumActionType
{
    ActionType_Invalid = -1,
    ActionType_SendRecv,                            // 一发一收，短连接
    ActionType_SendRecv_KeepAlive,                  // 一发一收，长连接, 在连接上没有等待请求
    ActionType_SendRecv_KeepAliveWithPending,       // 一发一收，长连接，在连接上可以有等待请求
    ActionType_SendOnly,                            // 只发不收，短连接
    ActionType_SendOnly_KeepAlive,                  // 只发不收，长连接，在连接上没有等待请求
    ActionType_SendOnly_KeepAliveWithPending,       // 只发不收，长连接，在连接上可以有等待请求
    ActionType_SendRecv_Parallel,                   // 多发多收，长连接
}ActionType;

inline const char *ProtoTypeToStr(ProtoType type)
{
    if( type == ProtoType_TCP )
    {
        return "tcp";
    }
    else if ( type == ProtoType_UDP )
    {
        return "udp";
    }

    return "invalid";

}

inline const char *ConnTypeToStr(ConnType type)
{
    if( type == ConnType_Short )
    {
        return "short";
    }
    else if ( type == ConnType_Long )
    {
        return "long";
    }
    else if ( type == ConnType_LongWithPending )
    {
        return "long_with_pending";
    }

    return "invalid";

}


inline const char *ActionTypeToStr(ActionType type)
{
    if( type == ActionType_SendRecv )
    {
        return "SendRecv";
    }
    else if ( type == ActionType_SendRecv_KeepAlive )
    {
        return "SendRecv_KeepAlive";
    }
    else if ( type == ActionType_SendRecv_KeepAliveWithPending )
    {
        return "SendRecv_KeepAliveWithPending";
    }
    else if ( type == ActionType_SendOnly )
    {
        return "SendOnly";
    }
    else if ( type == ActionType_SendOnly_KeepAlive )
    {
        return "SendOnly_KeepAlive";
    }
    else if ( type == ActionType_SendOnly_KeepAliveWithPending )
    {
        return "SendOnly_KeepAliveWithPending";
    }
    else if ( type == ActionType_SendRecv_Parallel )
    {
        return "SendRecv_Parallel";
    }

    return "invalid";

}


enum ErrorNo
{
    ESuccess    = 0,        // 成功
    EConnect    = -1,       // 连接失败 
    ESend       = -2,       // send失败
    ERecv       = -3,       // recv失败
    ECheckPkg   = -4,       // 包检查失败
    ETimeout    = -5,       // Action处理超时
    ECreateSock = -6,       // 创建socket失败
    EAttachPoller = -7,     // AttachPoller失败
    EInvalidState = -8,     // 非法状态
    EHangup       = -9,     // Hangup事件发生
    EShutdown     = -10,    // 对端关闭连接
    EEncodeFail   = -11,    // 编码错误 
    EInvalidRouteType = -12,// 非法路由配置类型
    EMsgTimeout       = -13,// Msg处理超时
    EGetRouteFail     = -14,// 获取路由失败
    ERecvUncomplete   = -15,// buf太小，无法完整接收回包
    EOverLinkErrLimit = -16,// error link check limit
    EGetRouteOverload = -17,// 获取路由过载
};

inline const char* spperrToStr(int err)
{
    static const char* errstr[] =
    {
        "ESuccess",    
        "EConnect",    
        "ESend",       
        "ERecv",
        "ECheckPkg",
        "ETimeout",
        "ECreateSock",
        "EAttachPoller", 
        "EInvalidState", 
        "EHangup",       
        "EShutdown",
        "EEncodeFail",   
        "EInvalidRouteType", 
        "EMsgTimeout",       
        "EGetRouteFail",
        "ERecvUncomplete",
        "EOverLinkErrLimit",
        "EGetRouteOverload",
    };

    err = err < 0 ? -err : err;
    if (err >= (int)(sizeof(errstr)/sizeof(char*)))
    {
        return "unkown";
    }

    return errstr[err];
}

END_ASYNCFRAME_NS

#endif
