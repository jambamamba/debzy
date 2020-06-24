#!/bin/bash -xe
set -xe

#pidof debzy | sudo xargs kill -9
sudo rm -fr /opt/debzy
sudo rm -fr /home/oosman/DebMaker/debzy*
sudo cp debmaker debzy
sudo chown root:root debzy
sudo chmod 4755 debzy
./debzy &
