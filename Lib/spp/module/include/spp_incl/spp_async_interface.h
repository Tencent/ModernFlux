#ifndef SPP_ASYNC_INTERFACE
#define SPP_ASYNC_INTERFACE

#define  RESERVED_SESSION_ID_SUP  500
#define  RESERVED_SESSION_ID_INF  1

#define  DEFAULT_MULTI_CON_INF	  10
#define  DEFAULT_MULTI_CON_SUP    50

#include "poller.h"
#include "timerlist.h"

class CPollerUnit;
class CTimerUnit;

class SPP_ASYNC
{
public:
    typedef int SID;
    typedef int CONID;

    enum SPP_ASYNC_RET_ENUM {
        ASYNC_SUCC = 0,
        ASYNC_REG_ID_OVERLAP = -1,
        ASYNC_SESSION_NONE_EXIST = -2,
        ASYNC_SEND_FAILED = -3,
        ASYNC_GENERAL_ERR = -4,
        ASYNC_REG_SESSION_ERR = -5,
        ASYNC_PARAM_ERR = -6	,
        ASYNC_CONNECT_ERR = -7,
        ASYNC_FATAL_ERR = -13
    };

    static  SID	CreateSession(const int sid, 
                                const char* type, 
                                const char* subtype, 
                                const char* ip, 
                                const int  port,
                                const int recv_cnt, 
                                const int timeout,
                                const int inf = DEFAULT_MULTI_CON_INF, 
                                const int sup = DEFAULT_MULTI_CON_SUP);

    static  SID	CreateTmSession(const int tm_sid, 
                                const int time_interval, 
                                time_task_func func, 
                                void* task_para);

	//第二套接口
	static  SID CreateSession(const int sid, 
                                int fd, 
                                raw_handle_func input_f = NULL, 
                                void* input_param = NULL, 
                                raw_handle_func output_f = NULL, 
                                void* output_param = NULL, 
                                raw_handle_func exception_f = NULL, 
                                void* exception_param = NULL);
	//end

    static  SPP_ASYNC_RET_ENUM DestroySession(SID sid);

    static  SPP_ASYNC_RET_ENUM DestroyTmSession(SID tm_sid);

    static  SPP_ASYNC_RET_ENUM RegSessionCallBack(SID id, 
                                                process_func proc, 
                                                void* proc_param, 
                                                input_func input_f, 
                                                void* input_param);
	
	//第二套接口
	static  SPP_ASYNC_RET_ENUM RegSessionCallBack(const int sid, 
                                                raw_handle_func input_f, 
                                                void* input_param, 
                                                raw_handle_func output_f, 
                                                void* output_param, 
                                                raw_handle_func exception_f, 
                                                void* exception_param);

	static  int EnableInput(const int sid);

	static  int DisableInput(const int sid);

	static  int EnableOutput(const int sid);

	static  int DisableOutput(const int sid);
	//end

    static  SPP_ASYNC_RET_ENUM RegPackCallBack(SID id, 
                                            pack_cb_func proc, 
                                            void* global_param);


    static  SPP_ASYNC_RET_ENUM SendData(SID sid, char* buf, int buflen);

    static  SPP_ASYNC_RET_ENUM SendData(SID sid, char* buf, int buflen, 
                                        void* input_param, void* proc_param);

    static  SPP_ASYNC_RET_ENUM SendData(SID sid, char* buf, int buflen, void* pack_info);

    static  SPP_ASYNC_RET_ENUM SendData(SID sid, char* buf, int buflen, 
                                        void* input_param, void* proc_param, void* pack_info);

    static  SPP_ASYNC_RET_ENUM RecycleCon(SID sid = -1, CONID conid = -1);

    static  void		   SetConnectRetryInterval(int);

    static  void		   SetIdleTimeout(int timeout,int sid=-1);

    static  void		   SetUserData(int flow, void* data);

    static  void*		   GetUserData(int flow);

    static CPollerUnit* GetPollerUnit();
    static CTimerUnit* GetTimerUnit();

};

#endif
