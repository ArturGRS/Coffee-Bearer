#include "log.h"

void registrar_log(String evento) {
	if(!SPIFFS.exists("/datalog.txt")){
		File file = SPIFFS.open("/datalog.txt", FILE_WRITE);
		if(file){
			file.println("======== LOG DE EVENTOS DA CAFETEIRA ========");
			file.close();
		}
	}
	File file = SPIFFS.open("/datalog.txt", FILE_APPEND);
	if(file){
		file.println(evento);
		file.close();
	}
}
