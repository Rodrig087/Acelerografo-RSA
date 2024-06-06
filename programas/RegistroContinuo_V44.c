// Compilar:
// gcc -o /home/rsa/ejecutables/acelerografo /home/rsa/desarrollo/RegistroContinuo_*.c \
-I/home/rsa/librerias/lector-json \
-I/home/rsa/librerias/detector-eventos \
-L/home/rsa/librerias/lector-json \
-L/home/rsa/librerias/detector-eventos \
-llector_json -ldetector_eventos \
-Wl,-rpath,/home/rsa/librerias/lector-json \
-Wl,-rpath,/home/rsa/librerias/detector-eventos \
-lbcm2835 -lwiringPi -lm -ljansson
//

// Para manejo del tiempo
#define _XOPEN_SOURCE // Debe ir en la primera linea
#include <time.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <bcm2835.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// Incluye las librerias de deteccion de eventos y de lectura del archivo json
#include "lector_json.h"
#include "detector_eventos.h"

// Declaracion de constantes
#define P2 2
#define P1 0
#define MCLR 28    // Pin 38
#define LedTest 26 // Pin 32
#define NUM_MUESTRAS 199
#define NUM_ELEMENTOS 2506
#define TIEMPO_SPI 10
#define NUM_CICLOS 1
#define FreqSPI 2000000

// Declaracion de variables
unsigned short i;
unsigned int x;
unsigned short buffer;
unsigned short banFile;
unsigned short banNewFile;
unsigned short numBytes;
unsigned short contMuestras;
unsigned char tiempoPIC[8];
unsigned char tiempoLocal[8];
unsigned char tramaDatos[NUM_ELEMENTOS];

// Variables para crear los archivos de datos:
char filenameArchivoRegistroContinuo[100];
char filenameTemporalRegistroContinuo[100];
char filenameEventosDetectados[100];
char filenameActualRegistroContinuo[100];

char comando[40];
char dateGPS[22];
unsigned int timeNewFile[2] = {0, 0}; // Variable para configurar la hora a la que se desea generar un archivo nuevo (hh, mm)
unsigned short confGPS[2] = {0, 1};   // Parametros que se pasan para configurar el GPS (conf, NMA) cuando conf=1 realiza la configuracion del GPS y se realiza una sola vez la primera vez que es utilizado
unsigned short banNewFile;

unsigned short contCiclos;
unsigned short contador;

// Variables para control de tiempo:
int fuenteTiempo;
unsigned short fuenteTiempoPic;
unsigned short banTiempoRed, banTiempoRTC;
char datePICStr[20];
char datePicUNIX[15];
char dateRedUNIX[15];
struct tm datePIC;
// struct tm dateRed;
long tiempoPicUNIX, tiempoRedUNIX, deltaUNIX;

// Variables para extraer los datos de configuracion:
char id[10];
char deteccion_eventos[10];
char registro_continuo[60];
char eventos_detectados[60];
char archivos_temporales[60];

char extBin[5];
char extTxt[5];
char extTmp[5];
char nombreActualARC[25];
char nombreAnteriorARC[26];

// Variables para crear los archivos de datos:
char archivoRegistroContinuo[35];
char temporalRegistroContinuo[35];
char archivoEventoDetectado[35];
char archivoActualRegistroContinuo[35];

FILE *fp;
FILE *ftmp;
FILE *fTramaTmp;
FILE *ficheroDatosConfiguracion;
FILE *obj_fp;

// Metodo para manejar el tiempo del sistema
int ComprobarNTP();

// Metodos para la comunicacion con el dsPIC
int ConfiguracionPrincipal();
void LeerArchivoConfiguracion();
void CrearArchivos();
void GuardarVector(unsigned char *tramaD);
void ObtenerOperacion();                      // C:0xA0    F:0xF0
void IniciarMuestreo();                       // C:0xA1	F:0xF1
void DetenerMuestreo();                       // C:0xA2	F:0xF2
void NuevoCiclo();                            // C:0xA3	F:0xF3
void EnviarTiempoLocal();                     // C:0xA4	F:0xF4
void ObtenerTiempoPIC();                      // C:0xA5	F:0xF5
void ObtenerReferenciaTiempo(int referencia); // C:0xA6	F:0xF6
void SetRelojLocal(unsigned char *tramaTiempo);

