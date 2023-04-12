#!/bin/bash

# Compila todos los programas de la carpeta Programas 
gcc programas/RegistroContinuo_V35.c -o /home/rsa/ejecutables/acelerografo -lbcm2835 -lwiringPi -lm
gcc programas/ComprobarRegistro_V3.c -o /home/rsa/ejecutables/comprobarregistro
gcc programas/ExtraerEventoBin_V2.c -o /home/rsa/ejecutables/extraerevento
gcc /home/rsa/Acelerografo-RSA/programas/ResetMaster.c -o /home/rsa/ejecutables/resetmaster -lbcm2835 -lwiringPi 