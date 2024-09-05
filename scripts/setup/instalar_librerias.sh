#!/bin/bash

# Instalacion libreria WirinPi
cd $PROJECT_GIT_ROOT/main-libraries
sudo dpkg -i wiringpi-latest.deb

# Instalacion libreria bcm2835
tar zxvf bcm2835-1.58.tar.gz
cd bcm2835-1.58
./configure
make
sudo make check
sudo make install

# Intalacion de jansson
sudo apt-get install libjansson-dev

# Instalacion libreria paho-mqtt
sudo pip3 install paho-mqtt

# Instalacion libreria Google Drive API
sudo pip3 install --upgrade google-api-python-client google-auth-httplib2 google-auth-oauthlib
sudo pip3 install --upgrade oauth2client

# Intalacion de ObsPy
#sudo apt-get install -y python3-pip python3-scipy python3-lxml python3-setuptools python3-sqlalchemy python3-decorator python3-requests python3-packaging python3-pyproj python3-pytest python3-geographiclib python3-cartopy python3-pyshp libatlas-base-dev gfortran
#sudo pip3 install numpy matplotlib --upgrade
#sudo pip3 install obspy