// Declaración global
struct datos_config *datos_configuracion;

int main(void)
{

    printf("\n\nIniciando...\n");

    // Inicializa las variables:
    i = 0;
    x = 0;
    contMuestras = 0;
    banFile = 0;
    banNewFile = 0;
    numBytes = 0;
    contCiclos = 0;
    contador = 0;

    banTiempoRed = 0;
    banTiempoRTC = 0;

    // Realiza la configuracion principal:
    ConfiguracionPrincipal();

    // Comprueba si el equipo esta sincronizado con el tiempo de red:
    banTiempoRed = ComprobarNTP();

    // Lee el archivo de configuracion en formato json
    const char *filename = "/home/rsa/configuracion/configuracion_dispositivo.json";
    struct datos_config *datos_configuracion = compilar_json(filename);

    if (datos_configuracion != NULL)
    {
        printf("ID: %s\n", datos_configuracion->id);
        printf("Fuente de reloj: %s\n", datos_configuracion->fuente_reloj);
        printf("Deteccion de eventos: %s\n", datos_configuracion->deteccion_eventos);
        printf("Path archivos temporales: %s\n", datos_configuracion->archivos_temporales);
        printf("Path archivos registro continuo: %s\n", datos_configuracion->registro_continuo);
        printf("Path eventos detectados: %s\n", datos_configuracion->eventos_detectados);

        int fuente_reloj = atoi(datos_configuracion->fuente_reloj);     

        // Asignar el valor a la variable global
        strncpy(deteccion_eventos, datos_configuracion->deteccion_eventos, sizeof(deteccion_eventos) - 1);
        deteccion_eventos[sizeof(deteccion_eventos) - 1] = '\0'; // Asegurar la terminación nula

        // Obtiene la referencia de tiempo | 0:RPi 1:GPS 2:RTC
        if (fuente_reloj == 0 || fuente_reloj == 1 || fuente_reloj == 2)
        {
            ObtenerReferenciaTiempo(fuente_reloj);
        }
        else
        {
            fprintf(stderr, "Error: No se pudo recuperar la fuente de reloj. Revise el archivo de configuracion.\n");
            ObtenerReferenciaTiempo(0);
        }

        // Comprueba que la deteccion de eventos este habilitada en el archivo jason para lamar al metodo de inicializacion del filtro FIR
        
        if (strcmp(deteccion_eventos, "si") == 0){
            firFloatInit();
        } 
        

        free(datos_configuracion);

    }
    else
    {
        fprintf(stderr, "Error: No se pudo leer el archivo de configuracion.\n");
    }


    while (1)
    {
        delay(10);
    }

    bcm2835_spi_end();
    bcm2835_close();

    return 0;
}

int ConfiguracionPrincipal()
{

    // Reinicia el modulo SPI
    system("sudo rmmod  spi_bcm2835");
    // bcm2835_delayMicroseconds(500);
    system("sudo modprobe spi_bcm2835");

    // Configuracion libreria bcm2835:
    if (!bcm2835_init())
    {
        printf("bcm2835_init fallo. Ejecuto el programa como root?\n");
        return 1;
    }
    if (!bcm2835_spi_begin())
    {
        printf("bcm2835_spi_begin fallo. Ejecuto el programa como root?\n");
        return 1;
    }

    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);
    // bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);					//Clock divider RPi 2
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64); // Clock divider RPi 3
    bcm2835_spi_set_speed_hz(FreqSPI);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

    // Configuracion libreria WiringPi:
    wiringPiSetup();
    pinMode(P1, INPUT);
    pinMode(MCLR, OUTPUT);
    pinMode(LedTest, OUTPUT);
    wiringPiISR(P1, INT_EDGE_RISING, ObtenerOperacion);

    // Enciende el pin LedTest
    digitalWrite(LedTest, HIGH);

    printf("\n****************************************\n");
    printf("Configuracion completa\n");
    printf("****************************************\n");
}

