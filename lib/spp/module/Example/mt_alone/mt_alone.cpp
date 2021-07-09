/**
 * @file mt_alone.cpp
 * @info ΢�̵߳���ʹ������
 */

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mt_incl.h"

#define  REMOTE_IP      "127.0.0.1"
#define  REMOTE_PORT    9988

// Task������:ʹ��UDP�������սӿ�
class UdpSndRcvTask
    : public IMtTask
{
public:
    virtual int Process() {
        // ��ȡĿ�ĵ�ַ��Ϣ, ��ʾ��
        static struct sockaddr_in server_addr;
        static int initflg = 0;

        if (!initflg) {
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = inet_addr(REMOTE_IP);
            server_addr.sin_port = htons(REMOTE_PORT);
            initflg = 1;
        }

        char buff[1024] = "This is a udp sendrecv task example!";
        int  max_len = sizeof(buff);
        
        int ret = mt_udpsendrcv(&server_addr, (void*)buff, 100, buff, max_len, 100);
        if (ret < 0)
        {
            printf("mt_udpsendrcv failed, ret %d\n", ret);
            return -1;
        }

        return 0;
    };
};


#define PKG_LEN  100
// ��鱨���Ƿ�������
int CheckPkgLen(void *buf, int len) {
    if (len < PKG_LEN)
    {
        return 0;
    }

    return PKG_LEN;
}

// Task�����࣬ʹ��TCP���ӳص������սӿ�
class TcpSndRcvTask
    : public IMtTask
{
public:
    virtual int Process() {
        // ��ȡĿ�ĵ�ַ��Ϣ, ��ʾ��
        static struct sockaddr_in server_addr;
        static int initflg = 0;

        if (!initflg) {
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = inet_addr(REMOTE_IP);
            server_addr.sin_port = htons(REMOTE_PORT);
            initflg = 1;
        }

        char buff[1024] = "This is a tcp sendrecv task example!";
        int  max_len = sizeof(buff);
        
        int ret = mt_tcpsendrcv(&server_addr, (void*)buff, 100, buff, max_len, 100, CheckPkgLen);
        if (ret < 0)
        {
            printf("mt_udpsendrcv failed, ret %d\n", ret);
            return -1;
        }

        return 0;
    };
};

// Task������: ҵ�����������֤΢�߳�API������
class ApiVerifyTask
    : public IMtTask
{
public:
    virtual int Process() {

        // ����ҵ��ʹ��΢�߳�API
	printf("This is the api verify task!!!\n");

        return 0;
    };
};

int main(void)
{
    // ��ʼ��΢�߳̿��
    bool init_ok = mt_init_frame();
    if (!init_ok)
    {
        fprintf(stderr, "init micro thread frame failed.\n");
        return -1;
    }

    // ����΢�߳��л�
    mt_sleep(0);

    UdpSndRcvTask task1;
    TcpSndRcvTask task2;
    ApiVerifyTask task3;

    // ����ԭ���߳��Ѿ���demon�ĵ�������
    while (true)
    { // ����ʾ��һ����������
        IMtTaskList task_list;
        task_list.push_back(&task1);
        task_list.push_back(&task2);
        task_list.push_back(&task3);

        int ret = mt_exec_all_task(task_list);
        if (ret < 0)
        {
            fprintf(stderr, "execult tasks failed, ret:%d", ret);
            return -2;
        }

        // ѭ�����ÿһ��task�Ƿ�ִ�гɹ�����Process�ķ���ֵ
        for (unsigned int i = 0; i < task_list.size(); i++)
        {
            IMtTask *task = task_list[i];
            int result = task->GetResult();

            if (result < 0)
            {
                fprintf(stderr, "task(%u) failed, result:%d", i, result);
            }
        }

        // ˯��500ms
        mt_sleep(500);
    }

    return 0;
}


