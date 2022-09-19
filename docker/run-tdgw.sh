. setting.sh

docker-compose -f docker-compose-tdgw.yaml \
  --project-directory $ROOT_DIR_OF_3RDPARTY/iceoryx/src/iceoryx-2.0.2 up \
  --no-recreate --timeout 600 -d
