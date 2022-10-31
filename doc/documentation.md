# Documentation

## 📒 About the strategy
### 🔥 Overview
&emsp;&emsp;The strategy engine is responsible for add orders, canceling orders, managing orders for strategies and sub strategies, distributing returns to the corresponding sub strategies, etc. First, the framework design of the strategy interface is introduced. Each strategy has several sub strategies, and each sub strategy has a set of strategies Parameters, strategies and sub-strategies are in a one-to-one or one-to-many relationship. The strategy information is stored in the database table stgInfo table, and the sub-strategy information is stored in the stgInstInfo table. The stgInstInfo table has strategy parameters for each sub-strategy (field stgInstParams), strategy developer can modify this strategy parameter in the intraday. After the modification, the onStgInstChg event will be triggered, and the developer can handle the strategy parameter change event here. For performance reasons, all sub-strategies run in a thread pool, and the size of the thread pool can be configured:
```yaml
stgInstTaskDispatcherParam: moduleName=StgInstTaskDispatcher;taskRandAssignedThreadPoolSize=0;taskSpecificThreadPoolSize=4
```
Modify the taskSpecificThreadPoolSize=4 in the strategy configuration to adjust the size of the thread pool of the running sub-strategy. Because the sub-strategy runs in the thread pool, the sub-strategy callback event of the strategy written in C++ language needs to deal with some shared data contention issues (such as locking). For the strategy event callback written in python, it is currently serial, so No need to deal with competition. In addition, since there will be multiple sub-strategies triggering the callback, when processing the callback logic, a conditional judgment must be added first, which sub-strategy's stgInstId triggers this event:
```python
def on_stg_inst_start(self, stg_inst_info):
    if stg_inst_info.stg_inst_id == 1:
        # sub market data of trades, note that the topic is case sensitive.
        self.stg_eng.sub(
            stg_inst_info.stg_inst_id, "shm://MD.Binance.Spot/ADA-USDT/Trades"
        )   
```
At this point, there is no difference between c++ and python. The sub-strategy number stgInstId must start from 1, and each strategy must also have a sub-strategy with stgInstId of 1.  
<br/>  

### 🔥 Create strategy
&emsp;&emsp;Creating a new strategy mainly involves the following steps:
* Modify the table stgInfo to add a record of strategy information

| Field | Detailed description | Remark |
| ------ | ------ | ------ |
| productId | product id | If there are multiple strategies under a product that need to calculate and monitor position profit and loss in a unified manner, assign them the same prouductId |
| stgId | strategy id | interval\[10001, 20000), other systems reserve account numbers for test accounts or other markets |
| stgName | strategy name | Give the strategy a recognizable name |
| stgDesc | strategy description | some additional notes |
| userIdOfAuthor | strategy author | Record strategy author, currently does not affect strategy logic |


* Modify the table stgInstInfo to add a record of sub-strategy information

| Field | Detailed description | Remark |
| ------ | ------ | ------ |
| stgId | Strategy id | interval\[10001, 20000), other systems reserve account numbers for test accounts or other markets |
| stgInstId | Sub strategy id | Increment from 1 |
| stgInstParams  | Sub strategy parameters | json format |
| stgInstName | Sub strategy name | Give the sub-strategy a recognizable name |
| stgInstDesc | Sub strategy description | some additional notes |
| userId | user id | Used to count assets, positions, profit and loss at the user level |

&emsp;&emsp;There is currently no client, please change the database directly through SQLyog or other mysql client applications.  
```sql
INSERT INTO `BetterQuant`.`stgInfo`(`productId`, `stgId`, `stgName`, `stgDesc`, `userIdOfAuthor`) 
  VALUES(1, 10001, 'TestStg', 'Stg used to test', 1);
  
INSERT INTO `BetterQuant`.`stgInstInfo`(`stgId`, `stgInstId`, `stgInstParams`, `stgInstName`, `stgInstDesc`, `userId`) 
  VALUES(10001, 1, '{"symbolCode":"BTC-USDT"}', 'TestStgInst', 'Stg inst used to test.', 1);  
```
&emsp;&emsp;tips:  Call sql through command line under linux
```shell
docker exec -i bqdb mysql -uroot -p123456  <<< "use BetterQuant; select * from stgInfo;"
```
&emsp;&emsp;bqdb is the container name and 123456 is the database password.
<br/>  

* Create a config file of strategy   
Copy the modifications from the following template files:

| Language | File | Remark |
| ------ | ------ | ------ |
| python | bqstg/bqstgeng-py-demo/config/bqstgeng-py-demo.yaml |  |
| c++ | bqstg/bqstgeng-cxx-demo/config/bqstgeng-cxx-demo.yaml |  |

