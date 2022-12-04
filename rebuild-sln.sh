set -x
set -u
set -e

st=$(echo "`date +%s.%N`" | bc)

cd bin/ && ls | grep -v data | xargs -t -i rm -rf {} && cd -
rm -rf lib

. setting.sh

sed -i "s/SOLUTION_ROOT_DIR=.*/SOLUTION_ROOT_DIR=${SOLUTION_ROOT_DIR//\//\\/}/g" \
 $(find . -type f | grep 'build-proj.sh')

mkdir -p inc/cxx/
bash rsync-inc-of-stgeng.sh

cd pub                && bash build-proj.sh && cd -
cd bqipc              && bash build-proj.sh && cd -
cd bqweb              && bash build-proj.sh && cd -
cd bqpub              && bash build-proj.sh && cd -
cd bqmd/bqmd-pub      && bash build-proj.sh && cd -
cd bqmd/bqmd-svc-base && bash build-proj.sh && cd -
cd bqmd/bqmd-binance  && bash build-proj.sh && cd -
cd bqmd/bqmd-sim      && bash build-proj.sh && cd -
cd bqordmgr           && bash build-proj.sh && cd -
cd bqassetsmgr        && bash build-proj.sh && cd -
cd bqposmgr           && bash build-proj.sh && cd -
cd bqriskmgr          && bash build-proj.sh && cd -
cd bqstg/bqstgengimpl && bash build-proj.sh && cd -
cd bqstg/bqstgeng-cxx && bash build-proj.sh && cd -
cd bqstg/bqstgeng-cxx && bash build-proj.sh && cd -

cd bqstg/bqstgeng-cxx && bash build-proj.sh && cd -
cd bqstg/bqstgeng-cxx-demo && bash build-proj.sh && cd -

cd bqstg/bqstgeng-py  && bash build-proj.sh && cd -
cd bqstg/bqstgeng-py-demo  && bash build-proj.sh && cd -

cd bqtd/bqtd-srv-risk-plugin/           && bash build-proj.sh && cd -
cd bqtd/bqtd-srv-risk-plugin-flow-ctrl/ && bash build-proj.sh && cd -

cd bqtd/bqtd-srv      && bash build-proj.sh && cd -
cd bqtd/bqtd-pub      && bash build-proj.sh && cd -
cd bqtd/bqtd-svc-base && bash build-proj.sh && cd -
cd bqtd/bqtd-binance  && bash build-proj.sh && cd -

cd bqweb-srv && bash build-proj.sh && cd -

et=$(echo "`date +%s.%N`" | bc)
diff=$(echo "$et-$st" | bc)
echo $(date "+%Y-%m-%d %H:%M:%S") "End execution command $0, time-consuming： $diff" >> time-consuming.log
