#!/bin/sh
sudo rm -rf qjackctl
sudo apt-get update
sudo apt-get upgrade -y
sudo apt-get install qt5-default qttools5-dev-tools git -y
sudo git clone https://git.code.sf.net/p/qjackctl/code qjackctl-git
cd qjackctl-git
sudo sh autogen.sh
sudo ./configure prefix=/usr
sudo make
sudo make install
