import paho.mqtt.client as mqtt
import json
import os
import datetime
import subprocess
import sys
import math

from datetime import timedelta

path = '/home/rsa/configuracion'

############################################ ~FUNCIONES~ ############################################
# Definir los callbacks para los eventos de conexión
def on_connect(client, userdata, flags, rc):
    print("Conectado al broker MQTT con código de resultado: " + str(rc))
    # Suscribirse al topic al conectarse
    client.subscribe(topic)

def on_message(client, userdata, msg):
    print("Mensaje recibido en el topic " + msg.topic + " con el contenido: " + str(msg.payload))
    #procesar_mensaje(msg)
    print(str(msg.payload))
    payload_str = msg.payload.decode('utf-8')
    print(payload_str)
    #BuscarArchivoRegistro(payload_str)

    # Publicar en el topic que ya se cumplio la tarea
    publicar_mensaje(client, "status", "completado")

def publicar_mensaje(client, topic, mensaje):
    client.publish(topic, mensaje)

def read_json(path_base):
    with open(path_base,"r") as condig_file:
        config = json.load(condig_file)
    return config

def conversion_fecha(fecha,hora):
    hora_transf = str(timedelta(seconds=hora))
    
    fecha_transf = '20'+fecha[0:2]+'-'+fecha[2:4]+'-'+fecha[4:]
    
    return fecha_transf+'T'+hora_transf+'Z'

#####################################################################################################

######################################### ~PARAMETROS MQTT~ #########################################
mqtt_config = read_json(path+'/mqtt-configuracion.json')
server_address = mqtt_config["server_address"]
username = mqtt_config["username"]
password = mqtt_config["password"]
topic = "registrocontinuo/eventos"
#####################################################################################################

datos_config = read_json(path+'/DatosConfiguracion.json')
fecha = sys.argv[1]

hora = sys.argv[2]
hora = int(hora)

duracion = sys.argv[3]

# Crear una instancia del cliente MQTTc
client = mqtt.Client()

# Asignar los callbacks de conexión y de recepción de mensajes
client.on_connect = on_connect


# Conectarse al broker MQTT
client.username_pw_set(username, password)
client.connect(server_address, 1883, 60)

dic = {
    datos_config["dispositivo"]["ubicacion"]:{
        datos_config["dispositivo"]["id"]:{
            "inicio": conversion_fecha(fecha,hora),
            "duracion":duracion  
        }
    }
}
publicar_mensaje(client, topic, json.dumps(dic))