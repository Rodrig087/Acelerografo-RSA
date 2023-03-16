#!/bin/bash

# Instalacion libreria bcm2835
cd librerias
tar zxvf bcm2835-1.58.tar.gz
cd bcm2835-1.58
./configure
make
sudo make check
sudo make install

# Instalacion libreria WirinPi
sudo apt-get install wiringpi