In general, you only need to modify the stgId configuration item. Of course, you can also modify the size of the thread pool that runs the sub-strategy mentioned above according to the number of running sub-strategy:
```yaml
stgId: 10001
```
The format of the configuration file name is recommended: bqstgeng-10001.yaml, that is, the strategy number is included in the file name.  
<br/>  

### 🔥 Start up startegy
```bash
./bqstgeng-10001 --conf=config/bqstgengdemo/bqstgeng-10001.yaml &
```
  
<br/>  

### 🔥 DEMO

| Language | Path | Remark |
| ------ | ------ | ------ |
| python | bqstg/bqstgeng-cxx-demo/ | Development requires the header files and library files packaged in deploy_stgeng.sh |
| c++ | bqstg/bqstgeng-py-demo/ | The bqstgeng.so library packaged in deploy_stgeng.sh is required for development or operation |
| c | bqstg/bqstgeng-c-demo/ | TODO |
<br/>

### 🔥 Interface of strategy
* Add order
```c++
  std::tuple<int, OrderId> StgEng::order(const StgInstInfoSPtr& stgInstInfo, AcctId acctId,
                                         const std::string& symbolCode, Side side, PosSide posSide,
                                         Decimal orderPrice, Decimal orderSize);
```
<br/>

* Cancel order
```c++
  int StgEng::cancelOrder(OrderId orderId);
```
<br/>

* Subscribe topic
```c++
  int StgEng::sub(StgInstId subscriber, const std::string& topic);
```
&emsp;&emsp;The current sub-strategy subscription strategy id is 10000 positions, unrealized profit and loss, realized profit and loss, and fee usage:
```
  getStgEng()->sub(
      stgInstInfo->stgInstId_,
      "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000");
```
&emsp;&emsp;topic is "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000"，Be aware of case sensitivity  
<br/>  

* UnSubscribe
```c++
  int StgEng::unSub(StgInstId subscriber, const std::string& topic);
```
<br/>

* Install timers for sub-strategy
```c++
  void StgEng::installStgInstTimer(StgInstId stgInstId, const std::string& timerName,
                                   ExecAtStartup execAtStartUp, std::uint32_t millicSecInterval,
                                   std::uint64_t maxExecTimes = UINT64_MAX);
```
&emsp;&emsp; The minimum interval of the timer is 1 millisecond, but the background timing monitoring task is triggered once every 1 millisecond, so if the input parameter milliicSecInterval is 1, the timer trigger error will be relatively large. In theory, the actual trigger interval may reach 2 milliseconds.
<br/>

* Save some data during the operation of the sub-strategy, please use json format  
In addition to the startup parameters of the strategy, some intermediate data may be generated during the operation of the strategy and need to be saved. The following interfaces can be used.  
```c++
   bool StgEng::saveStgPrivateData(StgInstId stgInstId, const std::string& jsonStr);
```
&emsp;&emsp;Some intermediate data will be generated during the operation of the strategy. Sometimes we need to save these data, so we can call the above interface to save the data. If the strategy restarts and the saved data is needed, we can call the following loadStgPrivateData to reload and save the data. The intermediate data is a synchronization process. In order to affect the main logic of the strategy as little as possible, the configuration file will save it to the tmpfs system, that is, the shared memory, so it will disappear after restarting. Of course, you can also modify the configuration to make it permanent. Save to hard disk.
```yaml
rootDirOfStgPrivateData: /dev/shm
```
<br/>

* Load data generated during sub-strategy runs
```c++
   bool StgEng::loadStgPrivateData(StgInstId stgInstId);
```
<br/>

* Result of order info
```c++
   virtual void StgInstTaskHandlerBase::onOrderRet(const StgInstInfoSPtr& stgInstInfo,
                                                   const OrderInfoSPtr& orderInfo) {}
```
<br/>

* Result of cancel order
```c++
   virtual void StgInstTaskHandlerBase::onCancelOrderRet(const StgInstInfoSPtr& stgInstInfo,
                                                         const OrderInfoSPtr& orderInfo) {}
```
<br/>

* Market data of trades
```c++
   virtual void StgInstTaskHandlerBase::onTrades(const StgInstInfoSPtr& stgInstInfo,
                                                 const TradesSPtr& trades) {}
```
<br/>
 
* Market data of books
```c++
   virtual void StgInstTaskHandlerBase::onBooks(const StgInstInfoSPtr& stgInstInfo,
                                                const BooksSPtr& books) {}
```
<br/>
 
* Market data of candle
```c++
   virtual void StgInstTaskHandlerBase::onCandle(const StgInstInfoSPtr& stgInstInfo,
                                                 const CandleSPtr& candle) {}
```
<br/>
 
* Market data of tickers
```c++
   virtual void StgInstTaskHandlerBase::onTickers(const StgInstInfoSPtr& stgInstInfo,
                                                  const TickersSPtr& tickers) {}
```
<br/>

