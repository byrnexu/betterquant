. setting.sh

if [[ -z $(docker network ls | grep '\<bqnet\>') ]]; then
  docker network create bqnet
fi

docker-compose -f docker-compose.yaml \
  --project-directory $ROOT_DIR_OF_3RDPARTY/iceoryx/src/iceoryx-2.0.2 up \
  --no-recreate --timeout 600 -d
