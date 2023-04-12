# Version de prueba instalada en el equipo CHA02 el 23/04/12

Instrucciones de uso:

Para configurar una estación por primera vez, siga los siguientes pasos:

1. Conceda permisos de ejecución al script menu.sh con el comando chmod +x menu.sh.
2. Ejecute el menú con el comando ./menu.sh.
3. Seleccione la opción 0 para preparar los scripts de configuración.
4. Seleccione la opción 1 para instalar las librerías necesarias.
5. Seleccione la opción 2 para configurar el entorno de ejecución.
6. Seleccione la opción 3 para compilar los programas necesarios para operar el sistema de registro continuo.

Los pasos 1 a 5 deben ejecutarse en orden una sola vez durante la configuración inicial de la estación.

Después de ejecutar estos pasos, es importante editar el archivo /home/rsa/configuracion/DatosConfiguracion.txt para incluir el nombre de la estación y los tokens de las carpetas de Google Drive donde se subirán los archivos. Además, se deben incluir en la carpeta "configuracion" los archivos de token de la cuenta de Google Drive correspondiente.

El paso número 6 debe ejecutarse cada vez que se realice una actualización del sistema.
