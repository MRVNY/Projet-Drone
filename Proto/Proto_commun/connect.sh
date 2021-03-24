echo "Searching..."
i=1
cmp=""
wifi=$(iw dev | grep Interface | cut -d ' ' -f 2)
nmcli dev wifi rescan
while [ $i -le 5 ]
do
  #ssid=$(iwlist $wifi scan | grep ESSID:\"milos_pc_ | cut -d '"' -f 2)
  ssid=$(iwlist $wifi scan | grep ESSID:\"Bebop2-089802 | cut -d '"' -f 2)
  
  if [ "$ssid" = "" ]
  then 
    sleep 1
    i=$(($i + 1)) 
  else
    echo "Connecting..."
    nmcli device wifi connect $ssid
    i=10
  fi
done

[ $i = 10 ] && echo "Connected to $ssid" || echo "Can't find wifi"