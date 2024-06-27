######################################### ~Librerias~ #################################################
import numpy as np
from obspy import UTCDateTime, read, Trace, Stream
import subprocess
import time
import sys
import json
#######################################################################################################


######################################### ~Funciones~ #################################################
# Lee un archivo de configuración en formato JSON y devuelve su contenido como un diccionario.
def read_fileJSON(nameFile):
    try:
        with open(nameFile, 'r') as f:
            data = json.load(f)
        return data
    except FileNotFoundError:
        print(f"Archivo {nameFile} no encontrado.")
        return None
    except json.JSONDecodeError:
        print(f"Error al decodificar el archivo {nameFile}.")
        return None

# Lee el archivo binario, procesa los datos y devuelve un arreglo de NumPy con los datos y una lista de segundos faltantes.
def leer_archivo_binario(archivo_binario):
    datos = [[], [], []]
    contador = 0
    segundo_anterior = None
    segundos_faltantes = []

    with open(archivo_binario, "rb") as f:
        while True:
            tramaDatos = np.fromfile(f, np.int8, 2506)
            if len(tramaDatos) != 2506:
                break
            
            hora = tramaDatos[2503]
            minuto = tramaDatos[2504]
            segundo = tramaDatos[2505]
            n_segundo = hora * 3600 + minuto * 60 + segundo
            
            if segundo_anterior is not None:
                if n_segundo != segundo_anterior + 1:
                    segundos_faltantes.extend(range(segundo_anterior + 1, n_segundo))
                elif n_segundo > segundo_anterior + 1:
                    # Si hay un salto mayor, se llenan los segundos faltantes intermedios
                    segundos_faltantes.extend(range(segundo_anterior + 1, n_segundo))
            
            segundo_anterior = n_segundo
            
            for j in range(0, 3):
                for i in range(0, 250):
                    dato_1 = tramaDatos[i * 10 + j * 3 + 1]
                    dato_2 = tramaDatos[i * 10 + j * 3 + 2]
                    dato_3 = tramaDatos[i * 10 + j * 3 + 3]
                    xValue = ((dato_1 << 12) & 0xFF000) + ((dato_2 << 4) & 0xFF0) + ((dato_3 >> 4) & 0xF)
                    if (xValue >= 0x80000):
                        xValue = xValue & 0x7FFFF
                        xValue = -1 * (((~xValue) + 1) & 0x7FFFF)
                    datos[j].append(int(xValue))

            contador += 1
            if contador == 864:
                contador = 0

    datos_np = np.asarray(datos)

    if segundos_faltantes:
        print("Segundos faltantes:", segundos_faltantes)

    return datos_np, segundos_faltantes if segundos_faltantes else None

# Extrae y convierte valores de tiempo del archivo binario y los devuelve en un diccionario.
def extraer_tiempo_binario(archivo):
    # Abrir el archivo en modo de lectura binaria
    with open(archivo, "rb") as f:
        # Leer 2506 bytes del archivo y almacenarlos en un arreglo de numpy
        tramaDatos = np.fromfile(f, np.int8, 2506)
    
    # Extraer valores de tiempo de posiciones específicas
    hora = tramaDatos[2503]
    minuto = tramaDatos[2504]
    segundo = tramaDatos[2505]
    n_segundo = hora * 3600 + minuto * 60 + segundo
    
    anio = tramaDatos[2500] + 2000
    mes = tramaDatos[2501]
    dia = tramaDatos[2502]
    
    # Crear diccionario de resultados con valores numéricos y cadenas formateadas
    tiempo_binario = {
        "anio": anio,
        "anio_s": str(anio),
        "mes": mes,
        "mes_s": str(mes).zfill(2),
        "dia": dia,
        "dia_s": str(dia).zfill(2),
        "hora": hora,
        "hora_s": str(hora).zfill(2),
        "minuto": minuto,
        "minuto_s": str(minuto).zfill(2),
        "segundo": segundo,
        "segundo_s": str(segundo).zfill(2),
        "n_segundo": n_segundo
    }

    return(tiempo_binario)

