cppclean \
  --include-path ./pub/inc/ \
  --include-path ./bqpub/inc/ \
  --include-path ./bqmd/bqmd-pub/inc/ \
  --include-path ./bqmd/bqmd-svc-base/inc/ \
  --include-path ./bqmd/bqmd-binance/inc/ \
  $(find . -type f | grep '\.hpp\|\.cpp' | grep -v 3rdparty | grep -v '/build/\|\./inc$')
