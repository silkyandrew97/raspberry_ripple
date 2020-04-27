#!/bin/sh
sudo rm -rf jack1
sudo apt-get update
sudo apt-get upgrade -y
sudo apt-get install git libtool libdb5.3-dev -y
sudo git clone https://github.com/jackaudio/jack1.git
cd jack1
sudo git submodule init
sudo git submodule update
sudo sh autogen.sh
sudo ./configure prefix=/usr
sudo make
sudo make install
cd /etc/security
if [ -d limits.d ]; then
  cd limits.d
  CONF=audio.conf
else
  CONF=limits.conf
fi
if [ ! -f $CONF ];then
    sudo touch $CONF
fi
rtprio=0
memlock=0
while read -r line
do
  if grep -q "@audio - rtprio 95" "$CONF"; then
    rtprio=1
  fi
  if grep -q "@audio - memlock unlimited" "$CONF"; then
    memlock=1
  fi
done < "$CONF"
sudo chmod -R 777 $CONF
if [ $rtprio -eq 0 ]; then
  sudo echo "@audio - rtprio 95" >> $CONF
fi
if [ $memlock -eq 0 ]; then
  sudo echo "@audio - memlock unlimited" >> $CONF
fi
sudo groupadd @audio
sudo usermod -a -G audio $(whoami)
