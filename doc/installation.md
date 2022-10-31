# INSTALL
## 🔥 Execute the first step of compilation first

&emsp;&emsp;The results of the compilation are in the bin and lib directories. The ones ending with -d are the debug versions, and the others are the release versions.

## 🔥 Make images  

&emsp;&emsp;Please modify DB_PASSWD and ROOT_DIR_OF_INSTALLATION in docker/setting.sh, which are the docker mount directories for configuration, plugins, logs, and databases. In particular, the database password DB_PASSWD must be remembered to be modified.
```bash
   cd docker && bash make-images.sh
```
&emsp;&emsp;Use command docker images. You can see the created images.

## 🔥 Run

* run base services  

&emsp;&emsp;Here will start market data, trading, risk control, database and IPC services
```bash
   bash run-base.sh
```
&emsp;&emsp;Use command docker ps. You can see several services started.  

* Create a trading account  

&emsp;&emsp;In order to reduce the complexity of the system, a corresponding transaction gateway needs to be started for each account created. Therefore, creating a trading account includes the following steps:  

1. A record needs to be added to the table acctInfo.  
1. The configuration file of the transaction gateway corresponding to the new account needs to be created.  
1. Modify docker-compose-tdgw.yaml to add a new transaction gateway startup project.  

&emsp;&emsp;For convenience, I wrote a script create-acct.sh in the docker directory, run create-acct.sh to complete the above three steps.  
```bash
   bash create-acct.sh acctId marketCode symbolType acctName apiKey secKey phase
```
&emsp;&emsp;For a more detailed example, please refer to: docker/create-acct-for-test.sh  

| parameter | meaning | remark |
| ------ | ------ | ------ |
| acctId | acctId that needs to be created | interval \[10001,20000) |
| marketCode | market code | eg: Binance (Case Sensitive) |
| symbolType | symbol type | eg: Spot、Perp、CPerp、Futures、CFutures (Case sensitive, capital C starts with the currency standard) |
| acctName | account name | Give the account a recognizable name |
| apiKey | api key | Please bind IP to close transfer💣❗ |
| secKey | security key | Please bind IP to close transfer💣❗ |
| phase | phase | If not exists, just pass ""💣❗ |

&emsp;&emsp;Note: Regarding acctId, the account range used by users is defined between \[10001, 20000), and other ranges are reserved for test accounts and other markets.  
```bash
   bash create-acct.sh 10001 Binance Spot BinanceSpotTest apikey seckey ""  
```
&emsp;&emsp;Note: If acctId already exists, the original information will be overwritten.  

* Run the trading gateway (the database must have the corresponding account configuration and add the corresponding item in docker-compose-tdgw.yaml, see below)
```bash
   bash run-tdgw.sh
```
&emsp;&emsp;Use command docker ps. You can see the trading gateway startup result.

* stop services
```bash
   bash stop.sh
```

## 🔥 run (without docker)

&emsp;&emsp;If the compilation is successful, you can also directly run the compiled target file without using docker. This method can be considered for both development and testing. The database and IPC services are installed and started first, and there is no sequence requirement for others.
* Install and start the database
```shell
cd bqdb && bash run-db.sh & 
```
* Start IPC service
```shell
iox-roudi &
```
* Run the following services
```shell
# Running the market data service of Binance Spot Service
./bqmd-binance --conf=config/bqmd-binance/spot/bqmd-binance.yaml &

# Running the market data service of Binance usd based futures Service
./bqmd-binance --conf=config/bqmd-binance/futures/bqmd-binance.yaml &

# Running the market data service of Binance usd based perp Service
./bqmd-binance --conf=config/bqmd-binance/perp/bqmd-binance.yaml &

# Running the market data service of Binance currency based futures Service
./bqmd-binance --conf=config/bqmd-binance/cfutures/bqmd-binance.yaml &

# Running the market data service of Binance currency based perp Service
./bqmd-binance --conf=config/bqmd-binance/cperp/bqmd-binance.yaml &

# Running the risk mgr service
./bqriskmgr --conf=config/bqriskmgr/bqriskmgr.yaml

# Running the trading service
./bqtd-srv --conf=config/bqtd-srv/bqtd-srv.yaml

# Running the trading gateway (One trading gateway per account)  
./bqtd-binance --conf=config/bqtd-binance/spot/bqtd-binance-10001.yaml &

```
* Stop services
```shell
# First get the pid through the following command, taking the spot market gateway as an example:
ps -ef|grep -i bqmd|grep spot'

# Then stop the service by kill -SIGINT pid or kill -SIGTERM pid
# Remember not to use kill -SIGKILL or kill -9
```

* Notice‼️  
**apiKey, please bind the IP and turn off the transfer function, and set a database password of your own, remember💣❗**
