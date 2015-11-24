#bin/bash

#vde_switch -d -s /tmp/switch1

sudo vde_tunctl -u opossum
sudo ifconfig tap0 192.168.0.254 up
sudo route add -host 192.168.0.10 dev tap0

#vde_plug2tap --daemon -s /tmp/switch1 tap0
