#!/bin/bash

# Compila todos los programas escritos en C 
sh setup-scripts/compilar.sh

# Copia todos los programas escritos en Python a la carpeta /home/rsa/ejecutables
cp /home/rsa/Acelerografo-RSA/programas/*.py /home/rsa/ejecutables/

# Copia los task-scripts al directorio /usr/local/bin
sudo cp task-scripts/comprobar.sh /usr/local/bin/comprobar
sudo cp task-scripts/informacion.sh /usr/local/bin/informacion 
sudo cp task-scripts/registrocontinuo.sh /usr/local/bin/registrocontinuo