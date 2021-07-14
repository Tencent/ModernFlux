![Image text](https://github.com/cherieyuan/img-folder/blob/master/ModernFlux-Logo2.png)

![Image text](https://img.shields.io/badge/license-BSD3-brightgreen)  ![Image text](https://img.shields.io/badge/release-v1.0.0-brightgreen)
## 1：项目背景
  在手游时代后台系统设计经常会遇到以下场景：
  * 需求层面：秒杀活动瞬间会有巨大流量涌向后台，对系统形成冲击。这些瞬间流量往往是预估峰值流量的7-8倍，乃至10倍以上。
  * 后台设计：后台大量采用微服务设计方案，系统TOPO结构复杂，整个系统安全负载量不易评估。同时系统内各个服务抗压能力不一，有的模块会随着流量的线性增加性能产生断崖式下降，从而引发“雪崩”，有可能造成整个系统瘫痪。
  * 运维层面：各个业务独立部署会造成机器资源利用率低，混和部署有可能各个业务流量之间互相影响

  对于这些场景不能简单通过堆机器来解决，首先事先不太可能申请这么多资源，即使申请了，峰值过后，机器利用率会比较低，形成浪费。所以需要一种负载保护机制，使系统能抗住突然流量冲击，对用户提供柔性服务，同时降低运营成本。

## 2：设计方案
   设计方案需要满足以下一些基本原则
  * 负载保护系统不能明显加重原有系统负载，需做到业务流量正常时对系统没明显负载增加，流量突发时能成功保护系统
  * 支撑异构机器部署，同时方便系统动态扩缩容
  * 流控服务本身对流量变化不敏感，以免流量剧烈变化时，流控服务本身经常扩缩容
  * 既能控制单机流量又能控制整个系统流量
  * 既能控制业务维度流量，又能控制资源维度流量，从而达到各个业务能够安全混用机器，提高资源利用率
  * 上游下游能够有效互动，上游能管理好入口流量，下游能对上游产生“背压”，从而上游能主动限制对下游访问，必要时“熔断”
  * 有比较好容灾能力，当流控服务暂时不能用时，不会对业务造成大的影响
  * 流控系统足够简单，方便运维，不会明显增加系统复杂度

  综上从三个层面设计负载保护系统
  * 通过配额服务实时计算管理入口流量
  * 定时检测单机硬件资源消耗
  * 实时检测下游服务质量

**系统架构图**

![Image text](https://github.com/cherieyuan/img-folder/blob/master/flux1.png)

## 3: 目录简介

### 1.1	cppFcApi
- [流控API C++版本库，API使用方法详见](docs/wiki/API-C%2B%2B.md)

### 3.2	phpFcApi
- [流控API C++版本库，API使用方法详见](docs/wiki/API-php.md)
	
### 3.3	protocal
	Pb协议
	
### 3.4	QuaAgent
	小配额agent,针对于qps不高的活动进行负载均衡。
    环境依赖：Linux x64 环境，建议CentOS 7.x；Golang 1.0及以上
	编译：进入QuaAgent/src/flux,执行 sh huild.sh 即可。执行./qagent 

### 3.5	QuaServer
	配额计算Server,统计计算并分配各节点配额。
	编译：进入QuaServer 执行 make 即可。 
	
## 4: 效果图
![Image text](https://github.com/cherieyuan/img-folder/blob/master/FLux-Example1.png)
 
![Image text](https://github.com/cherieyuan/img-folder/blob/master/FLux-Example2.png)
  
![Image text](https://github.com/cherieyuan/img-folder/blob/master/FLux-Example3.png)

## 5:License
ModernFlux 是基于 BSD3 协议， 详细请参考 [LICENSE](LICENSE.txt)






