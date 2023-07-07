import numpy as np
import csv
from obspy import UTCDateTime, read, Trace, Stream
import subprocess

# ///////////////////////////////// Archivos //////////////////////////////////

archivoConfiguracionMseed = '/home/rsa/configuracion/configuracion_mseed.csv'
archivoNombresArchivosRC = '/home/rsa/tmp/NombreArchivoRegistroContinuo.tmp'
archivoDatosConfiguracion = '/home/rsa/configuracion/DatosConfiguracion.txt'

ficheroConfiguracion = open(archivoDatosConfiguracion)
ficheroNombresArchivos = open(archivoNombresArchivosRC)
    
lineasFicheroConfiguracion = ficheroConfiguracion.readlines()
lineasFicheroNombresArchivos = ficheroNombresArchivos.readlines()
    
nombreArchvioRegistroContinuo = lineasFicheroNombresArchivos[1].rstrip('\n')
print(nombreArchvioRegistroContinuo)
archivo_binario = lineasFicheroConfiguracion[2].rstrip('\n') + nombreArchvioRegistroContinuo
print(archivo_binario)

# /////////////////////////////////////////////////////////////////////////////

def parametros_():
#################################################################
#Método de Extracción de datos de conficuración de la estacion
#Desde el archivo configuracion.csv que debe estar presente en el mismo directorio del ejecutable.
        
        estaciones_=[]
        with open(archivoConfiguracionMseed,newline='') as f:
            datos=csv.reader(f,delimiter=';',quotechar=';')
            for r in datos:
                estaciones_.append(r)
        nombre_canal_total_=estaciones_[1][1]
        nombre_canal=estaciones_[1][2]
        tipo_canal_=estaciones_[1][3]
        n_canales_=estaciones_[1][4]
        hab_canal=estaciones_[1][5]
        componente_canal=estaciones_[1][6]
        grafico_=estaciones_[1][7]
        hab_plt=estaciones_[1][8]
        gan_plt=estaciones_[1][9]
        ganancia=estaciones_[1][10]
        diez_plt=estaciones_[1][11]
        bits_=estaciones_[1][12]
        calidad_=estaciones_[1][13]
        ubicacion_=estaciones_[1][14]
        tipo_canal=estaciones_[1][15]
        red_=estaciones_[1][16]
        muestreo_=estaciones_[1][17]
        longitud_=estaciones_[1][18]
        latitud_=estaciones_[1][19]
        altura_=estaciones_[1][20]
        ruido_=int(estaciones_[1][21])
        numero_est=int(estaciones_[1][0])
        filtro_=estaciones_[1][22]
        reserva_1=estaciones_[1][23]
        reserva_2=estaciones_[1][24]            
                
                
        return(nombre_canal_total_, #Canal 0  'NOMBRE'
               nombre_canal,        #Canal 1  'CODIGO'
               tipo_canal_,         #Canal 2  'SENSOR'
               n_canales_,          #Canal 3  'CANALES'
               hab_canal,           #Canal 4  'HAB_CANAL'
               componente_canal,    #Canal 5  'COMPONENTE'
               grafico_,            #Canal 6  'HAB_GRAFICO'
               hab_plt,             #Canal 7  'HAB_PLT'
               gan_plt,             #Canal 8  'GAN_PLT'
               ganancia,            #Canal 9  'GANANCIA'
               diez_plt,            #Canal 10 'DIEZ_PLT'
               bits_,               #Canal 11 'FACTOR_MUL'
               longitud_,           #Canal 12 'LONGITUD'
               latitud_,            #Canal 13 'LATITUD'
               altura_,             #Canal 14 'ALTITUD'
               ruido_,              #Canal 15 'RUIDO'
               calidad_,            #Canal 16 'CALIDAD'
               ubicacion_,          #Canal 17 'UBICACIÓN'
               tipo_canal,          #Canal 18 'CANAL'
               red_,                #Canal 19 'RED'
               muestreo_,           #Canal 20 'MUESTREO'
               numero_est,          #Canal 21 'N° ESTACION'
               filtro_,             #Canal 22 'FILTRO' 
               reserva_1,           #Canal 22 'Reserva 1'            
               reserva_2)           #Canal 22 'Reserva 2'


def obtenerTraza(nombreCanal,num_canal, data, anio, mes, dia, horas, minutos, segundos, microsegundos):#Depurado
    # Define todas las caracteristicas de la traza
    parametros=parametros_()
    nombreRed=parametros[19]
    nombreEstacion=parametros[1]
    tipoEstacion=parametros[2]
    localizacion=parametros[17]
    nCanal=parametros[18]
    fsample=int(parametros[20])
    calidad=parametros[16]
    if fsample>80:
        nombreCanal='E'
    else:
        nombreCanal='S'
    if tipoEstacion=='SISMICO':
        nombreCanal=nombreCanal+'L'
    else:
        nombreCanal=nombreCanal+'N'
    num_canal=num_canal-3*(int((num_canal-1)/3))
    nombreCanal=nombreCanal+nCanal[num_canal-1:num_canal]
    stats = {'network': nombreRed, 'station': nombreEstacion, 'location': localizacion,
             'channel': nombreCanal, 'npts': len(data), 'sampling_rate': fsample,
             'mseed': {'dataquality': calidad}}
    # Establece el tiempo
    stats['starttime'] = UTCDateTime(anio, mes, dia, horas, minutos, segundos, microsegundos)    
    # Crea la traza con los datos las caracteristicas
    traza = Trace(data = data, header = stats)
    return traza


