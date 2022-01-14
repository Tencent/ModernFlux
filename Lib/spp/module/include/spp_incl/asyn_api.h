#ifndef _ASYN_API_H
#define _ASYN_API_H


#define	 RESERVED_SESSION_ID_SUP  500

#define  RESERVED_SESSION_ID_INF  1

extern "C" {

    enum SESSION_EVENT
    {
        EVENT_NORMAL = 1,
        EVENT_DEFAULT_PROC = 2,
        EVENT_SPECIAL_TASK_PROC = 3,
        EVENT_SHUTDOWN_COM = 4,
        EVENT_TIMEOUT = 5,
        EVENT_INPUT_ERR = 6,
        EVENT_DETACH = 7,
        EVENT_SYS_QUIT = 8,
        EVENT_OVERACCESS = 9,
        EVENT_SEND_FAIL = 10,
        EVENT_SEND_SUCC = 11
    };

    enum INIT_CONSTANT
    {
        NO_COUNTER_LIMIT = -1,
        NO_TIME_LIMIT = -1
    };

    enum ADDR_CMP_ENUM
    {
        ADDR_EQ = 1,
        ADDR_LT = 2,
        ADDR_GT = 3,
        ADDR_LE = 4,
        ADDR_GE = 5,
        ADDR_TYPE_MISMATCH = 6
    };

    enum ADDR_TYPE
    {
        ADDR_CHAR = 1,
        ADDR_INT = 2,
        ADDR_SHORT = 3,
        ADDR_STRING = 4
    };


    struct addrunit {
        int type;
        int opt;
        int len;
        union {
            char   ch1;
            short  s1;
            int    it1;
            char   buf[0];
        } val;

    } __attribute__((aligned(4)));




#define ADDR_BUF_PAD            ((char*)(sizeof(addrunit))-((addrunit*)0)->val.buf)

    struct addrset {
        int count;
        addrunit **unit;
    };


    enum API_RET_CODE
    {
        API_RET_SUCCESS = 0,
        API_RET_ERR = -1,
        API_ILLEGAL_PARA = -2,
        API_REG_FAIL = -3,
        API_RET_SUCCESS_AND_DETACH = -4
    };



    typedef int (*recv_func)(int event, int sessionId, void* comm_param, char* buf, int len);

    typedef int (*input_func)(void* input_param, unsigned sessionId , void* blob, void* server);

    typedef int (*process_func)(int event, int sessionId, void* proc_param, void* data_blob, void* server);

    typedef int (*pack_cb_func)(int event, int sessionId, void* global_param, void* package_param, void* data_blob, void* server);

	//增加第二套接口，用于input, output, exception事件
	typedef int (*raw_handle_func)(unsigned sessionId, int* fd, void* param, void* server);

    typedef int (*time_task_func)(int sessionId, void* task_para, void* server);

}
#endif
