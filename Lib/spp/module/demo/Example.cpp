#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ExampleMsg.h"

/**
 * @brief 业务模块初始化插件接口（可选实现proxy,worker）
 * @param arg1 - 配置文件
 * @param arg2 - 服务器容器对象
 * @return 0 - 成功, 其它失败
 */
extern "C" int spp_handle_init(void* arg1, void* arg2)
{
    const char * etc  = (const char*)arg1;
    CServerBase* base = (CServerBase*)arg2;

    base->log_.LOG_P_PID(LOG_DEBUG, "spp_handle_init, config:%s, servertype:%d\n", etc, base->servertype());

    if (base->servertype() == SERVER_TYPE_WORKER)
    {
        /* 初始化框架 */
        int iRet = CSyncFrame::Instance()->InitFrame(base, 100000);
        if (iRet < 0)
        {
            base->log_.LOG_P_PID(LOG_FATAL, "Sync framework init failed, ret:%d\n", iRet);
            return -1;
        }
		
		/* 业务自身初始化 */
		// ......
    }
    
    return 0;
}


/**
 * @brief 业务模块检查报文合法性与分包接口(proxy)
 * @param flow - 请求包标志
 * @param arg1 - 数据块对象
 * @param arg2 - 服务器容器对象
 * @return ==0  数据包还未完整接收,继续等待
 *         > 0  数据包已经接收完整, 返回包长度
 *         < 0  数据包非法, 连接异常, 将断开TCP连接
 */
extern "C" int spp_handle_input(unsigned flow, void* arg1, void* arg2)
{
    blob_type* blob = (blob_type*)arg1;
    TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;
    CServerBase* base = (CServerBase*)arg2;

    base->log_.LOG_P(LOG_DEBUG, "spp_handle_input, %d, %d, %s\n",
                     flow,
                     blob->len,
                     inet_ntoa(*(struct in_addr*)&extinfo->remoteip_));

    return blob->len;
}

/**
 * @brief 业务模块报文按worker组分发接口(proxy)
 * @param flow - 请求包标志
 * @param arg1 - 数据块对象
 * @param arg2 - 服务器容器对象
 * @return 处理该报文的worker组id
 */
extern "C" int spp_handle_route(unsigned flow, void* arg1, void* arg2)
{
    return 1;
}

/**
 * @brief 业务模块报文,worker组的处理接口(worker)
 * @param flow - 请求包标志
 * @param arg1 - 数据块对象
 * @param arg2 - 服务器容器对象
 * @return 0 - 成功,其它表示失败
 */
extern "C" int spp_handle_process(unsigned flow, void* arg1, void* arg2)
{
    blob_type   * blob    = (blob_type*)arg1;
    TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;

    CServerBase* base  = (CServerBase*)arg2;
    CTCommu    * commu = (CTCommu*)blob->owner;

    base->log_.LOG_P_PID(LOG_DEBUG, "spp_handle_process, %d, %d, %s\n",
                         flow,
                         blob->len,
                         inet_ntoa(*(struct in_addr*)&extinfo->remoteip_));

    /* 简单的单发单收模型示例  */
    ExampleMsg *msg = new ExampleMsg;
    if (!msg) {
        blob_type respblob;
        respblob.data  = NULL;
        respblob.len   = 0;
        commu->sendto(flow, &respblob, NULL);
        base->log_.LOG_P_PID(LOG_ERROR, "close conn, flow:%u\n", flow);

        return -1;
    }

    /* 设置msg信息 */
    msg->SetServerBase(base);
    msg->SetTCommu(commu);
    msg->SetFlow(flow);
    msg->SetMsgTimeout(100);
    msg->SetReqPkg(blob->data, blob->len); /* 微线程有独立空间,这里要拷贝一次报文 */

    CSyncFrame::Instance()->Process(msg);

    return 0;
}


/**
 * @brief 业务服务终止接口函数(proxy/worker)
 * @param arg1 - 保留
 * @param arg2 - 服务器容器对象
 * @return 0 - 成功,其它表示失败
 */
extern "C" void spp_handle_fini(void* arg1, void* arg2)
{
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P(LOG_DEBUG, "spp_handle_fini\n");

    if ( base->servertype() == SERVER_TYPE_WORKER )
    {
        CSyncFrame::Instance()->Destroy();
    }
}

#if 0
/**
 * @brief 提取模调上报信息的回调函数
 * @param flow - 请求包标志
 * @param arg1 - 数据块对象
 * @param arg2 - 上报实例
 * @return 0-成功, >0 无需上报, <0 失败上报异常
 */
extern "C" int spp_handle_report(unsigned flow, void* arg1, void* arg2)
{
	blob_type   * blob    = (blob_type*)arg1;
	CReport     * rpt     = (CReport *)arg2;

	char *pMsg = blob->data;
	int len = blob->len;
	
	uint32_t cmd = 0;
	int ret = 0;
	
	rpt->set_cmd(cmd);
	rpt->set_result(ret);
	
	return 0;
}
#endif
