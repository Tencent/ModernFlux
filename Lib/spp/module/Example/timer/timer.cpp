#include "sppincl.h"

int time_task_demo(int sid, void* cookie, void* server)
{
	void* user_arg = cookie;
	CServerBase* base = (CServerBase*)server;

	printf("sid: %d, timeout [%lu]\n", sid, time(NULL));
	return 0;
}

//��ʼ����������ѡʵ�֣�; ͨ�ú��� proxy work
//arg1:	�����ļ�
//arg2:	��������������
//����0�ɹ�����0ʧ��
extern "C" int spp_handle_init(void* arg1, void* arg2)
{
    //�������������ļ�
    //const char* etc = (const char*)arg1;
    //��������������
    CServerBase* base = (CServerBase*)arg2;

    // ͬ������״̬��ʼ��
    if (base->servertype() == SERVER_TYPE_WORKER)
    {
		SPP_ASYNC::CreateTmSession(1, 1000, time_task_demo, NULL);
    }    

    return 0;
}

//���ݽ��գ�����ʵ�֣�; Proxy
//flow:	�������־
//arg1:	���ݿ����
//arg2:	��������������
//����ֵ��> 0 ��ʾ�����Ѿ����������Ҹ�ֵ��ʾ���ݰ��ĳ���
// == 0 ��ʾ���ݰ���δ��������
// < 0 ������ʾ��������Ͽ�����
extern "C" int spp_handle_input(unsigned flow, void* arg1, void* arg2)
{
    //���ݿ���󣬽ṹ��ο�tcommu.h
    blob_type* blob = (blob_type*)arg1;
    //extinfo����չ��Ϣ
    //TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;
    //��������������
    //CServerBase* base = (CServerBase*)arg2;

    return blob->len;
}

//·��ѡ�񣨿�ѡʵ�֣�Proxy
//flow:	�������־
//arg1:	���ݿ����
//arg2:	��������������
//����ֵ��ʾworker�����
extern "C" int spp_handle_route(unsigned flow, void* arg1, void* arg2)
{
    //���ݿ���󣬽ṹ��ο�tcommu.h
    //blob_type* blob = (blob_type*)arg1;
    //��������������
    //CServerBase* base = (CServerBase*)arg2;
    return 1;
}

//���ݴ�������ʵ�֣�WORKER
//flow:	�������־
//arg1:	���ݿ����
//arg2:	��������������
//����0��ʾ�ɹ�����0ʧ�ܣ����������Ͽ����ӣ�
extern "C" int spp_handle_process(unsigned flow, void* arg1, void* arg2)
{
    //���ݿ���󣬽ṹ��ο�tcommu.h
    blob_type* blob = (blob_type*)arg1;
    //������Դ��ͨѶ�������
    CTCommu* commu = (CTCommu*)blob->owner;
    //extinfo����չ��Ϣ
    TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;
    //��������������
    CServerBase* base = (CServerBase*)arg2;

    return 0;
}

//������Դ����ѡʵ�֣�
//arg1:	��������
//arg2:	��������������
extern "C" void spp_handle_fini(void* arg1, void* arg2)
{
    // ��������������
    CServerBase* base = (CServerBase*)arg2;
}
