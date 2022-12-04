set -xue
bash build-proj.sh

find ../../bin/ -type f | grep bqtd-binance | grep 'yaml\|-d-' | xargs -t -i rm -rf {}
cd ../bqtd-binance/ && bash build-proj.sh && cd -
