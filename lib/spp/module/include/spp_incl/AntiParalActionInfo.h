/*
 * =====================================================================================
 *
 *       Filename:  AntiParalActionInfo.h
 *
 *    Description:  防并发路由机制的Action Info
 *
 *        Version:  1.0
 *        Created:  11/19/2010 11:43:11 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __ANTI_PARAL_ACTION_INFO_H__
#define __ANTI_PARAL_ACTION_INFO_H__


#include <stdint.h>
#include "ActionInfo.h"


BEGIN_ASYNCFRAME_NS

class CAntiParalActionInfo
    : public CActionInfo
{
    public:
        /**
         * 构造函数
         * 
         * @param init_buf_size 内部buffer初始大小，该buffer用于存放请求包和响应包
         */
        CAntiParalActionInfo(int init_buf_size);
        virtual ~CAntiParalActionInfo();

        /**
         * 设置modid，cmdid, key, 由插件调用
         * 
         * @param modid modid
         * @param cmdid cmdid
         * @param key   key
         * 
         */
        void SetRouteKey(int modid, int cmdid, int64_t key);

        /**
         * 获取modid，cmdid, key
         */
        void GetRouteKey(int &modid, int &cmdid, int64_t &key);

        /**
         * 获取modid，cmdid
         * 
         */ 
        void GetRouteID( int &modid, int &cmdid );

        /**
         * 设置理想的ip:port，由框架调用，插件无需调用
         *
         * 尽量将同一个key的请求路由到相同的ip:port，但是当此ip:port过载时，会分配其他ip:port
         *
         * @param ideal_ip 理想ip
         * @param ideal_port 理想port
         *
         */
        void SetIdealHost( const std::string &ideal_ip, PortType ideal_port );

        /**
         * 获取理想的ip:port
         *
         * 尽量将同一个key的请求路由到相同的ip:port，但是当此ip:port过载时，会分配其他ip:port;
         * 实际使用的Host应该从GetDestIp/GetDestPort方法中获取
         *
         * @param ideal_ip 理想ip
         * @param ideal_port 理想port
         *
         */
        void GetIdealHost( std::string &ideal_ip, PortType &ideal_port );

    protected:
        int _modid;
        int64_t _key;

        int _cmdid;

        std::string _ideal_ip;
        PortType _ideal_port;

};

END_ASYNCFRAME_NS

#endif
