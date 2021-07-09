/**
 *  @file mt_msg.h
 *  @info 微线程同步消息的基类
 **/

#ifndef __MT_MSG_H__
#define __MT_MSG_H__

namespace NS_MICRO_THREAD {

/**
 * @brief  微线程消息处理基类
 */
class IMtMsg
{
public:

    /**
     * @brief  微线程消息类的处理流程入口函数
     * @return 0 -成功, < 0 失败 
     */
    virtual int HandleProcess() { return -1; };
 
    /**
     * @brief  微线程消息基类构造与析构
     */
    IMtMsg() {};
    virtual ~IMtMsg() {};
};


}

#endif

