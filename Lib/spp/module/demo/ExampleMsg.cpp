#include "ExampleMsg.h"

/** 
 * @return 0, 成功-用户自己回包到前端,框架不负责回包处理
 *         其它, 失败-框架关闭与proxy连接, 但不负责回业务报文
 */
int ExampleMsg::HandleProcess()
{
    //请求包
    const std::string &strReqBuff = this->GetReqPkg();

    //处理过程	
    //......

    //回包
    blob_type blob;
    blob.data = (char *)strReqBuff.data();
    blob.len  = (int)strReqBuff.size();
    
    this->SendToClient(blob);
	
    return 0;
}
