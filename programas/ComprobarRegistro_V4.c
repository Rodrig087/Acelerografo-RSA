// gcc /home/rsa/programas/ComprobarRegistro_V4.c -o /home/rsa/programas/comprobarregistro

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

const int tramaSize = 2506;
#define NUM_MUESTRAS 249

unsigned int tiempoSegundos;
// Declaracion de variables
unsigned short i;
unsigned int x;
unsigned short banLinea;
unsigned short banInicio;
unsigned short numBytes;
unsigned short contMuestras;
unsigned int numCiclos;
unsigned char tramaInSPI[20];
unsigned char tramaDatos[16 + (NUM_MUESTRAS * 10)];
unsigned short tiempoSPI;
unsigned short fuenteReloj, banErrorReloj;

char entrada[30];
char salida[30];
char ext1[8];
char ext2[8];

char idEstacion[10];
char pathTMP[60];
char pathRegistroContinuo[60];
char nombreActualARC[25];
char filenameActualRegistroContinuo[100];

FILE *ficheroDatosConfiguracion;
FILE *tmpf;
FILE *lf;

unsigned int segInicio;
unsigned int segActual;
unsigned int segTranscurridos;
unsigned int tiempoSegundos;

unsigned short xData[3];
unsigned short yData[3];
unsigned short zData[3];

int xValue;
int yValue;
int zValue;
double xAceleracion;
double yAceleracion;
double zAceleracion;

