#!/bin/bash

# Copiar el directorio "Configuracion" a la ruta /home/rsa/
cp -r configuracion /home/rsa/

# Crea los directorios necesarios
mkdir -p /home/rsa/operation-scripts
mkdir -p /home/rsa/operation-scripts/mqtt
mkdir -p /home/rsa/operation-scripts/drive
mkdir -p /home/rsa/operation-scripts/acelerografo
mkdir -p /home/rsa/operation-scripts/acelerografo/libraries
mkdir -p /home/rsa/log-files
mkdir -p /home/rsa/tmp
mkdir -p /home/rsa/resultados/eventos-detectados
mkdir -p /home/rsa/resultados/eventos-extraidos
mkdir -p /home/rsa/resultados/registro-continuo
mkdir -p /home/rsa/resultados/mseed

# Crea los archivos necesarios
echo $(date) > /home/rsa/resultados/registro-continuo/nueva-estacion.txt
echo 'nueva-estacion.txt' > /home/rsa/tmp/NombreArchivoRegistroContinuo.tmp

# Compila todos los programas escritos en C 
sh setup-scripts/compilar.sh

# Copia los scripts para mqtt
cp /home/rsa/Acelerografo-RSA/scripts/operation/mqtt/SubirDirectorioDrive*.py /home/rsa/ejecutables/SubirDirectorioDrive.py

# Copia los scripts para drive
cp /home/rsa/Acelerografo-RSA/programas/SubirDirectorioDrive*.py /home/rsa/ejecutables/SubirDirectorioDrive.py
cp /home/rsa/Acelerografo-RSA/programas/subir_archivo_drive*.py /home/rsa/ejecutables/SubirArchivoDrive.py
cp /home/rsa/Acelerografo-RSA/programas/conversor_mseed*.py /home/rsa/ejecutables/ConversorMseed.py
cp /home/rsa/Acelerografo-RSA/programas/LimpiarArchivosRegistro*.py /home/rsa/ejecutables/LimpiarArchivosRegistro.py
cp /home/rsa/Acelerografo-RSA/programas/PublicarEventoMQTT.py /home/rsa/ejecutables/PublicarEventoMQTT.py
cp /home/rsa/Acelerografo-RSA/programas/ExtractorEventosMQTT.py /home/rsa/ejecutables/ExtractorEventosMQTT.py

# Copia los task-scripts al directorio /usr/local/bin
sudo cp task-scripts/ayuda.sh /usr/local/bin/ayuda
sudo cp task-scripts/informacion.sh /usr/local/bin/informacion 
sudo cp task-scripts/comprobar.sh /usr/local/bin/comprobar
sudo cp task-scripts/registrocontinuo.sh /usr/local/bin/registrocontinuo

# Conceder permisos de ejecucion a los task-scripts
sudo cp task-scripts/ayuda.sh /usr/local/bin/ayuda
sudo chmod +x /usr/local/bin/informacion
sudo chmod +x /usr/local/bin/comprobar
sudo chmod +x /usr/local/bin/registrocontinuo