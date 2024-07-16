# Registro de Cambios

## programas/conversor_mseed.py [2.0.0] - 2024-06-27
### Cambios
- Se cambió la lectura de parametros mseed de un archivo csv a un archivo json.
- Se cambió la lectura de parametros del dispositivo de un archivo txt a un archivo json.
- Se realizó una revision completa del programa y se otimizaron varias funciones.
- Se actualizó el script de ayuda para incluir intrucciones para ejecutar este programa.
- Se actualizó el script registro continuo para corregir el nuevo formato del nombre del programa.

## programas/conversor_mseed.py [2.1.0] - 2024-07-05
### Optimización
- Se optimizó la función leer_archivo_binario() para realizar operaciones vectorizadas en lugar de iterativas.
- Esta optimización redujo el tiempo de conversión del archivo binario a formato mseed de 30 minutos a 30 segundos.
- Se realizaron comparaciones entre la matriz numpy extraída y el archivo mseed obtenido con la versión original para garantizar que la versión optimizada del programa produce los mismos resultados que la versión original.
- También se verificó utilizando un archivo binario incompleto del cual se eliminaron 100 muestras de manera aleatoria.

## programas/subir_archivo_drive.py [2.0.0] - 2024-07-11
### Cambios 
- Se depuró el código de la funcion para que se adapte al estandar establecido por el resto de programas.
- Se cambió la lectura de parametros del dispositivo de un archivo txt a un archivo json.
- Se cambió la estructura del nombre de los archivos de Registro Continuo y Evento Extraido almacenados en los archivos temporales NombreArchivoRegistroContinuo.tmp y NombreArchivoEventoExtraido.tmp para que no incluyan el path completo del archivo, sino unicamente el nombre y la extension. 
- Se agregó un parametro mas de entrada que indique si se debe borrar el archivo despues de subirlo a Drive.
- Se cambió el orden de los parametros de entrada quedando de la siguiente manera: subir_archivo_drive.py <nombre_archivo> <tipo_archivo> <borrar_despues>

## programas/extraer_evento_binario.c [2.1.1] - 2024-07-11
### Patch
- Se realizó un cambio en el código para que la estructura del nombre del archivo extraido coincida con los cambios implementados en el programa subir_archivo_drive.py

## programas/conversor_mseed.py [2.1.1] - 2024-07-11
### Patch
- Se realizó un cambio en el código para que la estructura del nombre de los archivos coincidan con los cambios implementados en el programa subir_archivo_drive.py

## configuracion/configuracion_dispositivo.json | setup-scripts/actualizar.sh | setup-scripts/iniciar.sh | setup-scripts/compilar.sh - 2024-07-11
### Patch
- Se realizaron correcciones para que los nombres de las variables y los programas coincidan con los nuevos formatos