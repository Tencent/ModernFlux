#include "sppincl.h"

int time_task_demo(int sid, void* cookie, void* server)
{
	void* user_arg = cookie;
	CServerBase* base = (CServerBase*)server;

	printf("sid: %d, timeout [%lu]\n", sid, time(NULL));
	return 0;
}

//初始化方法（可选实现）; 通用函数 proxy work
//arg1:	配置文件
//arg2:	服务器容器对象
//返回0成功，非0失败
extern "C" int spp_handle_init(void* arg1, void* arg2)
{
    //插件自身的配置文件
    //const char* etc = (const char*)arg1;
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;

    // 同步处理状态初始化
    if (base->servertype() == SERVER_TYPE_WORKER)
    {
		SPP_ASYNC::CreateTmSession(1, 1000, time_task_demo, NULL);
    }    

    return 0;
}

//数据接收（必须实现）; Proxy
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回值：> 0 表示数据已经接收完整且该值表示数据包的长度
// == 0 表示数据包还未接收完整
// < 0 负数表示出错，将会断开连接
extern "C" int spp_handle_input(unsigned flow, void* arg1, void* arg2)
{
    //数据块对象，结构请参考tcommu.h
    blob_type* blob = (blob_type*)arg1;
    //extinfo有扩展信息
    //TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;
    //服务器容器对象
    //CServerBase* base = (CServerBase*)arg2;

    return blob->len;
}

//路由选择（可选实现）Proxy
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回值表示worker的组号
extern "C" int spp_handle_route(unsigned flow, void* arg1, void* arg2)
{
    //数据块对象，结构请参考tcommu.h
    //blob_type* blob = (blob_type*)arg1;
    //服务器容器对象
    //CServerBase* base = (CServerBase*)arg2;
    return 1;
}

//数据处理（必须实现）WORKER
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回0表示成功，非0失败（将会主动断开连接）
extern "C" int spp_handle_process(unsigned flow, void* arg1, void* arg2)
{
    //数据块对象，结构请参考tcommu.h
    blob_type* blob = (blob_type*)arg1;
    //数据来源的通讯组件对象
    CTCommu* commu = (CTCommu*)blob->owner;
    //extinfo有扩展信息
    TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;

    return 0;
}

//析构资源（可选实现）
//arg1:	保留参数
//arg2:	服务器容器对象
extern "C" void spp_handle_fini(void* arg1, void* arg2)
{
    // 服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
}
