pid_list=$(ps -ef|grep -i './bq' | grep -v grep | awk '{print $2}' | xargs | sed 's/ /,/g')
if [[ ! -z $pid_list ]]; then
  top -p $(pid_list)
fi
