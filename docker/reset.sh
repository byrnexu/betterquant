. setting.sh

if [[ ! -z $(docker images | grep bq) ]]; then
  read -p "This will clear all images and containers, are you sure (y/n)? " -n 1 -r
  echo

  if [[ $REPLY =~ ^[Yy]$ ]]; then
    docker ps -a  | awk '{print $1}' | grep -v CONTAINER | xargs -t -i docker stop {}
    docker ps -a  | awk '{print $1}' | grep -v CONTAINER | xargs -t -i docker rm   {}
    docker images | awk '{print $3}' | grep -v IMAGE     | xargs -t -i docker rmi  {}

    rm -rf $ROOT_DIR_OF_INSTALLATION/betterquant/ && \
    rm -rf $ROOT_DIR_OF_INSTALLATION/bin/ &&

    cp -r $ROOT_DIR_OF_INSTALLATION/mysql \
      $ROOT_DIR_OF_INSTALLATION/mysql-$(date +"%Y%m%d-%H%M%S")
    rm -rf $ROOT_DIR_OF_INSTALLATION/mysql

    bash make-images.sh && bash run-base.sh
  fi
else
  rm -rf $ROOT_DIR_OF_INSTALLATION/betterquant/ && \
  rm -rf $ROOT_DIR_OF_INSTALLATION/bin/ &&

  cp -r $ROOT_DIR_OF_INSTALLATION/mysql \
    $ROOT_DIR_OF_INSTALLATION/mysql-$(date +"%Y%m%d-%H%M%S")
  rm -rf $ROOT_DIR_OF_INSTALLATION/mysql

  bash make-images.sh && bash run-base.sh
fi