int ComprobarNTP()
{
    char str1[5];
    const char *str2 = "yes";
    int v;

    printf("****************************************\n");
    printf("Comprobando conexion con servidor NTP...\n");
    // system("timedatectl | grep System | tail -c4");
    FILE *ftimedate = popen("timedatectl | grep System | tail -c4", "r");
    fscanf(ftimedate, "%s", str1);
    printf("System clock synchronized: %s\n", str1);
    pclose(ftimedate);

    // Comprueba si System clock synchronized == yes
    v = strcmp(str1, str2);
    if (v == 0)
    {
        system("date");
        time_t now;
        struct tm *lt;
        now = time(NULL);
        lt = localtime(&now);
        lt->tm_year;
        lt->tm_mon;
        lt->tm_mday;
        lt->tm_hour;
        lt->tm_min;
        lt->tm_sec;
        time_t midnight = mktime(lt);
        tiempoRedUNIX = (long)midnight;
        printf("Tiempo UNIX red: %d\n", tiempoRedUNIX);
        printf("****************************************\n");
        return 1;
    }
    else
    {
        return 2;
        printf("****************************************\n");
    }
}


void CrearArchivos()
{
    // Se leen los datos de configuracion:
    printf("\nLeyendo archivo de configuracion...\n");

    // Abre el fichero de datos de configuracion:
    ficheroDatosConfiguracion = fopen("/home/rsa/configuracion/DatosConfiguracion.txt", "r");
    // Recupera el contenido del archivo en la variable arg1 hasta que encuentra el carácter de fin de línea (\n):
    // fgets(id, 10, ficheroDatosConfiguracion); //La funcion fgets lee el archivo linea por linea, esta parte es necesaria para saltar el encabezado del archivo
    fgets(id, 10, ficheroDatosConfiguracion);
    fgets(archivos_temporales, 60, ficheroDatosConfiguracion);
    fgets(registro_continuo, 60, ficheroDatosConfiguracion);
    fgets(eventos_detectados, 60, ficheroDatosConfiguracion);
    // Cierra el fichero de informacion:
    fclose(ficheroDatosConfiguracion);

    // Elimina el caracter de fin de linea (\n):
    strtok(id, "\n");
    strtok(registro_continuo, "\n");
    strtok(eventos_detectados, "\n");
    strtok(archivos_temporales, "\n");
    // Elimina el caracter de retorno de carro (\r):
    strtok(id, "\r");
    strtok(registro_continuo, "\r");
    strtok(eventos_detectados, "\r");
    strtok(archivos_temporales, "\r");

    // Asigna el texto correspondiente a los array de carateres:
    strcpy(extBin, ".dat");
    strcpy(extTxt, ".txt");
    strcpy(extTmp, ".tmp");

    // Se crean los archivos necesarios para almacenar los datos:
    printf("\nSe crearon los archivos:\n");

    // Obtiene la hora y la fecha del sistema:
    time_t t;
    struct tm *tm;
    t = time(NULL);
    tm = localtime(&t);

    //*****************************************************************************
    // Crea el archivo binario para los datos de registro continuo:
    // Establece la fecha y hora actual como nombre que tendra el archivo binario
    strftime(archivoRegistroContinuo, 20, "%y%m%d-%H%M%S", tm);
    // Realiza la concatenacion de array de caracteres
    strcat(filenameArchivoRegistroContinuo, registro_continuo);
    strcat(filenameArchivoRegistroContinuo, id);
    strcat(filenameArchivoRegistroContinuo, archivoRegistroContinuo);
    strcat(filenameArchivoRegistroContinuo, extBin);
    printf("   %s\n", filenameArchivoRegistroContinuo);
    // Crea el archivo binario en modo append
    fp = fopen(filenameArchivoRegistroContinuo, "ab+");
    //*****************************************************************************

    //*****************************************************************************
    // Crea el archivo temporal para los datos de registro continuo:
    strcpy(temporalRegistroContinuo, "TramaTemporal");
    strcat(filenameTemporalRegistroContinuo, archivos_temporales);
    strcat(filenameTemporalRegistroContinuo, temporalRegistroContinuo);
    strcat(filenameTemporalRegistroContinuo, extTmp);
    // Crea el archivo de texto en modo sobrescritura
    // fTramaTmp = fopen(filenameTemporalRegistroContinuo, "wb");
    // printf("   %s\n",filenameTemporalRegistroContinuo);
    //*******************************************
    //*****************************************************************************

    //*****************************************************************************
    // Crea el archivo de texto para guardar los eventos detectados
    strcpy(archivoEventoDetectado, "EventosDetectados");
    strcat(filenameEventosDetectados, eventos_detectados);
    strcat(filenameEventosDetectados, id);
    strcat(filenameEventosDetectados, archivoEventoDetectado);
    strcat(filenameEventosDetectados, extTxt);
    // Crea el archivo de texto en modo append
    obj_fp = fopen(filenameEventosDetectados, "a");
    printf("   %s\n", filenameEventosDetectados);
    //*****************************************************************************

    //*****************************************************************************
    // Crea el archivo temporal para guardar los nombres actual y anterior de los archivos RC
    // Establece el nombre del archivo temporal:
    strcpy(archivoActualRegistroContinuo, "NombreArchivoRegistroContinuo");
    strcat(filenameActualRegistroContinuo, archivos_temporales);
    strcat(filenameActualRegistroContinuo, archivoActualRegistroContinuo);
    strcat(filenameActualRegistroContinuo, extTmp);
    // Abre el archivo temporal en modo lectura (El archivo debe existir):
    ftmp = fopen(filenameActualRegistroContinuo, "rt");
    // Recupera el nombre del archivo anterior (Lee solo la primera fila del archivo temporal):
    fgets(nombreAnteriorARC, 26, ftmp);
    // Cierra el archivo temporal:
    fclose(ftmp);

    // Abre el archivo temporal en modo escritura:
    ftmp = fopen(filenameActualRegistroContinuo, "w+");
    // printf("   %s\n",filenameActualRegistroContinuo);

    // Establece el nombre del archivo RC actual (es el mismo nombre sin el path):
    strcat(nombreActualARC, id);
    strcat(nombreActualARC, archivoRegistroContinuo);
    strcat(nombreActualARC, extBin);
    strcat(nombreActualARC, "\n");

    // Escribe el nombre del archivo RC actual en el archivo temporal:
    fwrite(nombreActualARC, sizeof(char), 24, ftmp);
    // Escribe el nombre del archivo RC anterior en el archivo temporal:
    fwrite(nombreAnteriorARC, sizeof(char), 24, ftmp);

    printf("\nArchivo RC Actual: %s", nombreActualARC);
    printf("Archivo RC Anterior: %s\n\n", nombreAnteriorARC);

    // Cierra el archivo temporal:
    fclose(ftmp);
    //*****************************************************************************
}