def lectura_archivo(archivo): #archivo 
    datos=[[],[],[]]
    bandera =1
    contador=0
    avance=0
    f = open(archivo, "rb")
    while bandera:
        tramaDatos = np.fromfile(f, np.int8, 2506)
        contador=contador+1
        if(len(tramaDatos)==2506):
            hora = tramaDatos[2503]
            minuto = tramaDatos[2504]
            segundo = tramaDatos[2505]
            n_segundo=hora*3600+minuto*60+segundo
        else:
            bandera=0
            break
        if(contador==864):
            contador=0
        for j in range(0,3):
            for i in range(0,250):
                dato_1=tramaDatos[i*10+j*3+1]
                dato_2=tramaDatos[i*10+j*3+2]
                dato_3=tramaDatos[i*10+j*3+3]
                xValue = ((dato_1 << 12) & 0xFF000) + ((dato_2 << 4) & 0xFF0) + ((dato_3 >> 4) & 0xF)
                if (xValue  >= 0x80000):
                    xValue  = xValue & 0x7FFFF  #Se descarta el bit 20 que indica el signo (1=negativo)
                    xValue = -1 * (((~xValue) + 1) & 0x7FFFF)
                datos[j].append(int(xValue))
    f.close
    datos_np = np.asarray(datos)        
    return (datos_np)  


def verificacion_archivo(archivo):
    f = open(archivo, "rb")
    tramaDatos = np.fromfile(f, np.int8, 2506)
    hora = tramaDatos[2503]
    minuto = tramaDatos[2504]
    segundo = tramaDatos[2505]
    n_segundo=hora*3600+minuto*60+segundo
    anio=tramaDatos[2500]
    anio_s= str(anio)
    mes=tramaDatos[2501]
    mes_s=str(mes)
    if(mes<10):
        mes_s='0'+mes_s
    dia=tramaDatos[2502]
    dia_s=str(dia)
    if(dia<10):
        dia_s='0'+dia_s
    hora_s=str(hora)
    if(hora<10):
        hora_s='0'+hora_s
    minuto_s=str(minuto)
    if(minuto<10):
        minuto_s='0'+minuto_s
    segundo_s=str(segundo)
    if(segundo<10):
        segundo_s='0'+segundo_s
    fecha_=(anio,mes,dia,hora,minuto,segundo,n_segundo),(anio_s,mes_s,dia_s,hora_s,minuto_s,segundo_s)
    f.close
    return(fecha_)


def nombre_mseed(nombre_,fecha_):
    anio_s=fecha_[1][0]
    mes_s=fecha_[1][1]
    dia_s=fecha_[1][2]
    hora_s=fecha_[1][3]
    minuto_s=fecha_[1][4]
    segundo_s=fecha_[1][5]
    fecha_string=anio_s+mes_s+dia_s        
    hora_string=hora_s+minuto_s+segundo_s
    fileName = '/home/rsa/resultados/mseed/'+nombre_+fecha_string+"_"+hora_string+".mseed" 
    return fileName
    
    
def conversion_mseed_digital(fileName,fecha_,data_np):
    # Nombre del archivo en funcion del tiempo de inicio
    anio=fecha_[0][0]
    mes=fecha_[0][1]
    dia=fecha_[0][2]
    horas=fecha_[0][3]
    minutos=fecha_[0][4]
    segundos=fecha_[0][5]
    parametros=parametros_()
    nombre_=parametros[2]

    #fileName=self.directorio_registros+fileName
    
    # Una vez que se tiene los datos, llama al metodo para obtener la traza
    # Todos los parametros que recibe se detallan en el metodo (mas abajo)
    trazaCH1 = obtenerTraza(nombre_,1, data_np[0],(2000 + anio), mes, dia, horas, minutos, segundos, 0)
    trazaCH2 = obtenerTraza(nombre_,2, data_np[1],(2000 + anio), mes, dia, horas, minutos, segundos, 0)        
    trazaCH3 = obtenerTraza(nombre_,3, data_np[2],(2000 + anio), mes, dia, horas, minutos, segundos, 0)
    # Crea un objeto Stream con la traza
    #stData = Stream(traces=[trazaCH1])
    # Si se desea varias trazas, esto seria para cuando se tiene 3 canales
    stData = Stream(traces=[trazaCH1, trazaCH2, trazaCH3])
    
    stData.write(fileName, format = 'MSEED', encoding = 'STEIM1', reclen = 512)


parametros=parametros_()
nombre=parametros[1]
fecha_=verificacion_archivo(archivo_binario)
nombre_= parametros[1]+'_20'
n_mseed=nombre_mseed(nombre_,fecha_)
datos_np=lectura_archivo(archivo_binario)
conversion_mseed_digital(n_mseed,fecha_,datos_np)

print(n_mseed)

subprocess.run(["python3", "/home/rsa/ejecutables/SubirArchivoDrive.py", "3", n_mseed])