# Genera el nombre del archivo Mini-SEED basado en el tipo de archivo, el código de estación y el tiempo extraído.
def nombrar_archivo_mseed(tipoArchivo,codigo_estacion,tiempo_binario):
   # Formatear fecha y hora como cadenas
    fecha_string = tiempo_binario["anio_s"] + tiempo_binario["mes_s"] + tiempo_binario["dia_s"]
    hora_string = tiempo_binario["hora_s"] + tiempo_binario["minuto_s"] + tiempo_binario["segundo_s"]
    
    # Determinar el nombre del archivo basado en el tipo de archivo
    if tipoArchivo == '1':
        # Archivos de registro continuo
        fileName = f'/home/rsa/resultados/mseed/{codigo_estacion}_{fecha_string}_{hora_string}.mseed'
    elif tipoArchivo == '2':
        # Archivos de eventos extraídos
        fileName = f'/home/rsa/resultados/eventos-extraidos/{codigo_estacion}_{fecha_string}_{hora_string}.mseed'
    
    print(fileName)

    return fileName
    
# Convierte los datos procesados del archivo binario a formato Mini-SEED y los guarda con el nombre especificado.
def conversion_mseed_digital(fileName, tiempo_binario, datos_archivo_binario, segundos_faltantes, parametros_mseed):
    nombre = parametros_mseed["SENSOR(2)"]

    # Crear trazas para cada canal
    trazaCH1 = obtenerTraza(nombre, 1, datos_archivo_binario[0], tiempo_binario, segundos_faltantes, parametros_mseed)
    trazaCH2 = obtenerTraza(nombre, 2, datos_archivo_binario[1], tiempo_binario, segundos_faltantes, parametros_mseed)        
    trazaCH3 = obtenerTraza(nombre, 3, datos_archivo_binario[2], tiempo_binario, segundos_faltantes, parametros_mseed)

    # Crear un objeto Stream con las trazas
    stData = Stream(traces=[trazaCH1, trazaCH2, trazaCH3])
    
    stData.write(fileName, format='MSEED', encoding='STEIM1', reclen=512)

# Crea una traza de datos con los parámetros especificados y ajusta los datos para incluir ceros en los segundos faltantes si es necesario.
def obtenerTraza(nombreCanal, num_canal, data, tiempo_binario, segundos_faltantes, parametros_mseed):
    anio = tiempo_binario["anio"]
    mes = tiempo_binario["mes"]
    dia = tiempo_binario["dia"]
    horas = tiempo_binario["hora"]
    minutos = tiempo_binario["minuto"]
    segundos = tiempo_binario["segundo"]
    microsegundos = 0  # Si siempre es 0, podemos establecerlo aquí directamente

    fsample = int(parametros_mseed["MUESTREO(20)"])
    calidad = parametros_mseed["CALIDAD(16)"]

    # Determinar el prefijo del nombre del canal basado en la frecuencia de muestreo
    if fsample > 80:
        nombreCanal = 'E'
    else:
        nombreCanal = 'S'

    # Añadir el sufijo basado en el tipo de sensor
    if parametros_mseed["SENSOR(2)"] == 'SISMICO':
        nombreCanal += 'L'
    else:
        nombreCanal += 'N'

    # Determinar el índice del canal
    num_canal = num_canal - 3 * (int((num_canal - 1) / 3))
    nombreCanal += parametros_mseed["CANAL(18)"][num_canal - 1:num_canal]

    # Crear diccionario de estadísticas
    stats = {
        'network': parametros_mseed["RED(19)"],
        'station': parametros_mseed["CODIGO(1)"],
        'location': str(parametros_mseed["UBICACION(17)"]),  # Convertir a cadena
        'channel': nombreCanal,
        'npts': len(data),
        'sampling_rate': fsample,
        'mseed': {'dataquality': calidad},
        'starttime': UTCDateTime(anio, mes, dia, horas, minutos, segundos, microsegundos)
    }

    # Si hay segundos faltantes, ajustar los datos para incluir ceros en los segundos faltantes
    if segundos_faltantes is not None:
        segundo_inicio = (horas * 3600) + (minutos * 60) + segundos
        muestras_por_segundo = fsample
        lista_ceros = np.zeros(muestras_por_segundo, dtype=np.int32)
        npts_completo = len(data) + int(len(segundos_faltantes) * muestras_por_segundo)
        data_completo = np.zeros(npts_completo, dtype=np.int32)
        data_completo[:len(data)] = data
        stats['npts'] = npts_completo

        for segundo_faltante in segundos_faltantes:
            tiempo_muestra_faltante = int(segundo_faltante - segundo_inicio)
            indice_muestra_faltante = tiempo_muestra_faltante * muestras_por_segundo
            data_completo = np.insert(data_completo, indice_muestra_faltante, lista_ceros)

        traza = Trace(data=data_completo, header=stats)
    else:
        traza = Trace(data=data, header=stats)

    return traza

