/**
 *  @file mt_msg.h
 *  @info ΢�߳�ͬ����Ϣ�Ļ���
 **/

#ifndef __MT_MSG_H__
#define __MT_MSG_H__

namespace NS_MICRO_THREAD {

/**
 * @brief  ΢�߳���Ϣ�������
 */
class IMtMsg
{
public:

    /**
     * @brief  ΢�߳���Ϣ��Ĵ���������ں���
     * @return 0 -�ɹ�, < 0 ʧ�� 
     */
    virtual int HandleProcess() { return -1; };
 
    /**
     * @brief  ΢�߳���Ϣ���๹��������
     */
    IMtMsg() {};
    virtual ~IMtMsg() {};
};


}

#endif

