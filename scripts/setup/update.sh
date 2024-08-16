#!/bin/bash

# Definir la raíz del proyecto en Git y el proyecto local
USER_HOME=$(eval echo ~$SUDO_USER)  # Obtiene el home del usuario original cuando se usa sudo
PROJECT_GIT_ROOT="$USER_HOME/git/Acelerografo-RSA"
PROJECT_LOCAL_ROOT="$USER_HOME/projects/acelerografo-rsa"

# Función para actualizar archivos si han cambiado
function update_files_if_changed {
    local src_dir=$1
    local dest_dir=$2

    for src_file in $(find $src_dir -type f); do
        dest_file="${src_file/$src_dir/$dest_dir}"
        if [ ! -f "$dest_file" ] || [ "$src_file" -nt "$dest_file" ]; then
            echo "Actualizando: $dest_file"
            cp "$src_file" "$dest_file"
        fi
    done
}

# Función para actualizar task-scripts en /usr/local/bin
function update_task_scripts {
    local src_dir=$1
    local dest_dir="/usr/local/bin"

    for script in $(find $src_dir -name "*.sh" -type f); do
        script_name=$(basename "$script" .sh)
        dest_file="$dest_dir/$script_name"
        if [ ! -f "$dest_file" ] || [ "$script" -nt "$dest_file" ]; then
            echo "Actualizando: $dest_file"
            sudo cp "$script" "$dest_file"
            sudo chmod +x "$dest_file"
        fi
    done
}

# Revisar y actualizar archivos en configuración, mqtt, mseed, drive
update_files_if_changed "$PROJECT_GIT_ROOT/configuration/" "$PROJECT_LOCAL_ROOT/configuracion/"
update_files_if_changed "$PROJECT_GIT_ROOT/scripts/operation/mqtt/" "$PROJECT_LOCAL_ROOT/scripts/mqtt/"
update_files_if_changed "$PROJECT_GIT_ROOT/scripts/operation/mseed/" "$PROJECT_LOCAL_ROOT/scripts/mseed/"
update_files_if_changed "$PROJECT_GIT_ROOT/scripts/operation/drive/" "$PROJECT_LOCAL_ROOT/scripts/drive/"

# Revisar y actualizar task-scripts en /usr/local/bin
update_task_scripts "$PROJECT_GIT_ROOT/scripts/task/"

# Revisar si hay cambios en acelerografo o libraries y ejecutar make
if [ "$(find $PROJECT_GIT_ROOT/scripts/operation/acelerografo/ -type f -newer $PROJECT_LOCAL_ROOT/scripts/acelerografo/ejecutables/registro_continuo 2>/dev/null)" ] || \
   [ "$(find $PROJECT_GIT_ROOT/scripts/operation/acelerografo/libraries/ -type f -newer $PROJECT_LOCAL_ROOT/scripts/acelerografo/ejecutables/registro_continuo 2>/dev/null)" ]; then
    echo "Se detectaron cambios en acelerografo o libraries, ejecutando make..."
    cd $PROJECT_GIT_ROOT/scripts/setup/
    make
else
    echo "No se detectaron cambios en acelerografo o libraries."
fi

echo "Actualización completada con éxito."