#######################################################################################################

############################################ ~Main~ ###################################################
def main():

    # Recibe como parametro el tipo de archivo binario a convertir (1:Resgistro continuo 2:Eventos extraidos)
    tipoArchivo = sys.argv[1] 

    config_mseed_path = '/home/rsa/configuracion/configuracion_mseed.json'
    config_dispositivo_path = '/home/rsa/configuracion/DatosConfiguracion.json'
    archivoNombresArchivosRC = '/home/rsa/tmp/NombreArchivoRegistroContinuo.tmp'
    archivoNombresArchivosEE = '/home/rsa/tmp/NombreArchivoEventoExtraido.tmp'

    # Lee el archivo de configuración mseed
    config_mseed = read_fileJSON(config_mseed_path)
    if config_mseed is None:
        print("No se pudo leer el archivo de configuración. Terminando el programa.")
        return
    
    # Lee el archivo de configuración del dispositivo
    config_dispositivo = read_fileJSON(config_dispositivo_path)
    if config_dispositivo is None:
        print("No se pudo leer el archivo de configuración del dispositivo. Terminando el programa.")
        return
    
    if tipoArchivo=='1':
        #Archivos registro continuo
        path_registro_continuo = config_dispositivo.get("directorios", {}).get("registro-continuo", "Unknown")
        with open(archivoNombresArchivosRC) as ficheroNombresArchivos:
            lineasFicheroNombresArchivos = ficheroNombresArchivos.readlines()
            nombreArchvioRegistroContinuo = lineasFicheroNombresArchivos[1].rstrip('\n')
            archivo_binario = path_registro_continuo + nombreArchvioRegistroContinuo
            print(f'Convirtiendo el archivo: {archivo_binario}')
    elif tipoArchivo=='2':
        #Archivos eventos extraidos
       with open(archivoNombresArchivosEE) as ficheroNombresArchivos:
            lineasFicheroNombresArchivos = ficheroNombresArchivos.readlines()
            archivo_binario = lineasFicheroNombresArchivos[0].rstrip('\n')
            print(f'Convirtiendo el archivo: {archivo_binario}')

    
    codigo_estacion = config_mseed["CODIGO(1)"]
    tiempo_binario = extraer_tiempo_binario(archivo_binario)
    nombre_archivo_mseed = nombrar_archivo_mseed(tipoArchivo, codigo_estacion, tiempo_binario)
    datos_archivo_binario, segundos_faltantes = leer_archivo_binario(archivo_binario)

    conversion_mseed_digital(nombre_archivo_mseed, tiempo_binario, datos_archivo_binario, segundos_faltantes, config_mseed)

    print('Se ha creado el archivo: %s' %nombre_archivo_mseed)

    '''
    # Sube los archivos convertidos a Drive
    if tipoArchivo=='1':
        subprocess.run(["python3", "/home/rsa/ejecutables/SubirArchivoDrive.py", "3", n_mseed])
        time.sleep(5)
        subprocess.run(["python3", "/home/rsa/ejecutables/SubirArchivoDrive.py", "1", archivo_binario])
    elif tipoArchivo=='2':
        subprocess.run(["python3", "/home/rsa/ejecutables/SubirArchivoDrive.py", "2", n_mseed])
    '''

#######################################################################################################
if __name__ == '__main__':
    main()
#######################################################################################################

