# 文档

## 🥑 行情子系统
* 💦 行情子系统负责连接交易所/交易柜台，订阅行情并将其pub到自己的PUB_CHANNEL频道。
* 💦 现货行情启动命令：
```bash
./bqmd-binance --conf=config/bqmd-binance/spot/bqmd-binance.yaml
```
* 💦 U本位永续合约行情启动命令：
```bash
./bqmd-binance --conf=config/bqmd-binance/perp/bqmd-binance.yaml
```
* 💦 币本位永续合约行情启动命令：
```bash
./bqmd-binance --conf=config/bqmd-binance/cperp/bqmd-binance.yaml
```
* 💦 U本位交割合约行情启动命令：
```bash
./bqmd-binance --conf=config/bqmd-binance/futures/bqmd-binance.yaml
```
* 💦 币本位交割合约行情启动命令：
```bash
./bqmd-binance --conf=config/bqmd-binance/cfutures/bqmd-binance.yaml
```
<br/>

## 🥑 策略引擎
* 💦 策略引擎负责下单、撤单、策略和子策略的订单管理、分发回报到相应的子策略等等，更多详细的功能可以参考接口文件和demo🥦。
* 💦 启动命令：
```bash
./bqstgengdemo --conf=config/bqstgengdemo/bqstgengdemo.yaml
```
<br/>

### 🥝 主要接口
* 💦 下单
```c++
  OrderInfoSPtr StgEng::order(const StgInstInfoSPtr& stgInstInfo, AcctId acctId,
                              const std::string& symbolCode, Side side, PosSide posSide,
                              Decimal orderPrice, Decimal orderSize);
```
* 💦 撤单
```c++
  OrderInfoSPtr StgEng::cancelOrder(OrderId orderId);
```
* 💦 订阅
```c++
  int StgEng::sub(StgInstId subscriber, const std::string& topic);
```
当前子策略订阅策略id为10000的仓位、未实现盈亏、已实现盈亏、手续费使用情况：
```
     getStgEng()->sub(
         stgInstInfo->stgInstId_,
         "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000");
```
* 💦 取消订阅
```c++
  int StgEng::unSub(StgInstId subscriber, const std::string& topic);
```
* 💦 给子策略安装定时器
```c++
  void StgEng::installStgInstTimer(StgInstId stgInstId, const std::string& timerName,
                                   ExecAtStartup execAtStartUp, std::uint32_t millicSecInterval,
                                   std::uint64_t maxExecTimes = UINT64_MAX);
```
* 💦 子策略运行过程中的一些数据保存，可考虑用json格式  
策略除了启动参数之外，另外在策略的运行过程中可能会产生一些中间数据需要保存下来，可以使用以下接口。
```c++
   bool StgEng::saveStgPrivateData(StgInstId stgInstId, const std::string& jsonStr);
```
* 💦 加载子策略运行过程中生成的数据
```c++
   bool StgEng::loadStgPrivateData(StgInstId stgInstId);
```
* 💦 委托回报
```c++
   virtual void StgInstTaskHandlerBase::onOrderRet(const StgInstInfoSPtr& stgInstInfo,
                                                   const OrderInfoSPtr& orderInfo) {}
```
* 💦 撤单应答
```c++
   virtual void StgInstTaskHandlerBase::onCancelOrderRet(const StgInstInfoSPtr& stgInstInfo,
                                                         const OrderInfoSPtr& orderInfo) {}
```

* 💦 逐笔成交
```c++
   virtual void StgInstTaskHandlerBase::onTrades(const StgInstInfoSPtr& stgInstInfo,
                                                 const TradesSPtr& trades) {}
```
 
* 💦 订单簿
```c++
   virtual void StgInstTaskHandlerBase::onBooks(const StgInstInfoSPtr& stgInstInfo,
                                                const BooksSPtr& books) {}
```
 
* 💦 K线
```c++
   virtual void StgInstTaskHandlerBase::onCandle(const StgInstInfoSPtr& stgInstInfo,
                                                 const CandleSPtr& candle) {}
```
 
* 💦 Tickers
```c++
   virtual void StgInstTaskHandlerBase::onTickers(const StgInstInfoSPtr& stgInstInfo,
                                                  const TickersSPtr& tickers) {}
```

