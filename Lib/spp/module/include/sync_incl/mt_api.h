/**
 *  @filename mt_api.h
 *  @info  ΢�̷߳�װϵͳapi, ͬ������΢�߳�API��ʵ���첽����
 */
 
#ifndef __MT_API_H__
#define __MT_API_H__
 
#include <netinet/in.h>
#include <vector>

using std::vector;

namespace NS_MICRO_THREAD {

/******************************************************************************/
/*  ΢�߳��û��ӿڶ���: UDP�������շ��ӿ�                                     */
/******************************************************************************/

/**
 * @brief ��������˿ڵ�socket�շ��ӿ�, ��socket������������, ҵ������֤������
 *        [ע��] UDP����buff, ���Ƽ�static����, ��һ�����յ��������Ĵ���[��Ҫ]
 * @param dst -�����͵�Ŀ�ĵ�ַ
 * @param pkg -�������װ�İ���
 * @param len -�������װ�İ��峤��
 * @param rcv_buf -����Ӧ�����buff
 * @param buf_size -modify-����Ӧ�����buff��С, �ɹ�����ʱ, �޸�ΪӦ�������
 * @param timeout -��ʱʱ��, ��λms
 * @return  0 �ɹ�, -1 ��socketʧ��, -2 ��������ʧ��, -3 ����Ӧ��ʧ��, �ɴ�ӡerrno
 */
int mt_udpsendrcv(struct sockaddr_in* dst, void* pkg, int len, void* rcv_buf, int& buf_size, int timeout);


/******************************************************************************/
/*  ΢�߳��û��ӿڶ���: TCP���ӳ��շ��ӿ�                                     */
/******************************************************************************/

/**
 * @brief TCP��ⱨ���Ƿ���������Ļص���������
 * @param buf ���ı��滺����
 * @param len �Ѿ����յĳ���
 * @return >0 ʵ�ʵı��ĳ���; 0 ����Ҫ�ȴ�����; <0 �����쳣
 */
typedef int (*MtFuncTcpMsgLen)(void* buf, int len);

/**
 * @brief TCP��������ӳصķ�ʽ����IP/PORT����, ���ӱ���Ĭ��10����
 *        [ע��] tcp���շ���buff, ��������static����, ����������Ĵ��� [��Ҫ]
 * @param dst -�����͵�Ŀ�ĵ�ַ
 * @param pkg -�������װ�İ���
 * @param len -�������װ�İ��峤��
 * @param rcv_buf -����Ӧ�����buff
 * @param buf_size -modify-����Ӧ�����buff��С, �ɹ�����ʱ, �޸�ΪӦ�������
 * @param timeout -��ʱʱ��, ��λms
 * @param check_func -��ⱨ���Ƿ�ɹ����ﺯ��
 * @return  0 �ɹ�, -1 ��socketʧ��, -2 ��������ʧ��, -3 ����Ӧ��ʧ��, 
 *          -4 ����ʧ��, -5 ��ⱨ��ʧ��, -6 ���տռ䲻��, -7 ��������ر����ӣ�-10 ������Ч
 */
int mt_tcpsendrcv(struct sockaddr_in* dst, void* pkg, int len, void* rcv_buf, int& buf_size, 
                  int timeout, MtFuncTcpMsgLen chek_func);


enum MT_TCP_CONN_TYPE
{
    MT_TCP_SHORT         = 1, /// ������
    MT_TCP_LONG          = 2, /// ������
    MT_TCP_SHORT_SNDONLY = 3, /// ������ֻ��
    MT_TCP_LONG_SNDONLY  = 4, /// ������ֻ��
    MT_TCP_BUTT
};

/**
 * @brief TCP�շ��ӿڣ�����ѡ���˱������ӻ��߶�����
 *        [ע��] tcp���շ���buff, ��������static����, ����������Ĵ��� [��Ҫ]
 * @param dst -�����͵�Ŀ�ĵ�ַ
 * @param pkg -�������װ�İ���
 * @param len -�������װ�İ��峤��
 * @param rcv_buf -����Ӧ�����buff��ֻ�����տ�������ΪNULL
 * @param buf_size -modify-����Ӧ�����buff��С, �ɹ�����ʱ, �޸�ΪӦ������ȣ�ֻ�����գ�����ΪNULL
 * @param timeout -��ʱʱ��, ��λms
 * @param check_func -��ⱨ���Ƿ�ɹ����ﺯ��
 * @param type - ��������
 *               MT_TCP_SHORT: һ��һ�������ӣ�
 *               MT_TCP_LONG : һ��һ�ճ����ӣ�
 *               MT_TCP_LONG_SNDONLY : ֻ�����ճ����ӣ� 
 *               MT_TCP_SHORT_SNDONLY: ֻ�����ն����ӣ�
 * @return  0 �ɹ�, -1 ��socketʧ��, -2 ��������ʧ��, -3 ����Ӧ��ʧ��, 
 *          -4 ����ʧ��, -5 ��ⱨ��ʧ��, -6 ���տռ䲻��, -7 ��������ر�����, -10 ������Ч
 */
int mt_tcpsendrcv_ex(struct sockaddr_in* dst, void* pkg, int len, void* rcv_buf, int* buf_size,
                     int timeout, MtFuncTcpMsgLen func, MT_TCP_CONN_TYPE type = MT_TCP_LONG);

