kill -2 $(ps -ef|grep -i './bq' | grep -v grep | awk '{print $2}')
