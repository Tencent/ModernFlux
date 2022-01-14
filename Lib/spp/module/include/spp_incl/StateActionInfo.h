/*
 * =====================================================================================
 *
 *       Filename:  StateActionInfo.h
 *
 *    Description:  L5有状态路由Action描述对象
 *
 *        Version:  1.0
 *        Created:  11/16/2010 11:15:54 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __STATE_ACTION_INFO_H__
#define __STATE_ACTION_INFO_H__

#include <stdint.h>
#include "ActionInfo.h"


BEGIN_ASYNCFRAME_NS

class CStateActionInfo
    : public CActionInfo
{
    public:
        /**
         * 构造函数
         * 
         * @param init_buf_size 内部buffer初始大小，该buffer用于存放请求包和响应包
         **/
        CStateActionInfo(int init_buf_size);
        virtual ~CStateActionInfo();

        /**
         * 设置modid，Key，funid, 由插件调用
         *
         * @param modid modid
         * @param key   key
         * @param funid 功能ID, 为cmdid的高16位
         *
         */ 
        void SetRouteKey(int modid, int64_t key, int32_t funid);
        
        /**
         * 获取modid，Key，funid
         */
        void GetRouteKey(int &modid, int64_t &key, int32_t &funid);

        /**
         * 设置cmdid, 由框架调用，cmdid由框架根据路由规则来计算
         *
         * @param cmdid
         */
        void SetCmdID( int cmdid );

        /**
         * 获取modid，cmdid，modid，cmdid唯一确定路由
         *
         */
        void GetRouteID( int &modid, int &cmdid );

    protected:
        int _modid;
        int64_t _key;
        int32_t _funid;

        int _cmdid;
};


END_ASYNCFRAME_NS

#endif
