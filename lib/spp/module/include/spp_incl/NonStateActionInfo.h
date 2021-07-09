/*
 * =====================================================================================
 *
 *       Filename:  NonStateActionInfo.h
 *
 *    Description:  无状态L5路由的Action Info
 *
 *        Version:  1.0
 *        Created:  11/11/2010 03:46:59 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __NONSTATE_ACTION_INFO_H__
#define __NONSTATE_ACTION_INFO_H__

#include "ActionInfo.h"


BEGIN_ASYNCFRAME_NS

class CNonStateActionInfo
    : public CActionInfo
{
    public:
        /**
         * 构造函数
         * 
         * @param init_buf_size 内部buffer初始大小，该buffer用于存放请求包和响应包
         **/
        CNonStateActionInfo(int init_buf_size);
        virtual ~CNonStateActionInfo();

        /**
         * 设置路由ID: modid, cmdid; 由插件调用，框架根据modid，cmdid向L5申请实际路由Host
         *
         * @param modid modid
         * @param cmdid cmdid
         *
         */ 
        void SetRouteID(int modid, int cmdid);

        /**
         * 获取路由ID：modid, cmdid
         *
         */ 
        void GetRouteID(int &modid, int &cmdid);

    protected:
        int _modid;
        int _cmdid;
};


END_ASYNCFRAME_NS



#endif
