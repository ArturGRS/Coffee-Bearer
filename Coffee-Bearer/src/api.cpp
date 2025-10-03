#include "api.h"
#include "usuarios.h"

AsyncWebServer server(80);

void configurar_rotas_api() {
	server.on("/api/usuarios", HTTP_GET, [](AsyncWebServerRequest *request){
		String resp = "[";
		for (int i = 0; i < total_usuarios; i++) {
			resp += "{\"uid\":\"" + usuarios[i].uid + "\",\"nome\":\"" + usuarios[i].nome + "\",\"creditos\":" + String(usuarios[i].creditos) + "}";
			if (i < total_usuarios - 1) resp += ",";
		}
		resp += "]";
		request->send(200, "application/json", resp);
	});
}
