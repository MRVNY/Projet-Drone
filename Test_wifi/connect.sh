#!/bin/bash

echo "Searching..."
i=1
cmp=""
while [ $i -le 5 ]
do
  ssid=$(iwlist wlxd0374523a03c scan | grep ESSID:\"milos_pc_ | cut -d '"' -f 2)
  
  if [ "$ssid" = "" ]
  then 
    nmcli dev wifi rescan
    i=$(($i + 1)) 
  else
    echo "Connecting..."
    nmcli device wifi connect $ssid
    i=10
  fi
done

[ $i = 10 ] && echo "Connected to $ssid" || echo "Can't find wifi"