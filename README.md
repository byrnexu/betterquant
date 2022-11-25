[**中文版**](README_cn.md)

[![github](https://img.shields.io/badge/github-byrnexu-brightgreen.svg)](https://github.com/byrnexu)  

[<img src="./assets/logo-big.png" width="180" />]()

**Better quant today, best quant tomorrow.** 💪  

Welcome to fork/star, your support is the greatest motivation for this project to get better and better.

## Overview
&emsp;&emsp;**I'm looking for job opportunities recently, if you need a technical director/partner or architect for a quantitative hedge fund in Shanghai, China, please email: 28645861@qqcom. Of course, if you support telecommuting, It doesn't matter where the job is.**   

&emsp;&emsp;The main functions and features of betterquant include:<br/>
* 🔥 This is a quantitative trading system designed to support multiple accounts, multiple strategies, multiple products, and multiple hosts in parallel, which can be horizontally expanded to support hundreds of billions of quantitative hedge fund.<br/>
&nbsp;
* 🔥 It supports C++ and Python to write trading strategies. A few shell commands can complete the installation and deployment of the entire system.<br/>
&nbsp;
* 🔥 In the current design, only the exchange needs to provide two interfaces: order status push and order information query (in fact, as long as the field information is perfect for the two interfaces is enough), the system can help you accurately calculate accounts, strategies, sub strategies, etc. The level of pnl and fee information, the recovery of various information after the system crashes, ignores various types of exchanges or counter interfaces, only these two interfaces are required.<br/>
&nbsp;
* 🔥  The supported trading products include spot, currency-based and USD-based perpetual contracts, currency-based and USD-based delivery contracts, as well as position management and profit and loss monitoring functions for one-way mode and hedge mode positions of the above contracts.<br/>
&nbsp;
* 🔥🔥 Powerful disaster recovery function, any subsystem crash will not lead to the final data exception‼ ️. If the trading service crashes, positions and PNL information will be rebuilt after restarting. After the trading gateway crashes and restarts, it will automatically process the status changes of unprocessed orders generated during the crash. After the risk control subsystem crashes and restarts, various risk control indicators will also be restored. The recovery process described above does not require cancel any open orders.<br/>
&nbsp;
* 🔥 All subsystems, including market data subsystem, strategy engine, trade service, trade gateway, and risk control subsystem interact through shared memory with no lock❕. Using shared memory as the ipc between subsystems enables the system to have both single-process performance and multi-process security, that is, any system crash will not cause other subsystems to crash. Of course, although the subsystems currently interact through shared memory, this is not a stand-alone trading system❕. In the future, a web service will be set up to provide restful and websocket interfaces to accept remote orders other business functions.<br/>
&nbsp;
* 🔥 Each subsystem has its own independent PUB_CHANNEL. You can publish TOPIC to your own PUB_CHANNEL. For example, the market data subsystem can publish the online of new contracts, the changes of contract parameters, and the risk control subsystem can customize its own risk control indicators and publish them to the PUB_CHANNEL of the risk control system. Each subsystem can subscribe and publish the topics they are interested in through the unified format of the system‼ ️.<br/>
&nbsp;
* 🔥 Independent position and order management functions at all levels such as account, strategy, sub-strategy, and user. <br/>
&nbsp;
* 🔥 After many systems receive orders, they will perform risk control processing for the orders. This step has become the bottleneck of these systems. By default, the system divides the received orders according to the account dimension, realizing parallel risk control processing with no lock. Make this link unimpeded✈️, of course, you can also slightly modify the code or add configuration items later, and achieve more fine-grained dimension request shunting according to the configuration specified parameters.<br/>
&nbsp;
* 🔥🔥 Plug-in risk control indicator management🔌, you can write risk control indicators in the form of dynamic link libraries according to the unified format of the system. You can enable, disable or upgrade these risk control plugins during the operation of the trading system‼ ️, so as to realize the dynamic management of risk control indicators and meet the needs of 7\*24-hour trading. The open api interface allows you to get any data you want in the risk control indicator interface.
Use these data to customize flexible and diversified risk control indicators based on your own needs.<br/>
&nbsp;
* 🔥🔥 Powerful pnl real-time monitoring function‼ ️, you can subscribe and monitor the realized profit and loss, unrealized profit and loss of each dimension such as account, strategy, sub-strategy, user, market, target, long position, short position and so on in any subsystem in real time, usage of fees. In other words, the realized profit and loss, unrealized profit and loss, and fee usage denominated in any currency with any combination of dimensions‼ ️, for example: </br>  
&emsp;&emsp;🌶️ To subscribe and monitor the pnl and fees of BTC-denominated ETH perpetual contracts with strategy number 10000 on Binance:
```c++
    // sub
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000");
    // on callback                     
    const auto queryCond = "stgId=10000&marketCode=Binance&symbolCode=ETH-USD-CPerp";
    const auto [statusCode, pnl] =
        posSnapshot->queryPnl(queryCond, QuoteCurrencyForCalc("BTC"));
```
&emsp;&emsp;&emsp;&emsp;🌶️ To subscribe and monitor pair ETH-BTC of account 10000 in Binance trading, pnl and fee denominated in BUSD:
```c++
    // sub
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://RISK.PubChannel.Trade/PosInfo/AcctId/10000");
    // on callback                     
    const auto queryCond = "stgId=10000&marketCode=Binance&symbolCode=ETH-BTC";
    const auto [statusCode, pnl] =
        posSnapshot->queryPnl(queryCond, QuoteCurrencyForCalc("BUSD"));
```

&emsp;&emsp;You can also monitor multiple strategy combinations at the same time, and calculate the realized profit and loss, unrealized profit and loss, and usage of fees of the strategy combination. Through these profit and loss data, you can even write a monitoring strategy for monitoring other strategies❕, triggering risk control Then send various flexible functions such as intervention instructions to other strategies. Of course, after adding the algorithmic trading function in the future, you can also track the profit and loss of an algorithmic order (including multiple sub orders) in real time. <br/>
&nbsp;
* 🔥 Dynamic maintenance of status codes from exchange. Since the trading system may be connected to multiple exchanges, each exchange has its own specific status code, and unknown external status will also be received during the execution of the strategy. After receiving these external states, they will be automatically included, and you can map them to the specified internal state codes during system in running, so that the policy side can correctly process the business logic of the new state codes. The whole process does not need to restart any subsystems❕.<br/>
&nbsp;
* 🔥 The symbol code table dynamic maintenance function, the system regularly detects the changes of the exchange code table, in addition to the new code online and offline, as well as some of its largest and smallest transactions and the change unit of the amount, etc., if there is a change, the local code table is updated immediately.<br/>
&nbsp;
* 🔥 Each strategy can specify sub-strategy with several sets of parameters. The sub-strategy set is managed by a thread pool. The parameters of the sub-policy can be modified and automatically updated during system in running, and sub-strategy can also be added or disabled during operation❕. This is especially important for some markets that trade on 7*24 hours.<br/>

---
## Table of contents
### 🛠 [BUILD](doc/build.md)
### 🐋 [INSTALL](doc/installation.md)
### ⭐ [DOCUMENTATION](doc/documentation.md)
### 🧨 [WARNING](doc/caution.md)
### ⁉️ [FAQ](doc/faq.md)
### 🥔 [TODO](doc/todo.md)

---
<div align="center"> <img  src="https://github-readme-streak-stats.herokuapp.com?user=byrnexu&theme=onedark&date_format=M%20j%5B%2C%20Y%5D" /> </div>

