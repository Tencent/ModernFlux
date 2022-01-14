#ifndef __SPP_HANDLE_REPORT_H__
#define __SPP_HANDLE_REPORT_H__

#include <inttypes.h>
#include <stdio.h>

class CReport
{
	public:

		CReport();
		~CReport();
		
		/**
		 * @brief  参数设置完后，执行上报
		   @return 0-成功, <0 失败上报异常
		 */
		int do_report();

		/**
		 * @brief  设置命令字
		 */
		void set_cmd(const char *cmd);

		/**
		 * @brief  设置返回码
		 */
		void set_result(int32_t result);

//-----------------------------业务自行上报时(非Proxy上报),需设置以下参数-------------------------------------//

		/**
		 * @brief  设置本地IP端口
		 */
		void set_local_addr(uint32_t ip, uint32_t port);

		/**
		 * @brief  设置对端IP端口
		 */
		void set_remote_addr(uint32_t ip, uint32_t port);
		
		/**
		 * @brief  设置业务耗时
		 */
		void set_msg_cost(uint32_t cost);
		
		/**
		 * @brief  设置业务私有数据
		 */
		void set_usr_define(uint32_t cookie);
		
		
//----------------------------- 框架调用 -------------------------------------//

		/**
		 * @brief  处理上报
		   @return 0-成功, <0 失败上报异常
		 */
		int proc_report(void *data, struct timeval *tv = NULL);
};

#endif