//**************************************************************************************************************************************
// Comunicacion RPi-dsPIC:

// C:0xA0	F:0xF0
void ObtenerOperacion()
{
    // bcm2835_delayMicroseconds(200);

    digitalWrite(LedTest, !digitalRead(LedTest));

    bcm2835_spi_transfer(0xA0);
    bcm2835_delayMicroseconds(TIEMPO_SPI);
    buffer = bcm2835_spi_transfer(0x00);
    bcm2835_delayMicroseconds(TIEMPO_SPI);
    bcm2835_spi_transfer(0xF0);
    // bcm2835_delayMicroseconds(TIEMPO_SPI);
    //   printf("%X \n", buffer);

    delay(1);

    // Aqui se selecciona el tipo de operacion que se va a ejecutar
    if (buffer == 0xB1)
    {
        // printf("Interrupcion P1: 0xB1\n");
        NuevoCiclo();
    }
    if (buffer == 0xB2)
    {
        printf("Interrupcion P1: 0xB2\n");
        printf("****************************************\n");
        ObtenerTiempoPIC();
    }
}

// C:0xA1	F:0xF1
void IniciarMuestreo()
{
    printf("\nIniciando el muestreo...\n");
    bcm2835_spi_transfer(0xA1);
    bcm2835_delayMicroseconds(TIEMPO_SPI);
    bcm2835_spi_transfer(0x01);
    bcm2835_delayMicroseconds(TIEMPO_SPI);
    bcm2835_spi_transfer(0xF1);
    bcm2835_delayMicroseconds(TIEMPO_SPI);
}

