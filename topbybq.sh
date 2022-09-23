top -p $(ps -ef|grep -i './bq' | grep -v grep | awk '{print $2}' | xargs | sed 's/ /,/g')
