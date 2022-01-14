/*
 * =====================================================================================
 *
 *       Filename:  ActionSet.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/13/2010 05:35:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __ACTION_SET_H__
#define __ACTION_SET_H__

#include "CommDef.h"
#include <set>

BEGIN_ASYNCFRAME_NS

class IState;
class CActionInfo;
class CAsyncFrame;
class CMsgBase;

class CActionSet
{
    public:
        friend class CActionInfo;
        friend class CAsyncFrame;

        typedef std::set< CActionInfo* > ActionSet;
    public:
        CActionSet(CAsyncFrame *pFrame, 
                    IState *pState,
                    CMsgBase *pMsg);
        ~CActionSet();

        /**
         * 添加一个动作，由插件在IState::HandleEncode方法里调用
         *
         * @param ai CActionInfo对象指针，此对象必须有插件以new方式分配，释放由框架负责
         *
         * @return 0: 成功；其他：失败
         *
         */ 
        int AddAction( CActionInfo *ai );
        
        /**
         * 获取动作个数
         *
         * @return 返回动作个数
         *
         */
        inline int GetActionNum()
        {
            return _ActionSet.size();
        }
        
        /**
         * 获取动作集合，通常在IState::HandleProcess接口中由插件调用
         *
         * @return 动作集合
         *
         */
        inline ActionSet & GetActionSet()
        {
            return _ActionSet;
        }

        /**
         * 清除动作
         */
        void ClearAction();
        
    protected:
        int StartAction();
        int HandleFinished( CActionInfo * ai );

        IState *GetIState();
        CMsgBase *GetMsg();

    protected:
        CAsyncFrame *_pFrame;
        IState *_pState;        // 状态接口    
        CMsgBase *_pMsg;    
        ActionSet _ActionSet;   // 动作集合
        int _finished_count;    // 已经完成的动作个数
};
END_ASYNCFRAME_NS
#endif

