. setting.sh

bash clear.sh && \
  rm -rf $ROOT_DIR_OF_INSTALLATION/betterquant/ && \
  rm -rf $ROOT_DIR_OF_INSTALLATION/bin/ &&

cp -r $ROOT_DIR_OF_INSTALLATION/mysql \
  $ROOT_DIR_OF_INSTALLATION/mysql-$(date +"%Y%m%d-%H%M%S")
rm -rf $ROOT_DIR_OF_INSTALLATION/mysql

bash make-images.sh && bash run-base.sh
