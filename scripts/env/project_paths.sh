#!/bin/bash

# Obtiene el home del usuario original cuando se usa sudo
USER_HOME=$(eval echo ~$SUDO_USER)

# Define las variables de entorno
export PROJECT_GIT_ROOT="$USER_HOME/git/Acelerografo-RSA"
export PROJECT_LOCAL_ROOT="$USER_HOME/projects/acelerografo-rsa"