// C:0xA3	F:0xF3
void NuevoCiclo()
{
    // printf("Nuevo ciclo\n");
    bcm2835_spi_transfer(0xA3); // Envia el delimitador de inicio de trama
    bcm2835_delayMicroseconds(TIEMPO_SPI);

    for (i = 0; i < 2506; i++)
    {
        buffer = bcm2835_spi_transfer(0x00); // Envia 2506 dummy bytes para recuperar los datos de la trama enviada desde el dsPIC
        tramaDatos[i] = buffer;              // Guarda los datos en el vector tramaDatos
        bcm2835_delayMicroseconds(TIEMPO_SPI);
    }

    bcm2835_spi_transfer(0xF3); // Envia el delimitador de final de trama
    bcm2835_delayMicroseconds(TIEMPO_SPI);

    GuardarVector(tramaDatos); // Guarda la el vector tramaDatos en el archivo binario
    // CrearArchivos();           //Crea un archivo nuevo si se cumplen las condiciones

    // Llama al metodo para determinar si existe o no un evento sismico:
    if (strcmp(deteccion_eventos, "si") == 0){
        DetectarEvento(tramaDatos);
    }
    
}

// C:0xA4	F:0xF4
void EnviarTiempoLocal()
{
    time_t t;
    struct tm *tm;
    int ban_segundo_inicio = 0; // Bandera para controlar el bucle
    int segundo_anterior = -1;  // Inicializa para detectar el cambio de segundos

    printf("Esperando inicio de segundo...\n");

    // Espera en el bucle hasta que el segundo actual sea 0
    while (ban_segundo_inicio == 0)
    {

        // Obtiene la hora y la fecha del sistema:
        time(&t);
        tm = localtime(&t);
        int segundo_actual = tm->tm_sec;

        // Envía la trama de tiempo al detectarse un cambio de segundo
        if (segundo_actual == 0 || (segundo_actual % 2 == 0))
        {
            printf("Enviando tiempo local: ");
            tiempoLocal[0] = tm->tm_year - 100; // Anio (contado desde 1900)
            tiempoLocal[1] = tm->tm_mon + 1;    // Mes desde Enero (0-11)
            tiempoLocal[2] = tm->tm_mday;       // Dia del mes (0-31)
            tiempoLocal[3] = tm->tm_hour;       // Hora
            tiempoLocal[4] = tm->tm_min;        // Minuto
            tiempoLocal[5] = segundo_actual;    // Segundo
            printf("%0.2d:", tiempoLocal[3]);   // hh
            printf("%0.2d:", tiempoLocal[4]);   // mm
            printf("%0.2d ", tiempoLocal[5]);   // ss
            printf("%0.2d/", tiempoLocal[0]);   // AA
            printf("%0.2d/", tiempoLocal[1]);   // MM
            printf("%0.2d\n", tiempoLocal[2]);  // DD
            printf("****************************************\n");

            // Envia la trama de tiempo a través de SPI
            bcm2835_spi_transfer(0xA4); // Envia el delimitador de inicio de trama
            bcm2835_delayMicroseconds(TIEMPO_SPI);
            for (int i = 0; i < 6; i++)
            {
                bcm2835_spi_transfer(tiempoLocal[i]); // Envia los 6 datos de la trama tiempoLocal al dsPIC
                bcm2835_delayMicroseconds(TIEMPO_SPI);
            }
            bcm2835_spi_transfer(0xF4); // Envia el delimitador de final de trama
            bcm2835_delayMicroseconds(TIEMPO_SPI);

            ban_segundo_inicio = 1; // Actualiza la bandera para salir del bucle
        }

        // Espera 1000us (1ms) antes de verificar nuevamente
        bcm2835_delayMicroseconds(1000);
    }
}