* 💦 策略启动事件
```c++
   virtual void StgInstTaskHandlerBase::onStgStart() {}
```
* 💦 子策略启动事件
```c++
   virtual void StgInstTaskHandlerBase::onStgInstStart(const StgInstInfoSPtr& stgInstInfo) {}
```
* 💦 账户层面仓位变动信息（收到的是全量，盈亏等数据有变化就收到通知，其他层面类似）
```c++
   virtual void StgInstTaskHandlerBase::onPosUpdateOfAcctId(const StgInstInfoSPtr& stgInstInfo,
                                                            const PosSnapshotSPtr& posSnapshot) {}
```
* 💦 账户层面仓位快照（收到的是全量，定时收到通知，其他层面类似）
```c++
   virtual void StgInstTaskHandlerBase::onPosSnapshotOfAcctId(const StgInstInfoSPtr& stgInstInfo,
                                                              const PosSnapshotSPtr& posSnapshot) {}
```
* 💦 策略层面仓位变动信息
```c++
   virtual void StgInstTaskHandlerBase::onPosUpdateOfStgId(const StgInstInfoSPtr& stgInstInfo,
                                   const PosSnapshotSPtr& posSnapshot) {}
```
* 💦 策略层面仓位快照
```c++
   virtual void StgInstTaskHandlerBase::onPosSnapshotOfStgId(const StgInstInfoSPtr& stgInstInfo,
                                                             const PosSnapshotSPtr& posSnapshot) {}
```
* 💦 子策略层面仓位变动信息
```c++
   virtual void StgInstTaskHandlerBase::onPosUpdateOfStgInstId(const StgInstInfoSPtr& stgInstInfo,
                                                               const PosSnapshotSPtr& posSnapshot) {}
```
* 💦 子策略层面仓位快照
```c++
   virtual void StgInstTaskHandlerBase::onPosSnapshotOfStgInstId(const StgInstInfoSPtr& stgInstInfo,
                                                                 const PosSnapshotSPtr& posSnapshot) {}
```
* 💦 账户资产变动信息
```c++
   virtual void StgInstTaskHandlerBase::onAssetsUpdate(const StgInstInfoSPtr& stgInstInfo,
                                                       const AssetsUpdateSPtr& assetsUpdate) {}
``` 
* 💦 账户资产快照
```c++
   virtual void StgInstTaskHandlerBase::onAssetsSnapshot(const StgInstInfoSPtr& stgInstInfo,
                                                         const AssetsSnapshotSPtr& assetsSnapshot) {}
```
* 💦 新增子策略事件
```c++
   virtual void StgInstTaskHandlerBase::onStgInstAdd(const StgInstInfoSPtr& stgInstInfo) {}
```
* 💦 移除子策略事件
```c++
   virtual void StgInstTaskHandlerBase::onStgInstDel(const StgInstInfoSPtr& stgInstInfo) {}
```
* 💦 子策略参数变化事件
```c++
   virtual void StgInstTaskHandlerBase::onStgInstChg(const StgInstInfoSPtr& stgInstInfo) {}
```
* 💦 子策略定时器触发事件
```c++
   virtual void StgInstTaskHandlerBase::onStgInstTimer(const StgInstInfoSPtr& stgInstInfo) {}
```
<br/>


## 🥑 交易服务
* 💦 交易服务接受策略子系统和交易网关的连接，接受策略子系统的报单请求并将其转发至交易网关，同时还负责风控插件的动态维护。
* 💦 启动命令：
```bash
./bqtd-srv --conf=config/bqtd-srv/bqtd-srv.yaml
```
* 🔌 插件安装启用和禁用：
1. 每个插件都包含一个动态链接库和配置文件：如libbqtd-srv-risk-plugin-flow-ctrl-0.so和libbqtd-srv-risk-plugin-flow-ctrl-0.yaml
2. 安装新的风控插件只需将动态链接库和配置文件拷贝到plugin目录下（这个目录在中配置）
3. 禁用风控插件只需将配置文件里的enable改成false即可，最多5秒后该插件就会被关闭，5秒这个时间可以在配置文件里配置。
4. 将禁用的风控插件启用只需将配置文件里的enable从false改成true。
5. 升级风控插件需要先禁用该插件，日志提示该插件已经被unload之后后覆盖动态链接库。再启用该插件。
6. 当风控插件配置文件发生变化的时候，系统会自动重新加载该插件，所以非必要不要修改生产环境里的配置文件。
<br/>

## 🥑 风控子系统
* 💦 风控子系统负责监控所有订单和仓位，根据订单状态变化实时计算已实现盈亏和未实现盈亏（浮动盈亏）并将其pub到自己的PUB_CHANNAL频道。
* 💦 启动命令：
```bash
./bqriskmgr-d --conf=config/bqriskmgr/bqriskmgr.yaml
```
<br/>

## 🥑 交易网关
* 💦 交易网关负责连接交易所/交易柜台，转发报单请求同时接受交易所/交易柜台的回报。
* 💦 现货交易启动命令：
```bash
./bqtd-binance --conf=config/bqtd-binance/spot/bqtd-binance.yaml
```
* 💦 U本位永续合约交易网关启动命令：
```bash
./bqtd-binance --conf=config/bqtd-binance/perp/bqtd-binance.yaml
```
* 💦 币本位永续合约交易网关启动命令：
```bash
./bqtd-binance --conf=config/bqtd-binance/cperp/bqtd-binance.yaml
```
* 💦 U本位交割合约交易网关启动命令：
```bash
./bqtd-binance --conf=config/bqtd-binance/futures/bqtd-binance.yaml
```
* 💦 币本位交割合约交易网关启动命令：
```bash
./bqtd-binance --conf=config/bqtd-binance/cfutures/bqtd-binance.yaml
```
<br/>

## 🥑 仓位管理模块
  &emsp;&emsp;💦负责维护和计算所有仓位的盈亏信息。
<br/>
<br/>

## 🥑 订单管理模块
  &emsp;&emsp;💦负责维护所有未完结的订单信息。
<br/>
<br/>

## 🥑 资产管理模块
  &emsp;&emsp;💦负责维护所有账户的资产信息。
<br/>
<br/>

## 🥑 bqmd-pub模块
  &emsp;&emsp;💦行情子系统用到的公共模块。
<br/>
<br/>

## 🥑 bqtd-pub模块
  &emsp;&emsp;💦交易网关用到的公共模块。
<br/>
<br/>

## 🥑 bqpub模块
  &emsp;&emsp;💦BetterQuant用到的公共模块。
<br/>
<br/>

## 🥑 pub模块
  &emsp;&emsp;💦和项目无关的，可用于其他项目的一些公共函数和模块。
<br/>
<br/>

