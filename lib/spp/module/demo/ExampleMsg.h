#include "syncincl.h"

class ExampleMsg : public CSyncMsg 
{
public:
    ExampleMsg(){}

    /**
     * @brief 同步消息处理函数
     * @return 0, 成功-用户自己回包到前端,框架不负责回包处理
     *         其它, 失败-框架关闭与proxy连接, 但不负责回业务报文
     */
    virtual int HandleProcess(); 
	
};

