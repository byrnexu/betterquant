apt-get update
apt install -y cmake
apt install -y clang-format
apt install -y libmysqlclient-dev
apt install -y gcc g++ cmake libacl1-dev libncurses5-dev pkg-config
apt install -y libssl-dev
apt install -y curl
apt install -y docker.io
curl -L https://get.daocloud.io/docker/compose/releases/download/v2.10.2/docker-compose-`uname -s`-`uname -m` > /usr/local/bin/docker-compose
chmod +x /usr/local/bin/docker-compose
