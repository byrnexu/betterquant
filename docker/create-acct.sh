#===========================================================================
# usage:
#
# bash create-acct.sh acctId marketCode symbolType acctName apiKey secKey ""
#
# eg:
# bash create-acct.sh 10001 Binance Spot BinanceSpotTest apikey seckey ""
#
#===========================================================================

. setting.sh

if [[ $# -ne 7 ]]; then
  echo 'usage: bash create-acct.sh acctId marketCode symbolType acctName apiKey secKey ""'
  echo ""
  echo ===========================================================================
  echo  eg:
  echo  bash create-acct.sh 10001 Binance Spot     BinanceSpotTest     apikey seckey phase
  echo  bash create-acct.sh 10002 Binance Futures  BinanceFuturesTest  apikey seckey phase
  echo  bash create-acct.sh 10003 Binance Perp     BinanceCPerpTest    apikey seckey phase
  echo  bash create-acct.sh 10004 Binance CFutures BinanceCFuturesTest apikey seckey phase
  echo  bash create-acct.sh 10005 Binance CPerp    BinanceCPerpTest    apikey seckey phase
  echo ===========================================================================
  echo ""
  exit
fi

acct_id=$1
market_code=$2
symbol_type=$3
acct_name=$4
api_key=$5
sec_key=$6
phase=$7

# replace acct info to database
acct_data="$api_key;$sec_key;$phase"
sql="REPLACE INTO BetterQuant.acctInfo(acctId, marketCode, symbolType, acctName, acctData) \
  VALUES($acct_id, \"$market_code\", \"$symbol_type\", \"$acct_name\", \"$acct_data\");"
docker exec -i bqdb mysql -uroot -p$DB_PASSWD <<< $sql

readonly market_code_lower_case=$(echo $market_code | tr A-Z a-z)
readonly symbol_type_lower_case=$(echo $symbol_type | tr A-Z a-z)

conf_template_of_tdgw="$ROOT_DIR_OF_SOLUTION/bqtd/bqtd-$market_code_lower_case/"\
"config/$symbol_type_lower_case/bqtd-$market_code_lower_case.yaml"

#
# create trade gateway config of new acct id
#
tdgw_conf_of_new_acct_id="$ROOT_DIR_OF_SOLUTION/bqtd/bqtd-$market_code_lower_case"\
"/config/$symbol_type_lower_case/bqtd-$market_code_lower_case-$acct_id.yaml"
cp $conf_template_of_tdgw $tdgw_conf_of_new_acct_id
sed -i "s/acctId: .*/acctId: $acct_id/g" $tdgw_conf_of_new_acct_id
sed -i "s/bqtd-$market_code_lower_case-$symbol_type_lower_case/"\
"bqtd-$market_code_lower_case-$symbol_type_lower_case-$acct_id/g" \
  $tdgw_conf_of_new_acct_id

#
# copy trade gateway config of new acct id to directory bin/config
#
dir_of_tdgw_conf_of_new_acct_id="$ROOT_DIR_OF_SOLUTION/bin/config/"\
"bqtd-$market_code_lower_case/config/$symbol_type_lower_case"
mkdir -p $dir_of_tdgw_conf_of_new_acct_id
cp $tdgw_conf_of_new_acct_id \
   $tdgw_conf_of_new_acct_id_dir/bqtd-$market_code_lower_case-$acct_id.yaml

#
# copy trade gateway config of new acct id to directory of docker setting
#
if [[ -d "$ROOT_DIR_OF_INSTALLATION/betterquant/config/" ]]; then
  cp $tdgw_conf_of_new_acct_id "$ROOT_DIR_OF_INSTALLATION/betterquant/config/"\
"bqtd-$market_code_lower_case/$symbol_type_lower_case/"\
"bqtd-$market_code_lower_case-$acct_id.yaml"
fi

#
# create docker-compose setting of current acct id
#
sed "s/market_code/$market_code_lower_case/g" docker-compose-tdgw-template.yaml > \
  docker-compose-tdgw/docker-compose-tdgw-$acct_id.yaml
sed -i "s/symbol_type/$symbol_type_lower_case/g" \
  docker-compose-tdgw/docker-compose-tdgw-$acct_id.yaml
sed -i "s/acct_id/$acct_id/g" docker-compose-tdgw/docker-compose-tdgw-$acct_id.yaml

#
# merge docker-compose into one file
#
cat docker-compose-tdgw-base.yaml > docker-compose-tdgw.yaml
for file in `find docker-compose-tdgw -type f | grep -v README | sort`; do
  cat $file >> docker-compose-tdgw.yaml
done

#
# Modify the host and password of mysql in the configuration required by docker
#
sed -i 's/host=0.0.0.0/host=mysql/g' \
  $(grep 'host=0.0.0.0' $ROOT_DIR_OF_INSTALLATION/betterquant/config -rl | grep yaml)
sed -i "s/password=.*/password=$DB_PASSWD/g" \
  $(grep 'password=.*' $ROOT_DIR_OF_INSTALLATION/betterquant/config -rl | grep yaml)

#===========================================================================
# eg:
# bash create-acct.sh 10001 Binance Spot     BinanceSpotTest     apikey seckey phase
# bash create-acct.sh 10002 Binance Futures  BinanceFuturesTest  apikey seckey phase
# bash create-acct.sh 10003 Binance Perp     BinanceCPerpTest    apikey seckey phase
# bash create-acct.sh 10004 Binance CFutures BinanceCFuturesTest apikey seckey phase
# bash create-acct.sh 10005 Binance CPerp    BinanceCPerpTest    apikey seckey phase
#===========================================================================