 /**
 * @brief TCP��ⱨ���Ƿ���������Ļص�����
 * @param buf ���ı��滺����
 * @param len �Ѿ����յĳ���
 * @param closed Զ�˹ر����ӣ�����ͨ�����ӹر���ȷ�����ĳ��ȵ�ҵ��
 * @param msg_ctx    �������ĵ������ı�����ͨ����ֵ��ҵ�����ʵ�ֱ��Ľ���״̬���ȣ������ظ����
 *               ҵ����mt_tcpsendrcv�д���ò�������ܻص�ʱ���ݸ�ҵ�񡣸ñ�������Ϊ����߽ṹ�壬ҵ���Զ��塣
 *               ���Ľ�������������ڱ����У�mt_tcpsendrcv���غ�ֱ��ʹ��
 * ��ע�⡿ �ûص����������bufָ����ܱ仯�����Ա��Ľ���״̬�����λ����Ϣ��ʹ��ƫ����   
 * @param msg_len_detected �������� ���ҵ��ȷ���˱��ĳ��ȣ���������Ϊtrue������Ϊfalse������ʱΪfalse��
 *         ��ע�⡿�����ֵ����true������Ժ�������ֵ��Ϊ���ĳ��ȣ�>0�������հ������հ����ǰ�����ٵ��ø�MtFuncTcpMsgChecker���������Ľ�����ɺ󣬵���MtFuncTcpMsgChecker�������һ�α��ġ������������հ��������Ч���á� 
 * @return >0 ����Ľ��ջ����С; 0 ����Ҫ�ȴ�����; <0 �����쳣
 *      ��ע�⡿ 
 *            ��������������Է��ش���0:
 *               1��ҵ��ȷ�����ĳ���,���ر��ĳ���
 *               2��ҵ��ȷ�����ĳ��ȣ���������buf�Ľ��鳤�ȣ�������ȴ�������buf���ȣ���ܸ��ݸ�ֵrealloc buf��
 *            
 *           ����ֵΪ0����ʾҵ���޷�ȷ�����ĳ��ȣ���Ҫ�������ա����buf��������ܽ�realloc buf��buf��С����һ��
 *           ����ֵС��0����ʾҵ��ȷ�����Ĵ���    
 */
typedef int (*MtFuncTcpMsgChecker)(void* buf, int len, bool closed, void* msg_ctx, bool &msg_len_detected);



/**
 * @brief TCP�շ��ӿڣ�����ѡ���˱������ӻ��߶�����
 *        [ע��] tcp���շ���buff, ��������static����, ����������Ĵ��� [��Ҫ]
 * @param dst -�����͵�Ŀ�ĵ�ַ
 * @param pkg -�������װ�İ���
 * @param len -�������װ�İ��峤��
 * @param rcv_buf -���������������ο����� keep_rcv_buf��ҵ����Ҫmalloc���ڴ档void* rcv_buf=NULL: ������ܼ���
 * @param recv_pkg_size -����Ӧ�����buff�ĳ�ʼ��С����������0ʱ�����ر���ʵ�ʳ��ȡ�
 * @param timeout -��ʱʱ��, ��λms
 * @param check_func -��ⱨ���Ƿ�ɹ����ﺯ��
 * @param msg_ctx -�������ĵ������ı�����
 *
 * @param type - ��������
 *               MT_TCP_SHORT: һ��һ�������ӣ�
 *               MT_TCP_LONG : һ��һ�ճ����ӣ�
 *               MT_TCP_LONG_SNDONLY : ֻ�����ճ����ӣ� 
 *               MT_TCP_SHORT_SNDONLY: ֻ�����ն����ӣ�
 * @param keep_rcv_buf -true,���óɹ��󣬿�ܽ�����rcv_buf��ֵ������������ҵ�����ͷŸ�buf���������ڵ��ý���ǰ�ͷŸ�buf����ע�⡿
 *               ҵ����Ҫ�Լ���msg_ctx�б��������Ϣ�������ͨ��malloc�����ڴ桿  Ĭ�Ͽ���Զ��ͷŸ�buf��ҵ����Ҫ��ctx������Ϣ   
 * @return  0 �ɹ�, -1 ��socketʧ��, -2 ��������ʧ��, -3 ����Ӧ��ʧ��, 
 *          -4 ����ʧ��, -5 ��ⱨ��ʧ��, -6 ���տռ䲻��, -7 ��������ر�����, -10 ������Ч, -11,�������bufʧ��
 */
int mt_tcpsendrcv_ex(struct sockaddr_in* dst, void* pkg, int len, void*& rcv_buf, int& recv_pkg_size, 
                     int timeout, MtFuncTcpMsgChecker check_func, void* msg_ctx=NULL, 
                     MT_TCP_CONN_TYPE type = MT_TCP_LONG, bool keep_rcv_buf=false);

/**
 * @brief TCP��������ӳصķ�ʽ����IP/PORT����, ���ӱ���Ĭ��10����
 *        [ע��] tcp���շ���buff, ��������static����, ����������Ĵ��� [��Ҫ]
 * @param dst -�����͵�Ŀ�ĵ�ַ
 * @param pkg -�������װ�İ���
 * @param len -�������װ�İ��峤��
 * @param rcv_buf -���������������ο����� keep_rcv_buf��
 * @param recv_pkg_size -����Ӧ�����buff�ĳ�ʼ��С����������0ʱ�����ر���ʵ�ʳ��ȡ�
 * @param timeout -��ʱʱ��, ��λms
 * @param check_func -��ⱨ���Ƿ�ɹ����ﺯ��
 * @param msg_ctx -�������ĵ������ı�����
 * @param keep_rcv_buf -true,��ܽ�����rcv_buf��ֵ������������ҵ�����ͷŸ�buf���������ڵ��ý���ǰ�ͷŸ�buf����ע�⡿
 *               ҵ����Ҫ�Լ���msg_ctx�б��������Ϣ�������ͨ��malloc�����ڴ桿     
 * @return  0 �ɹ�, -1 ��socketʧ��, -2 ��������ʧ��, -3 ����Ӧ��ʧ��, 
 *          -4 ����ʧ��, -5 ��ⱨ��ʧ��, -6 ���տռ䲻��, -7 ��������ر�����, -10 ������Ч
 */
int mt_tcpsendrcv(struct sockaddr_in* dst, void* pkg, int len, void*& rcv_buf, int& recv_pkg_size, 
                     int timeout, MtFuncTcpMsgChecker check_func, void* msg_ctx=NULL, bool keep_rcv_buf=false);


/******************************************************************************/
/*  ΢�߳��û��ӿڶ���: ΢�߳�Task��·����ģ�ͽӿڶ���                        */
/******************************************************************************/

/**
 * @brief  ΢�߳������������
 */
class IMtTask
{
public:

