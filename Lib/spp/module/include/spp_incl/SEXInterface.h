/**
 * @file SEXInterface.h (SEX = SppEXtra)
 * @brief 异步模式下的连接数上限管理
 * @author aoxu, aoxu@tencent.com
 * @version 1.0
 * @date 2011-10-10
 */
#ifndef __SEX_INTERFACE_H__
#define __SEX_INTERFACE_H__

BEGIN_ASYNCFRAME_NS

/**
 * @brief 设置一组ip/port的连接数上限
 *
 * @param ip 后端服务器ip
 * @param port 后端服务器port
 * @param limit 连接数上限（如果为0，则使用默认值3）
 */
void InitConnNumLimit(const std::string& ip, const unsigned short port, const unsigned limit);

//L5根据一组modid/cmdid来管理连接数上限，对应的每组ip/port都是最多limit个连接
/**
 * @brief 设置一组modid/cmdid的连接数上限
 *
 * @param modid 不可为0或-1
 * @param cmdid 仅当使用有状态路由时填-1
 * @param limit 连接数上限（如果为0，则使用默认值3）
 */
void InitConnNumLimit(const int modid, const int cmdid, const unsigned limit);

// 取得当前正在处理的CActionInfo*指针
void*       GetCurrentActionInfo();

// 设置与取得后端请求序列号Seq
void        SetCurrentSeq(uint64_t seq);
uint64_t    GetCurrentSeq();

// 获取当前进程组号
int GetGroupId();

//获取worker数
int GetGroupsSum();

END_ASYNCFRAME_NS
#endif

