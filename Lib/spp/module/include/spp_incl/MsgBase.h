/*
 * =====================================================================================
 *
 *       Filename:  MsgBase.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/27/2010 10:20:44 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __MSGBASE_H__
#define __MSGBASE_H__

#include <serverbase.h>
#include <tcommu.h>
#include <string>
#include <sstream>
#include "CommDef.h"
#include <sys/time.h>

BEGIN_ASYNCFRAME_NS


using namespace spp::comm;
using namespace tbase::tcommu;


class CMsgBase
{
    public:

    public:
        CMsgBase();
        virtual ~CMsgBase();

        /**
         * 设置CServerBase对象指针，由插件在创建CMsgBase派生类对象之后设置
         */
        inline void SetServerBase(CServerBase* srvbase)
        {
            _srvbase = srvbase;
        }

        /**
         * 获取CServerBase对象指针
         */
        inline CServerBase* GetServerBase()
        {
            return _srvbase;
        }

        /**
         * 设置CTCommu对象指针，由插件在创建CMsgBase派生类对象之后设置
         * CTCommu对象在回包时候使用
         */
        inline void SetTCommu(CTCommu* commu)
        {
            _commu = commu;
        }

        /**
         * 获取CTCommu对象指针
         */
        inline CTCommu* GetTCommu()
        {
            return _commu;
        }

        /**
         * 设置请求包的flow值，由插件在创建CMsgBase派生类对象之后设置
         */
        inline void SetFlow(unsigned flow)
        {
            _flow = flow;
        }

        /**
         * 获取请求包的flow值
         */
        inline unsigned GetFlow()
        {
            return _flow;
        }

        /**
         * 设置信息输出标志
         *
         * @param flag true为打开输出，false为关闭输出
         *
         */
        inline void SetInfoFlag(bool flag)
        {
            _info_flag = flag;
        }

        /**
         * 获取信息输出标志
         *
         * @return 返回信息输出标志
         *
         */
        inline bool GetInfoFlag()
        {
            return _info_flag;
        }

        /**
         * 获取请求相关的详细信息
         *
         * @param info 详细信息
         */
        inline void GetDetailInfo(std::string &info)
        {
            info = _info.str();
        }

        /**
         * 输出信息
         *
         * @param str 待输出的信息字符串
         */ 
        void AppendInfo(const char *str);

        /**
         * 给客户端回包
         *
         * @param blob 回报内容
         */
        int SendToClient(blob_type &blob);

        /**
         * 设置请求处理总体超时
         *
         * Action处理之前，检查是否超时，如果超时，不会进入Action的实际处理流程，回调IAction::HandleError(EMsgTimeout)
         *
         * @param timeout 请求处理超时配置，单位：ms，默认为0, 0：无需检查请求总体处理超时
         *
         */
        void SetMsgTimeout(int timeout);

        /**
         * 获取请求处理总体超时
         *
         * @return 请求处理超时配置
         *
         */
        int GetMsgTimeout();

        /**
         * 检查请求处理是否超时？
         *
         * 请求处理的开始时间：CMsgBase对象创建时刻
         *
         * @return true: 超时，false: 还没有超时
         *
         */
        bool CheckMsgTimeout();


        /**
         * 获取请求处理时间开销
         *
         * @return 请求处理时间开销，单位: ms
         */
        int GetMsgCost();

    protected:
        CServerBase* _srvbase;
        CTCommu*     _commu;
        unsigned     _flow;

        bool        _info_flag;
        std::stringstream _info;

        int             _msg_timeout;   // 请求处理超时配置
        struct timeval  _start_tv;      // 请求处理开始时间
};

END_ASYNCFRAME_NS

#endif
