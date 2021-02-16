ssid=$(iwlist wlan0 scan | grep ESSID:\"milos_pc_ | cut -d '"' -f 2)
nmcli device wifi connect $ssid