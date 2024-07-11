#!/bin/bash

# Compila todos los programas escritos en C 
sh setup-scripts/compilar.sh

# Copia los scripts de Python a la carpeta /home/rsa/ejecutables
cp /home/rsa/Acelerografo-RSA/programas/SubirDirectorioDrive*.py /home/rsa/ejecutables/SubirDirectorioDrive.py
cp /home/rsa/Acelerografo-RSA/programas/subir_archivo_drive*.py /home/rsa/ejecutables/SubirArchivoDrive.py
cp /home/rsa/Acelerografo-RSA/programas/conversor_mseed*.py /home/rsa/ejecutables/ConversorMseed.py
cp /home/rsa/Acelerografo-RSA/programas/LimpiarArchivosRegistro*.py /home/rsa/ejecutables/LimpiarArchivosRegistro.py
cp /home/rsa/Acelerografo-RSA/programas/PublicarEventoMQTT.py /home/rsa/ejecutables/PublicarEventoMQTT.py
cp /home/rsa/Acelerografo-RSA/programas/ExtractorEventosMQTT.py /home/rsa/ejecutables/ExtractorEventosMQTT.py

# Copia los task-scripts al directorio /usr/local/bin
sudo cp task-scripts/comprobar.sh /usr/local/bin/comprobar
sudo cp task-scripts/informacion.sh /usr/local/bin/informacion 
sudo cp task-scripts/registrocontinuo.sh /usr/local/bin/registrocontinuo