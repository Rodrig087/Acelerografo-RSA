#!/bin/bash

# Dependiendo de los parámetros que se le pasen al programa se usa una opción u otra
case "$1" in
  start)
    echo "Arrancando sistema de registro continuo..."
    sudo killall -q acelerografo
    sudo /home/rsa/ejecutables/acelerografo &
    sleep 5
    sudo python3 /home/rsa/ejecutables/SubirRegistroDrive.py &
    sleep 5
    sudo python3 /home/rsa/ejecutables/ConversorMseed.py &
    ;;
  stop)
    echo "Deteniendo sistema de registro continuo..."
    sudo killall -q acelerografo
    sudo /home/rsa/ejecutables/resetmaster
    ;;
  *)
    echo "Modo de uso: registrocontinuo start|stop"
    exit 1
    ;;
esac
 
exit 0 