// C:0xA5	F:0xF5
void ObtenerTiempoPIC()
{

    printf("Hora dsPIC: ");
    bcm2835_spi_transfer(0xA5); // Envia el delimitador de final de trama
    bcm2835_delayMicroseconds(TIEMPO_SPI);

    fuenteTiempoPic = bcm2835_spi_transfer(0x00); // Recibe el byte que indica la fuente de tiempo del PIC
    bcm2835_delayMicroseconds(TIEMPO_SPI);

    for (i = 0; i < 6; i++)
    {
        buffer = bcm2835_spi_transfer(0x00);
        tiempoPIC[i] = buffer; // Guarda la hora y fecha devuelta por el dsPIC
        bcm2835_delayMicroseconds(TIEMPO_SPI);
    }

    bcm2835_spi_transfer(0xF5); // Envia el delimitador de final de trama
    bcm2835_delayMicroseconds(TIEMPO_SPI);

    sprintf(datePICStr, "%0.2d:%0.2d:%0.2d %0.2d/%0.2d/%0.2d", tiempoPIC[3], tiempoPIC[4], tiempoPIC[5], tiempoPIC[0], tiempoPIC[1], tiempoPIC[2]);

    switch (fuenteTiempoPic)
    {
    case 0:
        printf("RPi ");
        break;
    case 1:
        printf("GPS ");
        break;
    case 2:
        printf("RTC ");
        break;
    default:
        printf("E%d ", fuenteTiempoPic);
        break;
    }

    // Imprime la trama de tiempo del dsPIC:
    printf("%s\n", datePICStr);

    // Imprime el tipo de error si es que existe:
    if (fuenteTiempoPic == 3 || fuenteTiempoPic == 4 || fuenteTiempoPic == 5)
    {
        switch (fuenteTiempoPic)
        {
        case 3:
            printf("**Error E3/GPS: No se pudo comprobar la trama GPRS\n");
            break;
        case 4:
            printf("**Error E4/RTC: No se pudo recuperar la trama GPRS\n");
            break;
        case 5:
            printf("**Error E5/RTC: El GPS no responde\n");
            break;
        }
    }

    // Calcula el tiempo UNIX de la trama de tiempo recibida del dsPIC
    strptime(datePICStr, "%H:%M:%S %y/%m/%d", &datePIC);        // Convierte el tiempo en string a struct
    strftime(datePicUNIX, sizeof(datePicUNIX), "%s", &datePIC); //%s: The number of seconds since the Epoch, 1970-01-01 00:00:00
    tiempoPicUNIX = atoi(datePicUNIX);
    printf("Tiempo UNIX dsPIC: %d\n", tiempoPicUNIX);
    printf("****************************************\n");

    /*
    // Comprueba si se recibio cualquier fuente de reloj proveniente del RTC del dsPIC:
    if (fuenteTiempoPic == 2 || fuenteTiempoPic == 4 || fuenteTiempoPic == 5)
    {
        // Comprueba que la hora de Red sea confiable:
        if (banTiempoRed == 1)
        {
            // Comprueba que la diferencia entre el tiempo de Red y el tiempo del RTC no sea mayor a 10 segundos:
            deltaUNIX = abs(tiempoRedUNIX - tiempoPicUNIX);
            if (deltaUNIX > 10)
            {
                // Envia la hora de la Red al dsPIC:
                printf("Hora RTC no valida. Enviando hora de Red...\n");
                fuenteTiempo = 0; // 0:RPi 1:GPS 2:RTC
                ObtenerReferenciaTiempo(fuenteTiempo);
            }
            else
            {
                printf("Hora RTC valida.\n");
                // Crea los archivos e inicia el muestreo:
                CrearArchivos();
                IniciarMuestreo();
            }
        }
        else
        {
            // Si la hora de Red no es confiable, utiliza la hora del RTC:
            SetRelojLocal(tiempoPIC);
            // Crea los archivos e inicia el muestreo:
            CrearArchivos();
            IniciarMuestreo();
        }
    }
    else
    {
        CrearArchivos();
        IniciarMuestreo();
    }
    */

    CrearArchivos();
    IniciarMuestreo();
}

