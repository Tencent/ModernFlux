/*
 * =====================================================================================
 *
 *       Filename:  IState.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/22/2010 03:23:57 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __ISTATE_H__
#define __ISTATE_H__
#include "ActionSet.h"


BEGIN_ASYNCFRAME_NS

class CAsyncFrame;
class CMsgBase;

class IState
{       
    public: 
        virtual ~IState() {}

        /**
         * 异步框架在启动状态之前调用，由插件实现；
         * 在这个方法里，插件告诉框架应该启动哪些动作，当然，也可以不启动任何动作；
         * 如果需要执行Action，则框架启动Action进行异步处理,当所有Action都执行结束时，再回调IState::HandleProcess方法；
         * 如果不需要执行Action，则框架马上调用IState::HandleProcess进行同步处理。
         * 
         * @param pFrame CAsyncFrame对象指针，提供日志记录功能
         * @param pActionSet CActionSet对象指针，提供接口添加需要执行的动作
         * @param pMsgBase CMsgBase对象指针，通常是插件实现的CMsgBase派生类对象，它用来存放用户数据。该对象代表一个请求
         *
         * @return 0: 成功； 其他：失败，失败则会调用注册到框架的CBType_Fini类型回调函数
         *
         */
        virtual int HandleEncode(CAsyncFrame *pFrame, 
                                    CActionSet *pActionSet, 
                                    CMsgBase *pMsgBase) = 0;

        /**
         * 异步框架在状态处理完毕之后调用，由插件实现；
         * 在这个方法里，插件可以处理动作的执行结果;
         *
         * @param pFrame CAsyncFrame对象指针，提供日志记录功能
         * @param pActionSet CActionSet对象指针，提供接口访问动作的执行结果
         * @param pMsgBase CMsgBase对象指针，通常是插件实现的CMsgBase派生类对象，它用来存放用户数据。该对象代表一个请求
         *
         * @return >0： 下一个状态ID，框架会切换到该状态；<=0: 当前请求处理结束，接着框架会调用注册到框架的CBType_Fini类型回调函数
         *
         */
        virtual int HandleProcess(CAsyncFrame *pFrame, 
                                    CActionSet *pActionSet, 
                                    CMsgBase *pMsgBase) = 0;
};

END_ASYNCFRAME_NS

#endif
