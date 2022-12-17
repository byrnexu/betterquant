目录
=================

* [文档](#文档)
   * [策略相关](#-策略相关)
      * [说明](#-说明)
      * [新建策略](#-新建策略)
      * [启动](#-启动)
      * [DEMO](#-demo)
      * [策略接口](#-策略接口)
         * [下单](#下单)
         * [撤单](#撤单)
         * [订阅](#订阅)
         * [取消订阅](#取消订阅)
         * [给子策略安装定时器](#给子策略安装定时器)
         * [根据区间\[tsBegin, tsEnd)查询历史行情](#根据区间tsbegin-tsend查询历史行情)
         * [根据时间点往前获取num条历史行情](#根据记录条数查询历史行情从ts往前获取num条历史行情)
         * [根据时间点往后获取num条历史行情](#根据记录条数查询历史行情从ts往后获取num条历史行情)
         * [子策略运行过程中的一些数据保存](#子策略运行过程中的一些数据保存请用json格式)
         * [加载子策略运行过程中生成的数据](#加载子策略运行过程中生成的数据)
         * [委托回报](#委托回报)
         * [撤单应答](#撤单应答)
         * [逐笔成交](#逐笔成交)
         * [订单簿](#订单簿)
         * [Candle](#Candle)
         * [Tickers](#tickers)
         * [人工干预指令处理](#人工干预指令处理)
         * [策略启动事件](#策略启动事件)
         * [子策略启动事件](#子策略启动事件)
         * [账户层面仓位变动信息](#账户层面仓位变动信息收到的是全量盈亏等数据有变化就收到通知其他层面类似)
         * [账户层面仓位快照](#账户层面仓位快照收到的是全量定时收到通知其他层面类似)
         * [策略层面仓位变动信息](#策略层面仓位变动信息)
         * [策略层面仓位快照](#策略层面仓位快照)
         * [子策略层面仓位变动信息](#子策略层面仓位变动信息)
         * [子策略层面仓位快照](#子策略层面仓位快照)
         * [账户资产变动信息](#账户资产变动信息)
         * [账户资产快照](#账户资产快照)
         * [新增子策略事件](#新增子策略事件)
         * [移除子策略事件](#移除子策略事件)
         * [子策略参数变化事件](#子策略参数变化事件)
         * [子策略定时器触发事件](#子策略定时器触发事件)
      * [算法单接口](#-算法单接口)
   * [web服务](#-web服务)
      * [相关接口](#-相关接口)
         * [人工干预指令](#人工干预指令)
         * [根据区间查询历史行情](#根据区间查询历史行情)
         * [根据时间点往前查num条记录](#根据时间点往前查num条记录)
         * [根据时间点往后查num条记录](#根据时间点往后查num条记录)
   * [行情服务和配置](#-行情服务和配置)
   * [风控插件](#-风控插件)
   * [历史行情回放](#-历史行情回放)
   * [模拟成交](#-模拟成交)
   * [数据库表](#-数据库表)


<br/>  


# 文档

## 📒 策略相关
### 🔥 说明
&emsp;&emsp;策略引擎负责下单、撤单、策略和子策略的订单管理、分发回报到相应的子策略等等，先介绍下策略接口的框架设计，每个策略都有若干个子策略，每个子策略有一套策略参数，策略和子策略是一对一或者一对多的关系，策略信息存放在数据库表stgInfo表中，子策略信息存放在stgInstInfo表中，stgInstInfo表中有每个子策略的策略参数（字段stgInstParams），金工可以在盘中修改此策略参数，修改后 onStgInstChg 事件会触发，开发人员可在此处理策略参数变化事件。出于性能考虑，所有子策略运行在一个线程池中，线程池的大小可配置：
```yaml
stgInstTaskDispatcherParam: moduleName=StgInstTaskDispatcher;taskRandAssignedThreadPoolSize=0;taskSpecificThreadPoolSize=4
```
修改策略配置中的 taskSpecificThreadPoolSize=4 即可调整运行的子策略的线程池的大小。因为子策略运行在线程池中，所以 c++ 语言编写的策略的子策略回调事件中要处理好一些共享数据的竞争问题（比如加锁），对于 python 编写的策略事件回调目前是串行的，所以无需处理竞争。 另外由于会有多个子策略触发回调，因此在处理回调逻辑的时候要先加上条件判断，是哪个子策略的 stgInstId 触发了此事件：
```python
def on_stg_inst_start(self, stg_inst_info):
    if stg_inst_info.stg_inst_id == 1:
        # sub market data of trades, note that the topic is case sensitive.
        self.stg_eng.sub(
            stg_inst_info.stg_inst_id, "shm://MD.Binance.Spot/ADA-USDT/Trades"
        )   
```
在这一点上 c++ 和 python 没什么区别，子策略编号 stgInstId 必须从1开始，每个策略也必须得有一个 stgInstId 为1的子策略。
  
<br/>  

### 🔥 新建策略
&emsp;&emsp;新建策略主要涉及以下几个步骤：
* 修改表 stgInfo ，增加策略信息的记录

| 字段 | 说明 | 备注 |
| ------ | ------ | ------ |
| productId | 产品编号 | 如果一个产品下有多个策略需要统一计算和监控仓位盈亏等信息，那么给他们指定一个相同的 prouductId |
| stgId | 策略编号 | 区间\[10001, 20000)，其他系统预留用于测试账户、国内期现交易或其他市场的账号 |
| stgName | 策略名称 | 给策略取一个有辨识度的名称 |
| stgDesc | 策略描述 | 一些额外的说明 |
| userIdOfAuthor | 策略作者 | 记录策略作者，不影响策略逻辑 |
<br/>  


* 修改表 stgInstInfo ，增加子策略信息的记录

| 字段 | 说明 | 备注 |
| ------ | ------ | ------ |
| stgId | 策略编号 | 区间\[10001, 20000)，其他系统预留用于测试账户、国内期现交易或其他市场的账号 |
| stgInstId | 子策略编号 | 从1开始递增 |
| stgInstParams  | 子策略参数 | json格式 |
| stgInstName | 子策略名称 | 给子策略取一个有辨识度的名称 |
| stgInstDesc | 子策略描述 | 一些额外的说明 |
| userId | 用户Id | 用于统计用户层面的资产、仓位、盈亏 |
<br/>  

&emsp;&emsp;目前没有客户端，请通过SQLyog或者其他 mysql 客户端应用直接改库。  
```sql
INSERT INTO `BetterQuant`.`stgInfo`(`productId`, `stgId`, `stgName`, `stgDesc`, `userIdOfAuthor`) 
  VALUES(1, 10001, 'TestStg', 'Stg used to test', 1);
  
INSERT INTO `BetterQuant`.`stgInstInfo`(`stgId`, `stgInstId`, `stgInstParams`, `stgInstName`, `stgInstDesc`, `userId`) 
  VALUES(10001, 1, '{"symbolCode":"BTC-USDT"}', 'TestStgInst', 'Stg inst used to test.', 1);  
```
&emsp;&emsp;tips:  linux下通过命令行调用sql
```shell
docker exec -i bqdb mysql -uroot -p123456  <<< "use BetterQuant; select * from stgInfo;"
```
&emsp;&emsp;bqdb 是容器名称，123456 是数据库密码。  
<br/>  

* 创建策略配置文件  
从以下模板文件拷贝修改：

| 语言 | 路径 | 备注 |
| ------ | ------ | ------ |
| python | bqstg/bqstgeng-py-demo/config/bqstgeng-py-demo.yaml |  |
| c++ | bqstg/bqstgeng-cxx-demo/config/bqstgeng-cxx-demo.yaml |  |

一般情况下只需修改其中 stgId 配置项即可，当然你也可以根据运行的子策略的数量修改前面提到的运行子策略的线程池的大小：
```yaml
stgId: 10001
```
配置文件名的格式建议：bqstgeng-10001.yaml，也就是文件名中带上策略编号。  
<br/>  

### 🔥 启动
```bash
./bqstgeng-10001 --conf=config/bqstgengdemo/bqstgeng-10001.yaml &
```
  
<br/>  

### 🔥 DEMO

| 语言 | 路径 | 备注 |
| ------ | ------ | ------ |
| python | bqstg/bqstgeng-cxx-demo/ | 开发需要 deploy_stgeng.sh 中打包的头文件和库文件 |
| c++ | bqstg/bqstgeng-py-demo/ | 开发或者运行都需要 deploy_stgeng.sh 中打包的 bqstgeng.so 库 |
| c | bqstg/bqstgeng-c-demo/ | TODO |
<br/>

### 🔥 策略接口
#### 下单
```c++
  std::tuple<int, OrderId> StgEng::order(const StgInstInfoSPtr& stgInstInfo, AcctId acctId,
                                         const std::string& symbolCode, Side side, PosSide posSide,
                                         Decimal orderPrice, Decimal orderSize);
```
<br/>

#### 撤单
```c++
  int StgEng::cancelOrder(OrderId orderId);
```
<br/>

#### 订阅
```c++
  int StgEng::sub(StgInstId subscriber, const std::string& topic);
```
&emsp;&emsp;当前子策略订阅策略id为10000的仓位、未实现盈亏、已实现盈亏、手续费使用情况：
```
  getStgEng()->sub(
      stgInstInfo->stgInstId_,
      "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000");
```
&emsp;&emsp;topic 为 "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000"，注意大小写敏感  
<br/>  

#### 取消订阅
```c++
  int StgEng::unSub(StgInstId subscriber, const std::string& topic);
```
<br/>

#### 给子策略安装定时器
```c++
  void StgEng::installStgInstTimer(StgInstId stgInstId, const std::string& timerName,
                                   ExecAtStartup execAtStartUp, std::uint32_t millicSecInterval,
                                   std::uint64_t maxExecTimes = UINT64_MAX);
```
&emsp;&emsp; 定时器最小间隔为1毫秒，但是后台定时监测任务是1毫秒触发一次，所以如果入参 millicSecInterval 为1的话，定时器触发误差会比较大，理论上实际触发间隔可能会达到2毫秒。  
<br/>

#### 根据区间\[tsBegin, tsEnd)查询历史行情
```c++
   std::tuple<int, std::string> queryHisMDBetween2Ts(
       MarketCode marketCode, SymbolType symbolType,
       const std::string& symbolCode, MDType mdType, std::uint64_t tsBegin,
       std::uint64_t tsEnd, const std::string& ext = "");
 
   std::tuple<int, std::string> queryHisMDBetween2Ts(const std::string& topic,
                                                     std::uint64_t tsBegin,
                                                     std::uint64_t tsEnd);
```                                                     
<br/>
 
#### 根据记录条数查询历史行情，从ts往前获取num条历史行情
```c++
   std::tuple<int, std::string> querySpecificNumOfHisMDBeforeTs(
       MarketCode marketCode, SymbolType symbolType,
       const std::string& symbolCode, MDType mdType, std::uint64_t ts, int num,
       const std::string& ext = "");
 
   std::tuple<int, std::string> querySpecificNumOfHisMDBeforeTs(
       const std::string& topic, std::uint64_t ts, int num);
```                                                     
<br/>
 
#### 根据记录条数查询历史行情，从ts往后获取num条历史行情
```c++
   std::tuple<int, std::string> querySpecificNumOfHisMDAfterTs(
       MarketCode marketCode, SymbolType symbolType,
       const std::string& symbolCode, MDType mdType, std::uint64_t ts, int num,
       const std::string& ext = "");
 
   std::tuple<int, std::string> querySpecificNumOfHisMDAfterTs(
       const std::string& topic, std::uint64_t ts, int num);
```       
<br/>
 
#### 子策略运行过程中的一些数据保存，请用json格式  
策略除了启动参数之外，另外在策略的运行过程中可能会产生一些中间数据需要保存下来，可以使用以下接口。
```c++
   bool StgEng::saveStgPrivateData(StgInstId stgInstId, const std::string& jsonStr);
```
&emsp;&emsp;策略运行中会产生一些中间数据，有时候我们需要将这些数据需要保存下来，那么可以调用上面的接口保存数据，如果策略重启，需要这些保存的数据，那么可以调用下面的 loadStgPrivateData 重新加载，保存这些中间数据是一个同步的过程，为了尽可能小的影响策略主体逻辑，配置文件中将其保存到tmpfs系统，也就是共享内存中，所以重启后会消失，当然你也可以修改配置，将其永久保存到硬盘中。
```yaml
rootDirOfStgPrivateData: /dev/shm
```
<br/>

#### 加载子策略运行过程中生成的数据
```c++
   bool StgEng::loadStgPrivateData(StgInstId stgInstId);
```
<br/>

#### 委托回报
```c++
   virtual void StgInstTaskHandlerBase::onOrderRet(const StgInstInfoSPtr& stgInstInfo,
                                                   const OrderInfoSPtr& orderInfo) {}
```
<br/>

#### 撤单应答
```c++
   virtual void StgInstTaskHandlerBase::onCancelOrderRet(const StgInstInfoSPtr& stgInstInfo,
                                                         const OrderInfoSPtr& orderInfo) {}
```
<br/>

#### 逐笔成交
```c++
   virtual void StgInstTaskHandlerBase::onTrades(const StgInstInfoSPtr& stgInstInfo,
                                                 const TradesSPtr& trades) {}
```
<br/>
 
#### 订单簿
```c++
   virtual void StgInstTaskHandlerBase::onBooks(const StgInstInfoSPtr& stgInstInfo,
                                                const BooksSPtr& books) {}
```
<br/>
 
#### Candle
```c++
   virtual void StgInstTaskHandlerBase::onCandle(const StgInstInfoSPtr& stgInstInfo,
                                                 const CandleSPtr& candle) {}
```
<br/>
 
#### Tickers
```c++
   virtual void StgInstTaskHandlerBase::onTickers(const StgInstInfoSPtr& stgInstInfo,
                                                  const TickersSPtr& tickers) {}
```
<br/>

#### 人工干预指令处理
```c++
   virtual void onStgManualIntervention(const StgInstInfoSPtr& stgInstInfo,
                                        const CommonIPCDataSPtr& commonIPCData) {
   }
```
此消息在 **[人工干预指令](#人工干预指令)** 中发起，在这里触发。  
<br/>

#### 策略启动事件
```c++
   virtual void StgInstTaskHandlerBase::onStgStart() {}
```
&emsp;&emsp;策略启动的时候触发，这个事件会 dispatch 给 stgInstId 为 1 的子策略。  
<br/>

#### 子策略启动事件
```c++
   virtual void StgInstTaskHandlerBase::onStgInstStart(const StgInstInfoSPtr& stgInstInfo) {}
```
&emsp;&emsp;子策略启动的时候触发，每个子策略启动的时候都会接收到这个事件。  
<br/>

#### 账户层面仓位变动信息（收到的是全量，盈亏等数据有变化就收到通知，其他层面类似）
```c++
   // 需要先 sub(stgInstInfo.stgInstId_, topic = "shm://RISK.PubChannel.Trade/PosInfo/AcctId/10001");
   virtual void StgInstTaskHandlerBase::onPosUpdateOfAcctId(const StgInstInfoSPtr& stgInstInfo,
                                                            const PosSnapshotSPtr& posSnapshot) {}
```
&emsp;&emsp;订阅了账户层面的仓位变动信息，那么账户的仓位，盈亏一有变化，就会收到这个事件，下面策略层面、子策略层面都类似。  
<br/>

#### 账户层面仓位快照（收到的是全量，定时收到通知，其他层面类似）
```c++
   // 需要先 sub(stgInstInfo.stgInstId_, topic = "shm://RISK.PubChannel.Trade/PosInfo/AcctId/10001");
   virtual void StgInstTaskHandlerBase::onPosSnapshotOfAcctId(const StgInstInfoSPtr& stgInstInfo,
                                                              const PosSnapshotSPtr& posSnapshot) {}
```
&emsp;&emsp;订阅了账户层面的仓位变动信息，就会定时触发这个事件，下面策略层面、子策略层面都类似。  
<br/>

#### 策略层面仓位变动信息
```c++
   // 需要先 sub(stgInstInfo.stgInstId_, "shm://RISK.PubChannel.Trade/PosInfo/StgId/10001")
   virtual void StgInstTaskHandlerBase::onPosUpdateOfStgId(const StgInstInfoSPtr& stgInstInfo,
                                   const PosSnapshotSPtr& posSnapshot) {}
```
<br/>

#### 策略层面仓位快照
```c++
   // 需要先 sub(stgInstInfo.stgInstId_, "shm://RISK.PubChannel.Trade/PosInfo/StgId/10001")
   virtual void StgInstTaskHandlerBase::onPosSnapshotOfStgId(const StgInstInfoSPtr& stgInstInfo,
                                                             const PosSnapshotSPtr& posSnapshot) {}
```
<br/>

#### 子策略层面仓位变动信息
```c++
   // 需要先 sub(stgInstInfo.stgInstId_, "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000/StgInstId/1")
   virtual void StgInstTaskHandlerBase::onPosUpdateOfStgInstId(const StgInstInfoSPtr& stgInstInfo,
                                                               const PosSnapshotSPtr& posSnapshot) {}
```
<br/>

#### 子策略层面仓位快照
```c++
   // 需要先 sub(stgInstInfo.stgInstId_, "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000/StgInstId/1")
   virtual void StgInstTaskHandlerBase::onPosSnapshotOfStgInstId(const StgInstInfoSPtr& stgInstInfo,
                                                                 const PosSnapshotSPtr& posSnapshot) {}
```
<br/>

#### 账户资产变动信息
```c++
   // 需要先 sub(stgInstInfo.stgInstId_, "shm://RISK.PubChannel.Trade/AssetInfo/AcctId/10001")
   virtual void StgInstTaskHandlerBase::onAssetsUpdate(const StgInstInfoSPtr& stgInstInfo,
                                                       const AssetsUpdateSPtr& assetsUpdate) {}
``` 
&emsp;&emsp;订阅了账户层面的资产变动信息，账户资产一有变化就会触发此事件。  
<br/>

#### 账户资产快照
```c++
   // 需要先 sub(stgInstInfo.stgInstId_, "shm://RISK.PubChannel.Trade/AssetInfo/AcctId/10001")
   virtual void StgInstTaskHandlerBase::onAssetsSnapshot(const StgInstInfoSPtr& stgInstInfo,
                                                         const AssetsSnapshotSPtr& assetsSnapshot) {}
```
&emsp;&emsp;订阅了账户层面的资产变动信息，就会定时触发此事件。  
<br/>

#### 新增子策略事件
```c++
   virtual void StgInstTaskHandlerBase::onStgInstAdd(const StgInstInfoSPtr& stgInstInfo) {}
```
&emsp;&emsp;新增一个子策略的时候，此事件会触发。  
<br/>

#### 移除子策略事件
```c++
   virtual void StgInstTaskHandlerBase::onStgInstDel(const StgInstInfoSPtr& stgInstInfo) {}
```
&emsp;&emsp;移除一个子策略的时候，此事件会触发。  
<br/>

#### 子策略参数变化事件
```c++
   virtual void StgInstTaskHandlerBase::onStgInstChg(const StgInstInfoSPtr& stgInstInfo) {}
```
&emsp;&emsp;修改一个子策略参数的时候，此事件会触发。  
<br/>

#### 子策略定时器触发事件
```c++
   virtual void StgInstTaskHandlerBase::onStgInstTimer(const StgInstInfoSPtr& stgInstInfo, const std::string& timerName) {}
```
&emsp;&emsp;给子策略安装一个定时器的时候，此事件会触发。  
<br/>

### 🔥 算法单接口
TODO  
<br/>

## 📒 web服务
### 🔥 相关接口  
#### 人工干预指令
* POST /v1/manualIntervention

| 名称 | 类型 | 描述 |
| ------ | ------ | ------ |
| stgId | INT | 策略编号 |
| stgInstId | INT | 子策略编号 |

body中传输JSON格式数据。 

在 **[人工干预指令处理](#人工干预指令处理)** 收到此消息编写代码处理。

<br/>

#### 根据区间查询历史行情  
* GET /v1/QueryHisMD/between/Binance/Spot/BTC-USDT/Trades  
* GET /v1/QueryHisMD/between/Binance/Spot/BTC-USDT/Books  
* GET /v1/QueryHisMD/between/Binance/Spot/BTC-USDT/Candle  
* GET /v1/QueryHisMD/between/Binance/Spot/BTC-USDT/Tickers  

| 名称 | 类型 | 描述 |
| ------ | ------ | ------ |
| tsBegin | INT64 | 查询区间起点 |
| tsEnd | INT64 | 查询区间终点 |
| level | INT | 订单簿档数（仅对Books生效） |
| detail | BOOL | 是否查询Candle区间内的所有数据（仅对Candle生效） |

&emsp;&emsp;
<br/>

#### 根据时间点往前查num条记录  
* GET /v1/QueryHisMD/before/Binance/Spot/BTC-USDT/Trades  
* GET /v1/QueryHisMD/before/Binance/Spot/BTC-USDT/Books  
* GET /v1/QueryHisMD/before/Binance/Spot/BTC-USDT/Candle  
* GET /v1/QueryHisMD/before/Binance/Spot/BTC-USDT/Tickers  

| 名称 | 类型 | 描述 |
| ------ | ------ | ------ |
| ts | INT64 | 查询区间起点 |
| num | INT | 查询记录数 |
| level | INT | 订单簿档数（仅对Books生效） |
| detail | BOOL | 是否查询Candle区间内的所有数据（仅对Candle生效） |

&emsp;&emsp;
<br/>

#### 根据时间点往后查num条记录  
* GET /v1/QueryHisMD/after/Binance/Spot/BTC-USDT/Trades  
* GET /v1/QueryHisMD/after/Binance/Spot/BTC-USDT/Books  
* GET /v1/QueryHisMD/after/Binance/Spot/BTC-USDT/Candle  
* GET /v1/QueryHisMD/after/Binance/Spot/BTC-USDT/Tickers  

| 名称 | 类型 | 描述 |
| ------ | ------ | ------ |
| ts | INT64 | 查询区间起点 |
| num | INT | 查询记录数 |
| level | INT | 订单簿档数（仅对Books生效） |
| detail | BOOL | 是否查询Candle区间内的所有数据（仅对Candle生效） |

&emsp;&emsp;
<br/>

## 📒 行情服务和配置
* 🔥 配置  

1. 以币安为例，行情服务的配置在 bqmd-binance/config 下

| 目录 | 说明 | 备注 |
| ------ | ------ | ------ |
| bqmd-binance/spot | 现货行情配置 |  |
| bqmd-binance/futures | U本位交割合约行情配置 |  |
| bqmd-binance/perp | U本位永续合约行情配置 |  |
| bqmd-binance/cfutures | 币本位交割合约行情配置 |  |
| bqmd-binance/cperp | 币本位永续合约行情配置 |  |

注意：以现货为例，行情必须在 bqmd-binance/spot/TopicGroupMustSubInAdvance.yaml 配置，否则即使策略端调用了sub，也收不到行情，这个配置可以在运行期修改，无需重启。
```yaml
# 注意大小写敏感
topicGroup:
  - BTC-USDT@Trades     # 成交明细
  - BTC-USDT@Tickers    # tickers
  - BTC-USDT@Candle     # k线
  - BTC-USDT@Books@400  # 订单簿（目前只支持400档）
```
如果没有这个配置，即使策略订阅了BTC-USDT成交明细，也收不到行情。  
<br/>

## 📒 风控插件
* 🔥 风控插件安装、启用和禁用：
1. 每个插件都包含一个动态链接库和配置文件：如 libbqtd-srv-risk-plugin-flow-ctrl-0.so 和 libbqtd-srv-risk-plugin-flow-ctrl-0.yaml
1. 插件名称中的数字代表风控规则检查顺序，不需要连续，最大值为 MAX_TD_SRV_RISK_PLUGIN_NUM = 32，这是个常量，可以修改并重新编译，建议不要超过128。
1. 安装新的风控插件只需将动态链接库和配置文件拷贝到 plugin 目录下（这个目录可在配置中修改）。
1. 启用风控插件只需将配置文件里的 enable 改成 true 即可。
1. 禁用风控插件只需将配置文件里的 enable 改成 false 即可，最多5秒后该插件就会被关闭，5秒这个时间可以在配置文件里配置。
1. 升级风控插件需要先禁用该插件，日志提示该插件已经被 unload 之后后覆盖动态链接库。再启用该插件。
1. 当风控插件配置文件发生变化的时候，系统会自动重新加载该插件，所以非必要不要修改生产环境里的配置文件。
<br/>

## 📒 历史行情回放
* 启动命令  
```bash
./bqmd-sim --conf=config/bqmd-sim/bqmd-sim.yaml
```
配置文件中可以指定回放速度、回放的topic等信息。配置中的enable设定为true才能启动行情回放。
<br/>

## 📒 模拟成交
* 启动命令  
```bash
./bqtd-binance --conf=config/bqtd-binance/spot/bqtd-binance.yaml
```
和启动交易网关的命令一样，注意配置中的simedMode项下的enable必须设定为true才能进入模拟成交模式。
<br/>
<br/>


## 📒 数据库表
系统本身是全内存交易，数据库只是一个存放一些基本信息和交易流水的地方。

| 表名 | 说明 | 备注 |
| ------ | ------ | ------ |
| 📰 acctInfo | 账户信息 | 存放内部账户编号 acctId 和交易所交易账号相关信息 |
| 📰 assetInfo | 资产信息 | 存放最新的资产信息 |
| 📰 externalStatusCode | 外部状态码 | 存放系统内部状态码和交易所状态码之间的映射关系 |
| 📰 hisAssetInfo | 历史资产信息 | 每分钟 assetInfo 会自动备份到 hisAssetInfo |
| 📰 hisPnl | 历史pnl信息 | PosSnapshot::saveToDB 会插入 pnl 到这个表 |
| 📰 hisPosInfo | 历史仓位信息 | 历史仓位变动信息，结合历史行情可用于计算历史pnl变动情况。 |
| 📰 orderInfo | 订单流水 | 包含每个订单的详细信息 |
| 📰 posInfo | 仓位信息 | 仓位的详细信息 |
| 📰 productInfo | 产品信息 | 创建产品信息用于产品层面的资金、仓位和订单维护 |
| 📰 stgInfo | 策略信息 | 保存策略的基本信息 |
| 📰 stgInstInfo | 子策略信息 | 子策略包括子策略参数在内的基本信息 |
| 📰 symbolInfo | 代码表 | 行情服务会自动维护这张表 |
| 📰 tradeInfo | 成交流水 | 成交流水表主要用于灾备恢复 |
| 📰 trdSymbol | 成交代码表 | 所有交易过的代码都会存放在这张表里 |

注：表和字段名之所以是驼峰的而不是传统的snake风格，是因为交易系统的c++代码采用驼峰命名法，内部有一个将表转换为json的反射(通过metadata实现的并非真正意义上的反射)，不想去处理json中每个字段名的转换，因此就用了驼峰。


