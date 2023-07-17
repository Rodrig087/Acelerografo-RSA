#!/bin/bash

# Función para mostrar el menú y leer la opción seleccionada
show_menu() {
  echo " "
  echo "*****************************************************************"
  echo "Bienvenido al menú de configuración"
  echo "*****************************************************************"
  echo "0. Preparar setup-scripts"
  echo "1. Instalar librerías"
  echo "2. Preparar el entorno"
  echo "3. Compilar scripts C"
  echo "4. Actualizar"
  echo "5. Salir"
  read -p "Ingrese el número de opción que desea ejecutar: " option
}

# Ciclo principal del programa
while true; do
  # Mostrar el menú y leer la opción seleccionada
  show_menu

  # Ejecutar el script correspondiente según la opción seleccionada
  case $option in
    0)
      echo "Preparando setup-scripts..."
      chmod +x setup-scripts/instalar-librerias.sh
      chmod +x setup-scripts/iniciar.sh
      chmod +x setup-scripts/compilar.sh
      ;;
    1)
      echo "Instalado librerias necesarias..."
      sh setup-scripts/instalar-librerias.sh
      ;;
    2)
      echo "Advertencia: Esta opción debe ejecutarse únicamente durante la configuración inicial de una nueva estación."
      echo "Si se ejecuta más de una vez puede borrar los archivos de configuracion."
      read -p "Desea continuar? (s/n) " response
      case "$response" in
        s|S)
            echo "Preparando el entorno..."
            sh setup-scripts/iniciar.sh
            break
            ;;
        n|N)
            ;;
        *)
            echo "Opción no válida, por favor ingrese s para sí o n para no."
            ;;
      esac
      ;;
    3)
      echo "Compilando..."
      sh setup-scripts/compilar.sh
      ;;
    4)
      echo "Actualizando.."
      sh setup-scripts/actualizar.sh
      ;;
    5)
      echo "Saliendo del programa..."
      echo "*****************************************************************"
      echo " "
      exit 0
      ;;
    *)
      echo "Opción no válida, por favor ingrese un número del 0 al 4."
      ;;
  esac
  echo "*****************************************************************"
done