* Strategy start event
```c++
   virtual void StgInstTaskHandlerBase::onStgStart() {}
```
&emsp;&emsp;Triggered when the strategy is started, this event will dispatch to the sub-strategy whose stgInstId is 1.  
<br/>

* Sub-strategy start event
```c++
   virtual void StgInstTaskHandlerBase::onStgInstStart(const StgInstInfoSPtr& stgInstInfo) {}
```
&emsp;&emsp;Triggered when the sub-strategy is started, and each sub-strategy will receive this event when it is started.  
<br/>

* Account-level position change information (received the full amount, and received a notification when there is a change in data such as profit and loss, and other levels are similar)
```c++
   // need sub(stgInstInfo.stgInstId_, topic = "shm://RISK.PubChannel.Trade/PosInfo/AcctId/10001");
   virtual void StgInstTaskHandlerBase::onPosUpdateOfAcctId(const StgInstInfoSPtr& stgInstInfo,
                                                            const PosSnapshotSPtr& posSnapshot) {}
```
&emsp;&emsp;After subscribing to the position change information at the account level, then the account's position, profit and loss will be changed, and this event will be received. The following strategy level and sub-strategy level are similar.  
<br/>

* Account-level position snapshot (received the full amount, received notifications regularly, and other levels are similar)
```c++
   // need sub(stgInstInfo.stgInstId_, topic = "shm://RISK.PubChannel.Trade/PosInfo/AcctId/10001");
   virtual void StgInstTaskHandlerBase::onPosSnapshotOfAcctId(const StgInstInfoSPtr& stgInstInfo,
                                                              const PosSnapshotSPtr& posSnapshot) {}
```
&emsp;&emsp;After subscribing to account-level position change information, this event will be triggered regularly. The following strategy level and sub-strategy level are similar.  
<br/>

* Strategy level position change information
```c++
   // need sub(stgInstInfo.stgInstId_, "shm://RISK.PubChannel.Trade/PosInfo/StgId/10001")
   virtual void StgInstTaskHandlerBase::onPosUpdateOfStgId(const StgInstInfoSPtr& stgInstInfo,
                                   const PosSnapshotSPtr& posSnapshot) {}
```
<br/>

* Strategy level position snapshot
```c++
   // need sub(stgInstInfo.stgInstId_, "shm://RISK.PubChannel.Trade/PosInfo/StgId/10001")
   virtual void StgInstTaskHandlerBase::onPosSnapshotOfStgId(const StgInstInfoSPtr& stgInstInfo,
                                                             const PosSnapshotSPtr& posSnapshot) {}
```
<br/>

* Sub-strategy level position change information
```c++
   // need sub(stgInstInfo.stgInstId_, "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000/StgInstId/1")
   virtual void StgInstTaskHandlerBase::onPosUpdateOfStgInstId(const StgInstInfoSPtr& stgInstInfo,
                                                               const PosSnapshotSPtr& posSnapshot) {}
```
<br/>

* Sub-strategy level position snapshot
```c++
   // need sub(stgInstInfo.stgInstId_, "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000/StgInstId/1")
   virtual void StgInstTaskHandlerBase::onPosSnapshotOfStgInstId(const StgInstInfoSPtr& stgInstInfo,
                                                                 const PosSnapshotSPtr& posSnapshot) {}
```
<br/>

* Account Asset Change Information
```c++
   // need sub(stgInstInfo.stgInstId_, "shm://RISK.PubChannel.Trade/AssetInfo/AcctId/10001")
   virtual void StgInstTaskHandlerBase::onAssetsUpdate(const StgInstInfoSPtr& stgInstInfo,
                                                       const AssetsUpdateSPtr& assetsUpdate) {}
``` 
&emsp;&emsp;Subscribing to the asset change information at the account level, this event will be triggered whenever the account assets change.  
<br/>

* Account Asset Snapshot
```c++
   // need sub(stgInstInfo.stgInstId_, "shm://RISK.PubChannel.Trade/AssetInfo/AcctId/10001")
   virtual void StgInstTaskHandlerBase::onAssetsSnapshot(const StgInstInfoSPtr& stgInstInfo,
                                                         const AssetsSnapshotSPtr& assetsSnapshot) {}
```
&emsp;&emsp;This event will be triggered regularly if you subscribe to the asset change information at the account level.  
<br/>

* Add sub-strategy event
```c++
   virtual void StgInstTaskHandlerBase::onStgInstAdd(const StgInstInfoSPtr& stgInstInfo) {}
```
&emsp;&emsp;This event is fired when a sub-strategy is added.  
<br/>

* Remove sub-strategy event
```c++
   virtual void StgInstTaskHandlerBase::onStgInstDel(const StgInstInfoSPtr& stgInstInfo) {}
```
&emsp;&emsp;This event fires when a sub-strategy is removed.  
<br/>

