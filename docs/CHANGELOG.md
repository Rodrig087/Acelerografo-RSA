# Registro de Cambios

## programas/conversor_mseed.py [2.0.0] - 2024-06-27
### Cambios
- Se cambio la lectura de parametros mseed de un archivo csv a un archivo json
- Se cambio la lectura de parametros del dispositivo de un archivo txt a un archivo json
- Se realizo una revision completa del programa y se otimizaron varias funciones
- Se actualizo el script de ayuda para incluir intrucciones para ejecutar este programa
- Se actualizo el script registro continuo para corregir el nuevo formato del nombre del programa

## programas/conversor_mseed.py [2.1.0] - 2024-07-05
### Optimizacion
- Se optimizó la función leer_archivo_binario() para realizar operaciones vectorizadas en lugar de iterativas.
- Esta optimización redujo el tiempo de conversión del archivo binario a formato mseed de 30 minutos a 30 segundos.
- Se realizaron comparaciones entre la matriz numpy extraída y el archivo mseed obtenido con la versión original para garantizar que la versión optimizada del programa produce los mismos resultados que la versión original.
- También se verificó utilizando un archivo binario incompleto del cual se eliminaron 100 muestras de manera aleatoria.