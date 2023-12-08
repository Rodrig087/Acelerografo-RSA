#!/bin/bash

# Compila las librerias
cp /home/rsa/Acelerografo-RSA/programas/librerias/eventos.h /home/rsa/librerias/
gcc -c -o /home/rsa/librerias/eventos.o /home/rsa/Acelerografo-RSA/programas/librerias/eventos.c 
gcc -shared -o /home/rsa/librerias/libeventos.so /home/rsa/librerias/eventos.o

# Compila todos los programas de la carpeta Programas 
gcc -o /home/rsa/ejecutables/acelerografo  /home/rsa/Acelerografo-RSA/programas/RegistroContinuo_*.c -I/home/rsa/librerias -L/home/rsa/librerias -leventos -Wl,-rpath,/home/rsa/librerias -lbcm2835 -lwiringPi -lm
gcc programas/ComprobarRegistro_*.c -o /home/rsa/ejecutables/comprobarregistro
gcc programas/ExtraerEventoBin_*.c -o /home/rsa/ejecutables/extraerevento
gcc /home/rsa/Acelerografo-RSA/programas/ResetMaster.c -o /home/rsa/ejecutables/resetmaster -lbcm2835 -lwiringPi 

#