    /**
     * @brief  ΢�߳�������Ĵ���������ں���
     * @return 0 -�ɹ�, < 0 ʧ�� 
     */
    virtual int Process() { return -1; };

    /**
     * @brief ����taskִ�н��
     * @info  ��Process����ֵ
     */
    void SetResult(int rc)
    {
        _result = rc;
    }

    /**
     * @brief ��ȡtaskִ�н��
     * @info  ��Process����ֵ
     */
    int GetResult(void)
    {
        return _result;
    }

    /**
     * @brief ����task����
     */
    void SetTaskType(int type)
    {
        _type = type;
    }

    /**
     * @brief  ��ȡtask����
     * @info   ���ҵ���ж���task������ʹ�ø��ֶ����ֲ�ͬ��task����
     * @return ��ȡtask����
     */
    int GetTaskType(void)
    {
        return _type;
    }
 
    /**
     * @brief  ΢�߳�������๹��������
     */
    IMtTask() {};
    virtual ~IMtTask() {};

protected:

    int _type;      // task���ͣ���������task��ҵ������Զ������ͣ�����ӻ���ת��
    int _result;    // taskִ�н������Process����ֵ
};

typedef vector<IMtTask*>  IMtTaskList;

/**
 * @brief ��·IO����, Task-fork-waitģʽ�ӿ�
 * @param req_list -task list ��װ����api��task�б�
 * @return  0 �ɹ�, -1 �������߳�ʧ��
 */
int mt_exec_all_task(IMtTaskList& req_list);


/******************************************************************************/
/*  ΢�߳��û��ӿڶ���: ΢�̷߳�װϵͳ�ӿ�                                    */
/******************************************************************************/

/**
 * @brief ΢�߳�����sleep�ӿ�, ��λms
 * @info  ҵ����Ҫ�����ó�CPUʱʹ��
 */
void mt_sleep(int ms);

/**
 * @brief ΢�̻߳�ȡϵͳʱ�䣬��λms
 */
unsigned long long mt_time_ms(void);

/******************************************************************************/
/*  ΢�߳��û��ӿڶ���: ΢�߳��û�˽�����ݽӿ�                                */
/******************************************************************************/

/**
 * @brief ���õ�ǰIMtMsg��˽�б���
 * @info  ֻ����ָ�룬�ڴ���Ҫҵ�����
 */
void mt_set_msg_private(void *data);

/**
 * @brief  ��ȡ��ǰIMtMsg��˽�б���
 * @return ˽�б���ָ��
 */
void* mt_get_msg_private();


/******************************************************************************/
/*  ΢�߳��û��ӿڶ���: ΢�̷߳�װϵͳ�ӿ�(���Ƽ�ʹ��)                        */
/******************************************************************************/

/**
 * @brief  ΢�߳̿�ܳ�ʼ��
 * @info   ҵ��ʹ��spp������΢�̣߳���Ҫ���øú�����ʼ����ܣ�
 *         ʹ��spp��ֱ�ӵ���SyncFrame�Ŀ�ܳ�ʼ����������
 * @return false:��ʼ��ʧ��  true:��ʼ���ɹ�
 */
bool mt_init_frame(void);

/**
 * @brief ����΢�̶߳���ջ�ռ��С
 * @info  �Ǳ������ã�Ĭ�ϴ�СΪ128K
 */
void mt_set_stack_size(unsigned int bytes);

/**
 * @brief ΢�̰߳�����ϵͳIO���� recvfrom
 * @param fd ϵͳsocket��Ϣ
 * @param buf ������Ϣ������ָ��
 * @param len ������Ϣ����������
 * @param from ��Դ��ַ��ָ��
 * @param fromlen ��Դ��ַ�Ľṹ����
 * @param timeout ��ȴ�ʱ��, ����
 * @return >0 �ɹ����ճ���, <0 ʧ��
 */
int mt_recvfrom(int fd, void *buf, int len, int flags, struct sockaddr *from, socklen_t *fromlen, int timeout);

/**
 * @brief ΢�̰߳�����ϵͳIO���� sendto
 * @param fd ϵͳsocket��Ϣ
 * @param msg �����͵���Ϣָ��
 * @param len �����͵���Ϣ����
 * @param to Ŀ�ĵ�ַ��ָ��
 * @param tolen Ŀ�ĵ�ַ�Ľṹ����
 * @param timeout ��ȴ�ʱ��, ����
 * @return >0 �ɹ����ͳ���, <0 ʧ��
 */
int mt_sendto(int fd, const void *msg, int len, int flags, const struct sockaddr *to, int tolen, int timeout);


/**
 * @brief ΢�̰߳�����ϵͳIO���� connect
 * @param fd ϵͳsocket��Ϣ
 * @param addr ָ��server��Ŀ�ĵ�ַ
 * @param addrlen ��ַ�ĳ���
 * @param timeout ��ȴ�ʱ��, ����
 * @return >0 �ɹ����ͳ���, <0 ʧ��
 */
int mt_connect(int fd, const struct sockaddr *addr, int addrlen, int timeout);

/**
 * @brief ΢�̰߳�����ϵͳIO���� accept
 * @param fd �����׽���
 * @param addr �ͻ��˵�ַ
 * @param addrlen ��ַ�ĳ���
 * @param timeout ��ȴ�ʱ��, ����
 * @return >=0 accept��socket������, <0 ʧ��
 */
int mt_accept(int fd, struct sockaddr *addr, socklen_t *addrlen, int timeout);

/**
 * @brief ΢�̰߳�����ϵͳIO���� read
 * @param fd ϵͳsocket��Ϣ
 * @param buf ������Ϣ������ָ��
 * @param nbyte ������Ϣ����������
 * @param timeout ��ȴ�ʱ��, ����
 * @return >0 �ɹ����ճ���, <0 ʧ��
 */
ssize_t mt_read(int fd, void *buf, size_t nbyte, int timeout);

/**
 * @brief ΢�̰߳�����ϵͳIO���� write
 * @param fd ϵͳsocket��Ϣ
 * @param buf �����͵���Ϣָ��
 * @param nbyte �����͵���Ϣ����
 * @param timeout ��ȴ�ʱ��, ����
 * @return >0 �ɹ����ͳ���, <0 ʧ��
 */
ssize_t mt_write(int fd, const void *buf, size_t nbyte, int timeout);

/**
 * @brief ΢�̰߳�����ϵͳIO���� recv
 * @param fd ϵͳsocket��Ϣ
 * @param buf ������Ϣ������ָ��
 * @param len ������Ϣ����������
 * @param timeout ��ȴ�ʱ��, ����
 * @return >0 �ɹ����ճ���, <0 ʧ��
 */
ssize_t mt_recv(int fd, void *buf, int len, int flags, int timeout);

/**
 * @brief ΢�̰߳�����ϵͳIO���� send
 * @param fd ϵͳsocket��Ϣ
 * @param buf �����͵���Ϣָ��
 * @param nbyte �����͵���Ϣ����
 * @param timeout ��ȴ�ʱ��, ����
 * @return >0 �ɹ����ͳ���, <0 ʧ��
 */
ssize_t mt_send(int fd, const void *buf, size_t nbyte, int flags, int timeout);


/**
 * @brief ΢�̵߳ȴ�epoll�¼��İ�������
 * @param fd ϵͳsocket��Ϣ
 * @param events �ȴ����¼� IN/OUT
 * @param timeout ��ȴ�ʱ��, ����
 * @return >0 ������¼�, <0 ʧ��
 */
int mt_wait_events(int fd, int events, int timeout);


void* mt_start_thread(void* entry, void* args);


}

#endif

 
