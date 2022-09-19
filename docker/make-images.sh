set -x
set -u
set -e

. setting.sh

mkdir -p $ROOT_DIR_OF_INSTALLATION/mysql/conf.d \
         $ROOT_DIR_OF_INSTALLATION/mysql/data   \
         $ROOT_DIR_OF_INSTALLATION/mysql/logs
cat /dev/null > $ROOT_DIR_OF_INSTALLATION/mysql/conf.d/my.cnf
cat << EOF    > $ROOT_DIR_OF_INSTALLATION/mysql/conf.d/my.cnf
[mysqld]
event_scheduler=ON
EOF

mkdir -p roudi
cp $ROOT_DIR_OF_3RDPARTY/iceoryx/src/iceoryx-2.0.2/build/iox-roudi ./roudi/

make_inst_pkg() {
  rm -rf $1 && mkdir -p $1
  tar -cvzf $1.tar.gz \
    $(find ../bin | grep -v logs | grep $1 | grep -v '/config/\|/plugin/')
  tar -xvzf $1.tar.gz -C $1
  rm -rf $1.tar.gz
}

readonly BQMD_BINANCE=bqmd-binance
make_inst_pkg $BQMD_BINANCE
mkdir -p $BQMD_BINANCE/lib
cp -r $ROOT_DIR_OF_3RDPARTY/cpr/src/cpr-1.7.2/build/lib/ $BQMD_BINANCE/

readonly BQTD_BINANCE=bqtd-binance
make_inst_pkg $BQTD_BINANCE
mkdir -p $BQTD_BINANCE/lib
cp -r $ROOT_DIR_OF_3RDPARTY/cpr/src/cpr-1.7.2/build/lib/ $BQTD_BINANCE/

readonly BQTD_SRV=bqtd-srv
make_inst_pkg $BQTD_SRV

readonly BQRISK_MGR=bqriskmgr
make_inst_pkg $BQRISK_MGR

readonly BQSTG_ENG_DEMO=bqstgengdemo
make_inst_pkg $BQSTG_ENG_DEMO

mkdir -p $ROOT_DIR_OF_INSTALLATION/betterquant/
cp -r $ROOT_DIR_OF_SOLUTION/bin/config $ROOT_DIR_OF_INSTALLATION/betterquant/
cp -r $ROOT_DIR_OF_SOLUTION/bin/plugin $ROOT_DIR_OF_INSTALLATION/betterquant/
sed -i 's/host=0.0.0.0/host=mysql/g' \
  $(grep 'host=0.0.0.0' $ROOT_DIR_OF_INSTALLATION/betterquant/config -rl | grep yaml)
sed -i "s/password=123456/password=$DB_PASSWD/g" \
  $(grep 'password=123456' $ROOT_DIR_OF_INSTALLATION/betterquant/config -rl | grep yaml)

docker-compose build --no-cache
docker images

rm -rf roudi
rm -rf $BQTD_SRV
rm -rf $BQRISK_MGR
rm -rf $BQMD_BINANCE
rm -rf $BQTD_BINANCE
rm -rf $BQSTG_ENG_DEMO
