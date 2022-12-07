export readonly DB_PASSWD=showmethemoney
export readonly ROOT_DIR_OF_INSTALLATION=/mnt/storage/

export readonly ROOT_DIR_OF_3RDPARTY=`pwd`/../3rdparty
export readonly ROOT_DIR_OF_SOLUTION=`pwd`/..
export readonly TAG=$(git describe 2>/dev/null || echo v1.0.0-alpha.3 | sed 's/v//g')
