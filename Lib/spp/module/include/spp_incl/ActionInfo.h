/*
 * =====================================================================================
 *
 *       Filename:  ActionInfo.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/13/2010 03:40:09 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __ACTION_INFO_H__
#define __ACTION_INFO_H__

#include <string.h>
#include "CommDef.h"
#include <string>
#include <cstring>

BEGIN_ASYNCFRAME_NS

class IAction;
class CNetHandler;
class CParallelNetHandler;
class CAsyncFrame;
class CActionSet;
class CMsgBase;

class CActionInfo
{
    public:
        friend class CNetHandler;
        friend class CParallelNetHandler;
        friend class CActionSet;

    public:
        /**
         * 构造函数
         *
         * @param init_buf_size 内部buffer初始大小，该buffer用于存放请求包和响应包
         */
        CActionInfo(int init_buf_size);
        virtual ~CActionInfo();

        /**
         * 设置IAction派生类对象指针，由插件分配CActionInfo对象后调用此方法进行设置
         *
         * @param pAction IAction派生类对象指针
         */
        inline int SetIActionPtr( IAction *pAction )
        {
            if( NULL == pAction ) {
                return -1;
            }

            _pAction = pAction;

            return 0;
        }

        /**
         * 设置目标IP，由插件分配CActionInfo对象后调用此方法进行设置
         *
         * @param ip 目标IP
         *
         */
        inline int SetDestIp(const char *ip)
        {
            if( NULL == ip ) {
                return -1;
            }

            strncpy( _ip, ip, sizeof(_ip)-1 );

            return 0;
        }

        /**
         * 获取目标IP
         *
         * @param ip 目标IP
         *
         */
        inline void GetDestIp(std::string &ip)
        {
            ip = _ip;
        }

        /**
         * 设置目标Port，由插件分配CActionInfo对象后调用此方法进行设置
         *
         * @param port 目标port
         *
         */
        inline void SetDestPort( PortType port )
        {
            _port = port;
        }

        /**
         * 获取目标Port
         *
         * @param port 目标port
         *
         */
        inline void GetDestPort( PortType &port )
        {
            port = _port;
        }

        /**
         * 设置协议类型：tcp 或 udp，由插件分配CActionInfo对象后调用此方法进行设置
         *
         * @param proto 协议类型：ProtoType_TCP or ProtoType_UDP
         */
        inline void SetProto( ProtoType proto )
        {
            _proto = proto;
        }

        /**
         * 获取协议类型
         *
         * @param proto 协议类型
         */
        inline void GetProto( ProtoType &proto )
        {
            proto = _proto;
        }

        /**
         * 设置动作类型，由插件分配CActionInfo对象后调用此方法进行设置
         *
         * @param type 动作类型, 具体类型如下：
         *     ActionType_SendRecv,                            // 一发一收，短连接
         *     ActionType_SendRecv_KeepAlive,                  // 一发一收，长连接, 在连接上没有等待请求
         *     ActionType_SendRecv_KeepAliveWithPending,       // 一发一收，长连接，在连接上可以有等待请求
         *     ActionType_SendOnly,                            // 只发不收，短连接
         *     ActionType_SendOnly_KeepAlive,                  // 只发不收，长连接，在连接上没有等待请求
         *     ActionType_SendOnly_KeepAliveWithPending,       // 只发不收，长连接，在连接上可以有等待请求
         *     ActionType_SendRecv_Parallel,                   // 多发多收，长连接
         *
         **/
        inline void SetActionType( ActionType type)
        {
            _type = type;
        }

        /**
         * 获取动作类型
         *
         * @param type 动作类型
         *
         */
        inline void GetActionType( ActionType &type)
        {
            type = _type;
        }

        /**
         * 设置动作ID，可以用来唯一识别动作，由插件分配CActionInfo对象后调用此方法进行设置
         *
         * @param id 动作ID
         *
         */
        inline void SetID( int id )
        {
            _id = id;
        }

        /**
         * 获取动作ID
         *
         * @param id 动作ID
         *
         */
        inline void GetID( int &id )
        {
            id = _id ;
        }

        /**
         * 设置动作超时间隔，单位：ms，由插件分配CActionInfo对象后调用此方法进行设置
         *
         * @param timeout 超时间隔
         *
         */ 
        inline void SetTimeout( int timeout )
        {
            _timeout = timeout;
        }

        /**
         * 获取动作超时间隔
         *
         * @param timeout 超时间隔
         *
         */
        inline void GetTimeout( int &timeout )
        {
            timeout = _timeout;
        }

        /**
         * 设置错误码，由框架根据动作处理情况设置
         *
         * @param err_no 错误码
         */
        inline void SetErrno( int err_no )
        {
            _errno = err_no;
        }

        /**
         * 获取错误码，插件在处理动作执行结果时可以调用此方法获取错误码，0为处理成功。
         * 具体错误码含义如下： 
         *         enum ErrorNo  {                               
         *             EConnect    = -1,      // 连接失败
         *             ESend       = -2,      // send失败 
         *             ERecv       = -3,      // recv失败
         *             ECheckPkg   = -4,      // 包检查失败
         *             ETimeout    = -5,      // 超时
         *             ECreateSock = -6,      // 创建socket失败
         *             EAttachPoller = -7,    // AttachPoller失败
         *             EInvalidState = -8,    // 非法状态
         *             EHangup       = -9,    // Hangup事件发生
         *             EShutdown     = -10,   // 对端关闭连接
         *             EEncodeFail   = -11,   // 编码错误 
         *             EInvalidRouteType = -12,// 非法路由配置类型
         *             EMsgTimeout       = -13,// Msg处理超时
         *        };
         *
         * @param err_no 错误码
         */
        inline void GetErrno( int &err_no )
        {
            err_no = _errno;
        }

        /**
         * 设置动作处理时间消耗，单位：ms，由框架在动作处理完毕之后调用
         *
         * @param cost
         */
        inline void SetTimeCost( int cost )
        {
            _time_cost = cost;
        }

        /**
         * 获取动作处理时间消耗，单位：ms
         *
         * @param cost
         */
        inline void GetTimeCost( int &cost )
        {
            cost = _time_cost;
        }

        /**
         * 获取buffer地址和实际数据长度，通常是在动作处理完之后，获取响应包数据时调用;
         * 因为发送和接收使用同一块内存，因此，此方法也可以获取打包后的数据
         *
         * @param buf 数据buffer地址
         * @param size 实际数据长度
         *
         */
        void GetBuffer( char **buf, int &size );


        /**
         * 获取路由配置类型
         *
         */
        inline RouteType GetRouteType()
        {
            return _route_type;
        }


        /**
         * 以下两个方法，插件禁止调用
         */
        int HandleStart();
        int HandleError( int err_no );
    protected:
        /**
         * 设置CAsyncFrame对象指针，由框架完成，插件无需为CActionInfo对象设置此指针
         *
         * @param pFrame CAsyncFrame对象指针
         */
        inline void SetFramePtr( CAsyncFrame *pFrame)
        {
            _pFrame = pFrame;
        }

        /**
         * 设置CActionSet对象指针，由框架完成，插件无需为CActionInfo对象设置此指针
         *
         * @param pActionSet CActionSet对象指针
         */
        inline void SetActionSetPtr( CActionSet *pActionSet )
        {
            _pActionSet = pActionSet;
        }

        /**
         * 设置CMsgBase对象指针，由框架完成，插件无需为CActionInfo对象设置此指针
         *
         * @param pMsg CMsgBase对象指针
         */
        inline void SetMsgPtr( CMsgBase *pMsg )
        {
            _pMsg = pMsg;
        }

        /**
         * 设置CNetHandler对象指针，由框架完成，插件无需为CActionInfo对象设置此指针
         *
         * @param pHandler CNetHandler对象指针
         */
        inline void SetNetHandler(CNetHandler *pHandler)
        {
            _pHandler = pHandler;
        }

        /**
         * 获取CNetHandler对象指针
         *
         */
        inline CNetHandler *GetNetHandler()
        {
            return _pHandler;
        }


    protected:
        /**
         * 清空buffer
         *
         */
        void ResetBuffer();

        /**
         * 扩展buffer大小, 当剩余空间不足时才会真正扩展
         *
         * @param need_size 需要的buffer大小
         */
        int ExtendBuffer( int need_size );

    protected:
        int HandleEncode();
        int HandleInput( const char *buf, int len);
        int HandleProcess( const char *buf, int len);

        int HandleSendFinished();

        bool SendOnly();

    protected:
        RouteType       _route_type;

        CActionSet      *_pActionSet;
        IAction         *_pAction;
        CAsyncFrame     *_pFrame;
        CMsgBase        *_pMsg;
        CNetHandler     *_pHandler;

        char            _ip[IP_BUF_SIZE];
        PortType        _port;

        ProtoType       _proto;
        ActionType      _type;

        int             _id;
        int             _timeout;       // 超时间隔,ms
        int             _errno;         // 错误码
        int             _time_cost;     // 时间开销

        char *_buf;         // buffer 
        int _buf_size;      // size of buffer 
        int _real_data_len;   // real data length 

};

END_ASYNCFRAME_NS

#endif