int main()
{

    // Obtiene la hora y la fecha del sistema:
    time_t t;
    struct tm *tm_info;
    t = time(NULL);
    // Obtiene la hora y la fecha del sistema
    time(&t);
    tm_info = localtime(&t);
    // Formatea la fecha y hora en el formato deseado
    char formattedTime[20];
    strftime(formattedTime, 20, "%y/%m/%d %H:%M:%S", tm_info);
    // Imprime la fecha y hora formateadas
    printf("\nTiempo del sistema:\n");
    printf("%s\n", formattedTime);

    // Abre el fichero de datos de configuracion:
    ficheroDatosConfiguracion = fopen("/home/rsa/configuracion/DatosConfiguracion.txt", "r");
    fgets(idEstacion, 10, ficheroDatosConfiguracion);
    fgets(pathTMP, 60, ficheroDatosConfiguracion);
    fgets(pathRegistroContinuo, 60, ficheroDatosConfiguracion);
    // Cierra el fichero de informacion:
    fclose(ficheroDatosConfiguracion);

    // Elimina el caracter de fin de linea (\n):
    strtok(idEstacion, "\n");
    strtok(pathRegistroContinuo, "\n");
    strtok(pathTMP, "\n");
    // Elimina el caracter de retorno de carro (\r):
    strtok(idEstacion, "\r");
    strtok(pathRegistroContinuo, "\r");
    strtok(pathTMP, "\r");

    // Abre el archivo temporal en modo lectura
    tmpf = fopen("/home/rsa/tmp/NombreArchivoRegistroContinuo.tmp", "r");
    fgets(nombreActualARC, 25, tmpf);
    strtok(nombreActualARC, "\n");
    strtok(nombreActualARC, "\r");
    fclose(tmpf);

    // Incluye el path del nombre del archivo actual RC:
    strcat(filenameActualRegistroContinuo, pathRegistroContinuo);
    strcat(filenameActualRegistroContinuo, nombreActualARC);

    // const char *filenameActualRegistroContinuo = "/home/rsa/resultados/registro-continuo/TST00_230907-120002.dat";

    FILE *lf = fopen(filenameActualRegistroContinuo, "rb");
    if (!lf)
    {
        printf("No se pudo abrir el archivo.\n");
        return 1;
    }

    printf("\nArchivo actual: ");
    printf(nombreActualARC);

    // Calcula el tamaño total del archivo
    fseek(lf, 0, SEEK_END);
    long fileSize = ftell(lf);
    printf("\nTamaño del archivo:%d\n", fileSize);

    // Calcula el índice de la última trama
    long lastFrameIndex = (fileSize / tramaSize) - 1;

    // Calcula la posición de inicio de la última trama
    long lastFrameStart = lastFrameIndex * tramaSize;

    // Ve directamente a la posición de inicio de la última trama
    fseek(lf, lastFrameStart, SEEK_SET);

    // Lee la última trama
    char tramaDatos[tramaSize];
    fread(tramaDatos, sizeof(char), tramaSize, lf);

    // Ahora tienes la última trama en tramaDatos para su procesamiento
    // Recupera la primera trama de datos para obtener el tiempo de inicio:
    fread(tramaDatos, sizeof(char), tramaSize, lf);
    // Calcula el tiempo en segundos:
    tiempoSegundos = (3600 * tramaDatos[tramaSize - 3]) + (60 * tramaDatos[tramaSize - 2]) + (tramaDatos[tramaSize - 1]);

    // Extrae la fuente de reloj:
    fuenteReloj = tramaDatos[0];

    // Imprime la fuente de reloj:
    printf("\nDatos de la trama:\n");
    printf("| ");
    // Imprime la fuente de reloj:
    switch (fuenteReloj)
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
        printf("E%d ", fuenteReloj);
        banErrorReloj = 1;
        break;
    }
    // Imprime la fecha y hora:
    printf("%0.2d/", tramaDatos[tramaSize - 6]); // aa
    printf("%0.2d/", tramaDatos[tramaSize - 5]); // mm
    printf("%0.2d ", tramaDatos[tramaSize - 4]); // dd
    printf("%0.2d:", tramaDatos[tramaSize - 3]); // hh
    printf("%0.2d:", tramaDatos[tramaSize - 2]); // mm
    printf("%0.2d-", tramaDatos[tramaSize - 1]); // ss
    printf("%d ", tiempoSegundos);
    printf("| ");

    // Imprime los valores de acelaracion
    for (x = 0; x < 3; x++)
    {
        xData[x] = tramaDatos[x + 1];
        yData[x] = tramaDatos[x + 4];
        zData[x] = tramaDatos[x + 7];
    }

    // Calculo aceleracion eje x:
    xValue = ((xData[0] << 12) & 0xFF000) + ((xData[1] << 4) & 0xFF0) + ((xData[2] >> 4) & 0xF);
    // Apply two complement
    if (xValue >= 0x80000)
    {
        xValue = xValue & 0x7FFFF; // Se descarta el bit 20 que indica el signo (1=negativo)
        xValue = -1 * (((~xValue) + 1) & 0x7FFFF);
    }
    xAceleracion = xValue * (9.8 / pow(2, 18));

    // Calculo aceleracion eje y:
    yValue = ((yData[0] << 12) & 0xFF000) + ((yData[1] << 4) & 0xFF0) + ((yData[2] >> 4) & 0xF);
    // Apply two complement
    if (yValue >= 0x80000)
    {
        yValue = yValue & 0x7FFFF; // Se descarta el bit 20 que indica el signo (1=negativo)
        yValue = -1 * (((~yValue) + 1) & 0x7FFFF);
    }
    yAceleracion = yValue * (9.8 / pow(2, 18));

    // Calculo aceleracion eje z:
    zValue = ((zData[0] << 12) & 0xFF000) + ((zData[1] << 4) & 0xFF0) + ((zData[2] >> 4) & 0xF);
    // Apply two complement
    if (zValue >= 0x80000)
    {
        zValue = zValue & 0x7FFFF; // Se descarta el bit 20 que indica el signo (1=negativo)
        zValue = -1 * (((~zValue) + 1) & 0x7FFFF);
    }
    zAceleracion = zValue * (9.8 / pow(2, 18));

    printf("X: ");
    printf("%2.8f ", xAceleracion);
    printf("Y: ");
    printf("%2.8f ", yAceleracion);
    printf("Z: ");
    printf("%2.8f ", zAceleracion);
    printf("|\n");

    // Imprime el tipo de error si es que existe:
    if (banErrorReloj == 1)
    {
        switch (fuenteReloj)
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

    fclose(lf);

    return 0;
}
