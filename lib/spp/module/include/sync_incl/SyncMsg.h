/**
 *  @file SyncMsg.h
 *  @info �̳���msgbase��ͷ�ļ�����, ��Ҫ��չ��blob��Ϣ
 *  @time 20130515
 **/

#ifndef __NEW_SYNC_MSG_EX_H__
#define __NEW_SYNC_MSG_EX_H__
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include "tcommu.h"
#include "serverbase.h"

#include "mt_incl.h"

using std::string;
using namespace NS_MICRO_THREAD;
using namespace tbase;
using namespace tbase::tcommu;

USING_ASYNCFRAME_NS;

namespace SPP_SYNCFRAME {

/**
 * @brief ΢�߳�ͬ����Ϣ�ӿ�
 */
class CSyncMsg : public IMtMsg
{
    public:
        /**
         * ������������
         */
        CSyncMsg() {
            _srvbase        = NULL;
            _commu          = NULL;
            _flow           = 0;
            _start_time     = mt_time_ms();
            
            memset(&_from_addr, 0, sizeof(_from_addr));
            memset(&_local_addr, 0, sizeof(_local_addr));
            memset(&_time_rcv, 0, sizeof(_time_rcv));
        };
        virtual ~CSyncMsg(){};


        /**
         * ͬ����Ϣ������
         * @return 0, �ɹ�-�û��Լ��ذ���ǰ��,��ܲ�����ذ�����
         *         ����, ʧ��-��ܹر���proxy����, ���������ҵ����
         */
        virtual int HandleProcess(){ 
            return -1;
        };

        /**
         * ����CServerBase����ָ�룬�ɲ���ڴ���CMsgBase���������֮������
         */
        void SetServerBase(CServerBase* srvbase)
        {
            _srvbase = srvbase;
        };

        /**
         * ��ȡCServerBase����ָ��
         */
        CServerBase* GetServerBase()
        {
            return _srvbase;
        };

        /**
         * ����CTCommu����ָ�룬�ɲ���ڴ���CMsgBase���������֮������
         * CTCommu�����ڻذ�ʱ��ʹ��
         */
        void SetTCommu(CTCommu* commu)
        {
            _commu = commu;
        };

        /**
         * ��ȡCTCommu����ָ��
         */
        CTCommu* GetTCommu()
        {
            return _commu;
        };

        /**
         * �����������flowֵ���ɲ���ڴ���CMsgBase���������֮������
         */
        void SetFlow(unsigned flow)
        {
            _flow = flow;
        };

        /**
         * ��ȡ�������flowֵ
         */
        unsigned GetFlow()
        {
            return _flow;
        };

        /**
         * ���ͻ��˻ذ�
         *
         * @param blob �ر�����
         */
        int SendToClient(blob_type &blob) {
            if (NULL == _commu) {
                return -999;
            }
            
            int ret = _commu->sendto( _flow, &blob, _srvbase);
            return ret;
        };
       

        /**
         * �������������峬ʱ
         *
         * Action����֮ǰ������Ƿ�ʱ�������ʱ���������Action��ʵ�ʴ������̣��ص�IAction::HandleError(EMsgTimeout)
         *
         * @param timeout ������ʱ���ã���λ��ms��Ĭ��Ϊ0, 0���������������崦��ʱ
         *
         */
        void SetMsgTimeout(int timeout) {
            _msg_timeout = timeout;
        };

        /**
         * ��ȡ���������峬ʱ
         *
         * @return ������ʱ����
         *
         */
        int GetMsgTimeout() {
            return _msg_timeout;
        };
        
        /**
         * ��ȡ������ʱ�俪��
         *
         * @return ������ʱ�俪������λ: ms
         */
        int GetMsgCost() {
            return (int)(mt_time_ms() - _start_time);
        };

        /**
         * ����������Ƿ�ʱ��
         *
         * ������Ŀ�ʼʱ�䣺CMsgBase���󴴽�ʱ��
         *
         * @return true: ��ʱ��false: ��û�г�ʱ
         *
         */
        bool CheckMsgTimeout() {
            if (_msg_timeout <= 0) {// ������������ʱ
                return false;
            }
            
            int cost = GetMsgCost();
            if (cost < _msg_timeout) {// δ��ʱ
                return false;
            }
            
            return true;
        };

        /**
         * ����Ŀ�ĵ�ַ�������ȡ�ӿ�
         *
         * @param addr �����ַ��Ϣ
         */ 
        void SetLocalAddr(const struct sockaddr_in& local_addr) {
            memcpy(&_local_addr, &local_addr, sizeof(_local_addr));
        };
        void GetLocalAddr(struct sockaddr_in& local_addr) {
            memcpy(&local_addr, &_local_addr, sizeof(_local_addr));
        };

        /**
         * ������Դ��ַ�������ȡ�ӿ�
         *
         * @param addr �����ַ��Ϣ
         */ 
        void SetFromAddr(const struct sockaddr_in& from_addr) {
            memcpy(&_from_addr, &from_addr, sizeof(_from_addr));
        };
        void GetFromAddr(struct sockaddr_in& from_addr) {
            memcpy(&from_addr, &_from_addr, sizeof(_from_addr));
        };

        /**
         * ������Դʱ�����Ϣ
         *
         * @param time_rcv �������ʱ���
         */ 
        void SetRcvTimestamp(const struct timeval& time_rcv) {
            memcpy(&_time_rcv, &time_rcv, sizeof(time_rcv));
        };
        void GetRcvTimestamp(struct timeval& time_rcv) {
            memcpy(&time_rcv, &_time_rcv, sizeof(time_rcv));
        };  

        /**
         * ����������Ϣ�洢, ��ѡ����
         *
         * @param pkg ������ʼָ��
         * @param len ���ĳ���
         */ 
        void SetReqPkg(const void* pkg, int pkg_len) {
            _msg_buff.assign((char*)pkg, pkg_len);
        };
        
        const string& GetReqPkg() {
            return _msg_buff;
        };

    protected:
        CServerBase* _srvbase;              // ������base
        CTCommu*     _commu;                // ����ͨѶ��
        unsigned     _flow;                 // ������ID
        int          _msg_timeout;          // ������ʱ����
        unsigned long long _start_time;     // ��Ϣ����ʱ�Ӽ�¼
    
        struct sockaddr_in _from_addr;      // ������ԴIP
        struct sockaddr_in _local_addr;     // ���Ľ��յı���IP
        struct timeval _time_rcv;           // �հ���ʱ���
        string _msg_buff;                   // ���Ŀ���һ��, ���̴߳�����Ҫ

};


}

#endif
