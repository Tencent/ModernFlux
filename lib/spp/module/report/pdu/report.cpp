#include "sppincl.h"
#include "qmf_protocal_define.h"

using namespace QMF_PROTOCAL;

/*
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
	
	//包长度检查
	if (len <= 25)
	{
		return -1;
	}
	else
	{
		pMsg += 24;
		len -= 25;
	}

	QMF_PROTOCAL::QmfDownstream resp;
	
	if (resp.Decode((uint8_t *)pMsg, &len, NULL) != 0)
		return -1;
	
	rpt->set_cmd(resp.ServiceCmd.c_str());
	rpt->set_result(resp.BizCode);
	
	return 0;
}