* Sub-strategy parameter change event
```c++
   virtual void StgInstTaskHandlerBase::onStgInstChg(const StgInstInfoSPtr& stgInstInfo) {}
```
&emsp;&emsp;This event is fired when a sub-strategy parameter is modified.  
<br/>

* Sub-strategy timer trigger event
```c++
   virtual void StgInstTaskHandlerBase::onStgInstTimer(const StgInstInfoSPtr& stgInstInfo) {}
```
&emsp;&emsp;This event fires when a timer is installed on the sub-strategy.  
<br/>

## 📒 Market data service and configuration
* 🔥 Configuration

1. Taking Binance as an example, the configuration of the market service is under bqmd-binance/config

| Path | Detailed description | Remark |
| ------ | ------ | ------ |
| bqmd-binance/spot | Spot market configuration |  |
| bqmd-binance/futures | USD based futures market configuration |  |
| bqmd-binance/perp | USD based perp market configuration |  |
| bqmd-binance/cfutures | Currency based futures market configuration |  |
| bqmd-binance/cperp | Currency based perp market configuration |  |

Note: Take the spot as an example, the market must be configured in bqmd-binance/spot/TopicGroupMustSubInAdvance.yaml, otherwise, even if the strategy side calls sub, the market will not be received. This configuration can be modified during runtime without restarting.
```yaml
# Be aware of case sensitivity
topicGroup:
  - BTC-USDT@Trades     # trades
  - BTC-USDT@Tickers    # tickers
  - BTC-USDT@Candle     # candle
  - BTC-USDT@Books@400  # books（Currently only supports 400 files）
```
Without this configuration, even if the strategy subscribes to the BTC-USDT transaction details, the market will not be received. The reason for this is that when I was working, a colleague subscribed to many varieties and did not cancel the subscription, which would affect the performance of the system. had a certain impact.  
<br/>

## 📒 Risk control plug-in
* 🔥 Install, enable and disable risk control plugins:
1. Each plugin contains a dynamic link library and configuration files: such as libbqtd-srv-risk-plugin-flow-ctrl-0.so and libbqtd-srv-risk-plugin-flow-ctrl-0.yaml
1. The number in the plug-in name represents the order of checking the risk control rules, which does not need to be continuous. The maximum value is MAX_TD_SRV_RISK_PLUGIN_NUM = 32, which is a constant that can be modified and recompiled. It is recommended not to exceed 128.
1. To install a new risk control plugin, just copy the dynamic link library and configuration files to the plugin directory (this directory is configured in).
1. To enable the risk control plugin, just change the enable in the configuration file to true.
1. To disable the risk control plugin, you only need to change the enable in the configuration file to false. The plugin will be closed after a maximum of 5 seconds, and this time can be configured in the configuration file.
1. To upgrade the risk control plug-in, you need to disable the plug-in first. The log indicates that the plug-in has been unloaded and then overwrites the dynamic link library. Enable the plugin again.
1. When the configuration file of the risk control plug-in changes, the system will automatically reload the plug-in, so do not modify the configuration file in the production environment unless necessary.
<br/>

## 📒 Tables
The system itself is a full-memory transaction, and the database is just a place to store some basic information and transaction flow.

| Table name | Detail description | Remark |
| ------ | ------ | ------ |
| 📰 acctInfo | account information | Store the internal account number acctId and the relevant information of the exchange trading account |
| 📰 assetInfo | asset information | Store the latest asset information |
| 📰 externalStatusCode | external status code | Stores the mapping relationship between the internal status code of the system and the exchange status code |
| 📰 hisAssetInfo | history asset info | assetInfo is automatically backed up to hisAssetInfo every minute |
| 📰 hisPnl | history pnl information | PosSnapshot::saveToDB will insert pnl into this table |
| 📰 orderInfo | order information | Contains details for each order |
| 📰 posInfo | position information | Details of the position |
| 📰 productInfo | product information | Create product information for product-level funding, position and order maintenance |
| 📰 stgInfo | strategy information | Basic information for saving strategries |
| 📰 stgInstInfo | sub-strategy information | Basic information of sub-strategy including sub-strategy parameters |
| 📰 symbolInfo | symbol information | The market service will automatically maintain this table |
| 📰 tradeInfo | trade information | The AC water meter is mainly used for disaster recovery and recovery |
| 📰 trdSymbol | trade symbol | All traded codes will be stored in this table |

Note: The reason why the table and field names are in camel case instead of the traditional snake style is because the C++ code of the trading system adopts the camel case nomenclature, and there is a reflection that converts the table to json (the implementation through metadata is not really meaningful reflection), I don't want to deal with the conversion of each field name in json, so camel case is used.

