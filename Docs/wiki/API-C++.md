## 流控接口说明
1. 上游流量管理API.
```text
int32_t ServiceQua::CheckQuota(string& keyid, int32_t groupid, int32_t totalquota, cl5::QOSREQUEST& aquol5,
                               string referid, const string& serial)
```
- string& keyid: 业务标识ID.
- int32_t groupid: 进程标识ID，保证单机唯一.
- int32_t totalquota：业务总配额.
- cl5::QOSREQUEST& aquol5：访问配额服务L5结构.
- string referid：无历史数据参考数据key,不填则配额生效前配额取平均值.
- const string& serial：请求流水号，用于定位问题，可不填.
- 返回值: 0: 配额 < 100%, 1: 配额 >100%, <120% 2: 配额 > 120%; 3：set 拒绝.

2.本机资源管理API
```text
bool ServiceQua::InitSysload()
```
- 初始化函数，进程初始化调用(spp可在spp_handle_init).
```text
bool ServiceQua::SysOverload(const open_app_desc::Flux *flux)
```
- const open_app_desc::Flux *flux:参数配置结构，设置CPU阈值等配置信息.
- 返回值：true:过载 false:负载正常.

3.下游负载检测API
```text
template<typename T>
static bool CheckQos(const T& keyname, const open_app_desc::Flux *flux=NULL)
```
- const T& keyname:下游服务key,可以是接口ID或其它.
- const open_app_desc::Flux *flux：阈值配置信息，可不填.
- 返回值：true: 正常 false:下游异常，限制访问.

```text
static int ServiceQua::UpdateQos(const T& keyname, int32_t ret, uint64_t ms=0)
```
- const T& keyname：下游服务key,可以是活动ID,PaaS ID，接口ID，以及其它.
- int32_t ret：本次调用下游结果 0：成功 其它:失败(-3, 超时).
- uint64_t ms: 本次调用下游耗时(毫秒).

4.内存资源清理
```text
int ServiceQua::ClearData()
```
- 内存清理函数(spp可以在spp_handle_loop).

5.伪码调用：
- 进程初始化.
- ServiceQua::InitSysload().
- 收到请求.
- ServiceQua::CheckQuota().
- ServiceQua::SysOverload().
- ServiceQua::CheckQos().
- 向下游发请求.
- 收到回复.
- ServiceQua::UpdateQos.
- 周期调用 ServiceQua::ClearData().







