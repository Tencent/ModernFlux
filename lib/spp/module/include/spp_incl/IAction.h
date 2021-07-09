/*
 * =====================================================================================
 *
 *       Filename:  IAction.h
 *
 *    Description:  
 *
 *
 *        Version:  1.0
 *        Created:  06/22/2010 09:38:18 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __IACTION_H__
#define __IACTION_H__

#include "CommDef.h"

BEGIN_ASYNCFRAME_NS

class CAsyncFrame;
class CMsgBase;

class IAction
{
    public:
        virtual ~IAction() {}

        /**
         * 动作的请求包编码方法，由框架在启动动作之前调用；
         * 插件在实现这个方法时，如果发现buffer的长度不够，返回实际需要的buffer大小，
         * 框架会根据返回值来扩展实际buffer大小，并再次调用HandleEncode方法；
         * buffer扩展只进行一次；
         *
         * @param pFrame CAsyncFrame对象指针，提供日志记录功能
         * @param buf 存放请求包的内存块
         * @param len [in]内存块大小，[out]请求包实际大小
         * @param pMsgBase CMsgBase对象指针，通常是插件实现的CMsgBase派生类对象，它用来存放用户数据。该对象代表一个请求
         *
         * @return 0: 成功；>0：内存块大小不够，返回实际需要的内存大小；<0: 失败
         *
         */
        virtual int HandleEncode( CAsyncFrame *pFrame, 
                                        char *buf, 
                                        int &len, 
                                        CMsgBase *pMsgBase) = 0;

        /**
         * 动作的响应包完整性检查，由框架在收到数据时调用
         *
         * @param pFrame CAsyncFrame对象指针，提供日志记录功能
         * @param buf 接收到的数据缓存地址
         * @param len 接收到的数据大小
         * @param pMsgBase CMsgBase对象指针，通常是插件实现的CMsgBase派生类对象，它用来存放用户数据。该对象代表一个请求
         *
         * @return >0: 返回完整响应包大小；=0：响应包还不完整； <0: 出错
         */
        virtual int HandleInput( CAsyncFrame *pFrame,
                                        const char *buf,
                                        int len,
                                        CMsgBase *pMsgBase) = 0;

        /**
         * 动作的响应包处理，当接收到一个完整响应包时由框架调用;
         * 插件可以在此方法中对动作的响应包进行处理，也可以在所有动作都处理完毕时，在IState::HandleProcess方法里处理
         *
         * @param pFrame CAsyncFrame对象指针，提供日志记录功能
         * @param buf 完整包数据缓存地址
         * @param len 完整包数据大小
         * @param pMsgBase CMsgBase对象指针，通常是插件实现的CMsgBase派生类对象，它用来存放用户数据。该对象代表一个请求
         *
         * @return 0: 成功；其他：失败
         */
        virtual int HandleProcess( CAsyncFrame *pFrame, 
                                        const char *buf,
                                        int len,
                                        CMsgBase *pMsgBase) 
        {
            return 0;
        }

        /**
         * 动作出错处理，当动作处理出错时，由框架调用;
         *
         * @param pFrame CAsyncFrame对象指针，提供日志记录功能
         * @param err_no 错误码, 具体定义如下：
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
         * @param pMsgBase CMsgBase对象指针，通常是插件实现的CMsgBase派生类对象，它用来存放用户数据。该对象代表一个请求
         *
         * @return 0: 成功；其他：失败
         *
         */
        virtual int HandleError( CAsyncFrame *pFrame,
                                        int err_no,
                                        CMsgBase *pMsgBase)
        {
            return 0;
        }

};

END_ASYNCFRAME_NS

#endif
