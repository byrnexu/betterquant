read -p "This will clear all images and containers, are you sure (y/n)? " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
  docker ps -a  | awk '{print $1}' | grep -v CONTAINER | xargs -t -i docker stop {}
  docker ps -a  | awk '{print $1}' | grep -v CONTAINER | xargs -t -i docker rm   {}
  docker images | awk '{print $3}' | grep -v IMAGE     | xargs -t -i docker rmi  {}
fi
