LIB_DIR = /home/rsa/librerias
EXE_DIR = /home/rsa/ejecutables
PROG_DIR = /home/rsa/Acelerografo-RSA/programas

all: acelerografo comprobarregistro extraerevento resetmaster

acelerografo:
	cp $(PROG_DIR)/librerias/eventos.h $(LIB_DIR)/
	gcc -c -o $(LIB_DIR)/eventos.o $(PROG_DIR)/librerias/eventos.c 
	gcc -shared -o $(LIB_DIR)/libeventos.so $(LIB_DIR)/eventos.o
	gcc -o $(EXE_DIR)/acelerografo $(PROG_DIR)/RegistroContinuo_*.c -I$(LIB_DIR) -L$(LIB_DIR) -leventos -Wl,-rpath,$(LIB_DIR) -lbcm2835 -lwiringPi -lm

comprobarregistro:
	gcc $(PROG_DIR)/ComprobarRegistro_*.c -o $(EXE_DIR)/comprobarregistro

extraerevento:
	gcc $(PROG_DIR)/ExtraerEventoBin_*.c -o $(EXE_DIR)/extraerevento

resetmaster:
	gcc $(PROG_DIR)/ResetMaster.c -o $(EXE_DIR)/resetmaster -lbcm2835 -lwiringPi 

clean:
	rm -f $(LIB_DIR)/*.o $(LIB_DIR)/*.so $(EXE_DIR)/*[!py]