// C:0xA6	F:0xF6
void ObtenerReferenciaTiempo(int referencia)
{
    // referencia = 0 -> RPi
    // referencia = 1 -> GPS
    // referencia = 2 -> RTC
    if (referencia == 0)
    {
        EnviarTiempoLocal();
    }
    else
    {
        if (referencia == 1)
        {
            printf("Obteniendo hora del GPS...\n");
        }
        else
        {
            printf("Obteniendo hora del RTC...\n");
        }
        printf("****************************************\n");
        bcm2835_spi_transfer(0xA6);
        bcm2835_delayMicroseconds(TIEMPO_SPI);
        bcm2835_spi_transfer(referencia);
        bcm2835_delayMicroseconds(TIEMPO_SPI);
        bcm2835_spi_transfer(0xF6);
        bcm2835_delayMicroseconds(TIEMPO_SPI);
    }
}
//**************************************************************************************************************************************

// Esta funcion sirve para guardar en el archivo binario las tramas de 1 segundo recibidas
void GuardarVector(unsigned char *tramaD)
{
    unsigned int outFwrite;
    unsigned int tmpFwrite;

    // Guarda la trama en el archivo de registro continuo
    if (fp != NULL)
    {
        do
        {
            // Guarda la trama en el archivo binario:
            outFwrite = fwrite(tramaD, sizeof(char), NUM_ELEMENTOS, fp);
            // Guarda la trama en el archivo temporal:
            // fTramaTmp = fopen(filenameTemporalRegistroContinuo, "wb");
            // tmpFwrite = fwrite(tramaD, sizeof(char), NUM_ELEMENTOS, fTramaTmp);
            // fclose(fTramaTmp); */
            // Ejecuta el script de python:
            // system("python /home/rsa/Programas/RegistroContinuo/V3/BinarioToMiniSeed_V12.py");
        } while (outFwrite != NUM_ELEMENTOS);
        fflush(fp);
    }

    // Guarda la trama en el archivo temporal

    fTramaTmp = fopen(filenameTemporalRegistroContinuo, "wb");
    if (fTramaTmp != NULL)
    {
        do
        {
            tmpFwrite = fwrite(tramaD, sizeof(char), NUM_ELEMENTOS, fTramaTmp);

        } while (tmpFwrite != NUM_ELEMENTOS);

        fflush(fTramaTmp);
        fclose(fTramaTmp);
    }
}

void SetRelojLocal(unsigned char *tramaTiempo)
{
    printf("Configurando hora de Red con la hora RTC...\n");
    char datePIC[22];
    // Configura el reloj interno de la RPi con la hora recuperada del PIC:
    strcpy(comando, "sudo date --set "); // strcpy( <variable_destino>, <cadena_fuente> )
    // Ejemplo: '2019-09-13 17:45:00':
    datePIC[0] = 0x27; //'
    datePIC[1] = '2';
    datePIC[2] = '0';
    datePIC[3] = (tramaTiempo[0] / 10) + 48; // dd
    datePIC[4] = (tramaTiempo[0] % 10) + 48;
    datePIC[5] = '-';
    datePIC[6] = (tramaTiempo[1] / 10) + 48; // MM
    datePIC[7] = (tramaTiempo[1] % 10) + 48;
    datePIC[8] = '-';
    datePIC[9] = (tramaTiempo[2] / 10) + 48;  // aa: (19/10)+48 = 49 = '1'
    datePIC[10] = (tramaTiempo[2] % 10) + 48; //    (19%10)+48 = 57 = '9'
    datePIC[11] = ' ';
    datePIC[12] = (tramaTiempo[3] / 10) + 48; // hh
    datePIC[13] = (tramaTiempo[3] % 10) + 48;
    datePIC[14] = ':';
    datePIC[15] = (tramaTiempo[4] / 10) + 48; // mm
    datePIC[16] = (tramaTiempo[4] % 10) + 48;
    datePIC[17] = ':';
    datePIC[18] = (tramaTiempo[5] / 10) + 48; // ss
    datePIC[19] = (tramaTiempo[5] % 10) + 48;
    datePIC[20] = 0x27;
    datePIC[21] = '\0';

    strcat(comando, datePIC);

    system(comando);
    system("date");
}
