# 1、说明

​		该Demo为配合配额计算Server使用的客户端流量上报示例。QuaServer编译成功后，可使用该demo模拟服务调用方持续上报流量并观测相应配额的限流效果。



# 2、环境要求

Go 1.12及以上版本，本项目依赖go mod进行包管理



# 3、使用方式

### 下载依赖

~~~
go mod tidy
~~~

### 编译

```
go build
```

### 提交依赖

```
go mod verify
```



Client端Demo与流控Server端的架构关系如下图所示：

![Image text](https://github.com/Xie-Jianxin/img-folder/Flux_Demo.png)

