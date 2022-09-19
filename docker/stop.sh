docker ps  | awk '{print $1}' | grep -v CONTAINER | grep -v roudi | grep -v mysql | xargs -t -i docker stop {}
docker ps  | awk '{print $1}' | grep -v CONTAINER | xargs -t -i docker stop {}
