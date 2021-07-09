/*
 * =====================================================================================
 *
 *       Filename:  AsyncFrame.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/15/2010 03:27:54 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __ASYNC_FRAME_H__
#define __ASYNC_FRAME_H__
#include <serverbase.h>
#include <map>
#include "timerlist.h"
#include "poller.h"

class CPollerUnit;
class CTimerUnit;

#include "PtrMgr.h"

BEGIN_ASYNCFRAME_NS

class IState;
class CActionSet;
class CActionInfo;
class CMsgBase;
using namespace spp::comm;

#define FRAME_LOG GetServerBase()->log_.LOG_P_ALL

class CAsyncFrame
{
    public:
        enum CBType {
            CBType_Init = 0,        // 请求开始处理时回调
            CBType_Fini,            // 请求处理完成时回调
            CBType_Overload,        // 异步框架过载时回调
            CBType_Num = 10,
        };

        typedef int (*CBFunc)(CAsyncFrame*, CMsgBase*);     // 回调函数原型
        typedef std::map<int, IState*>  StateMap;           // 状态Map

        friend class CActionSet;
        friend class CActionInfo;

    public:
        static CAsyncFrame* Instance (void);
        static void Destroy (void);

        CAsyncFrame();
        ~CAsyncFrame();

        /**
         * 初始化异步框架，由插件在worker进程的spp_handle_init里调用
         *
         * @param pServBase CServerBase对象指针
         * @param pPollerUnit CPollerUnit对象指针，从旧版本的异步接口获取
         * @param pTimerUnit CTimerUnit对象指针，从旧版本的异步接口获取
         * @param nPendingConnNum Pending类型连接数上限
         *
         * @return 0: 成功； 其他：失败
         *
         */
        int InitFrame(CServerBase *pServBase, 
                        CPollerUnit *pPollerUnit,
                        CTimerUnit *pTimerUnit,
                        int nPendingConnNum = 100);

        /**
         * 初始化异步框架，由插件在worker进程的spp_handle_init里调用
         * 本接口在spp2.2.0版本中引入，增加异步框架过载保护功能
         *
         * @param pServBase CServerBase对象指针
         * @param nPendingConnNum Pending类型连接数上限
         * @param nOverloadLimit 异步框架过载上限(限制异步框架当前处理的请求数)
         *                      过载时会回调CBType_Overload类型函数, 0:无过载限制
         * @param nL5Timeout 与L5Agent交互超时设置
         * @param nL5Port l5agent bind port， 只有非标准L5部署环境下才需要设置
         *
         * @return 0: 成功； 其他：失败
         *
         */
        int InitFrame2(CServerBase *pServBase, 
                            int nPendingConnNum = 100,
                            int nOverloadLimit = 0,
                            int nL5Timeout = 100,
                            unsigned short nL5Port = 8888);
        /**
         * 反初始化异步框架，由插件在worker进程的spp_handle_fini里调用
         *
         */
        void FiniFrame();

        /**
         * 注册回调函数，两种回调函数类型：
         * 1) CBType_Init // 请求开始处理时回调, 该回调函数返回值为处理请求的第一个状态ID，默认状态ID为1
         * 2) CBType_Fini // 请求处理完成时回调, 当请求处理完毕（可能发生错误）时回调，业务可以在这个回调函数里进行回包
         * 3) CBType_Overload // 框架过载时回调, 必须使用InitFrame2方法初始化框架，
         *                       并且OverloadLimit > 0，请求数超过OverloadLimit时被调用
         *
         * @param type 回调函数类型
         * @param func 回调函数
         *
         * @return 0: 成功；其他：失败
         */
        int RegCallback(CBType type, CBFunc func);

        /**
         * 添加处理请求时使用的所有IState派生类对象指针，通常由插件在spp_handle_init方法里调用
         *
         * @param id 状态ID, 必须>0
         * @param pState IState派生类对象指针
         *
         * @return 0: 成功；其他：失败
         */
        int AddState( int id, IState *pState); 

        /**
         * 处理请求，由插件在spp_handle_process方法里调用
         *
         * @param pMsg CMsgBase派生类对象指针，存放和请求相关的数据，该对象需要插件以new的方式分配，释放由框架负责
         *
         * @return 0: 成功； 其他：失败
         *
         */ 
        int Process(CMsgBase *pMsg);

        /**
         * 获取CServerBase对象指针，该对象提供日志、统计功能
         * 宏定义FRAME_LOG是为了简化日志记录的。
         *
         * @return CServerBase对象指针
         **/
        inline CServerBase *GetServerBase()
        {
            return _pServBase;
        }

        /**
         *  @brief Get msg count processing
         *  @return msg count process
         */
        int GetMsgCount();

    protected:
        // 启动状态处理
        int HandleEncode(int id, CMsgBase *pMsgBase);

        // 状态完成（所有动作都处理完毕）后的处理
        int HandleProcess(IState *pState, CActionSet *pActionSet, CMsgBase *pMsg);

        // 清空状态表
        void ClearStateMap();

    protected:
        static CAsyncFrame *_s_instance;
        CServerBase *_pServBase;
        CPtrMgr<CActionSet, PtrType_New> *_asptr_mgr;
        CPtrMgr<CMsgBase, PtrType_New> *_msgptr_mgr;

        CBFunc _cbFunc[CBType_Num];
        
        StateMap _mapState; 

        int _nOverloadLimit;    // 框架过载上限，0：无过载保护
        int _nRequestCount;     // 框架正在处理的请求计数

        CActionSet *_pNeedDeleteAS;

};

END_ASYNCFRAME_NS

#endif
