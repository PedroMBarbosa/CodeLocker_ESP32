#include <WiFi.h>
#include <HTTPClient.h>
#include <Keypad.h>

// === CONFIG WIFI ===
const char* ssid = "WIFI-EDUC-NB";
const char* password = "ac7ce9ss2@educanb";

// === ENDPOINT DA API (ajuste IP/local conforme necessário) ===
const String apiUrl = "http://10.90.146.20:5275/api/Usuarios/VerificaQRCode/"; // ex: http://seu-ip/api/...

// === CONFIG TECLADO ===
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {5,18,19,21}; // ajuste conforme seus pinos
byte colPins[COLS] = {32,33,25,26};  // ajuste conforme seus pinos

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String codigoDigitado = "";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  Serial.print("Conectando ao WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao WiFi com sucesso!");
  Serial.println("Digite o código e pressione '#' para enviar:");
}

void loop() {
  char tecla = keypad.getKey();

  if (tecla) {
    Serial.print(tecla);

    if (tecla == '#') {
      if (codigoDigitado.length() > 0) {
        enviarCodigoParaAPI(codigoDigitado);
        codigoDigitado = "";
        Serial.println("Digite o código e pressione '#' para enviar:");
      }
    } else if (tecla == '*') {
      codigoDigitado = "";
      Serial.println("\nCódigo apagado.");
    } else {
      codigoDigitado += tecla;
    }
  }
}

void enviarCodigoParaAPI(String codigo) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = apiUrl + codigo; // Exemplo: /VerificaQRCode/7921...

    http.begin(url);
    http.setTimeout(5000); // timeout opcional

    int httpResponseCode = http.POST(""); // como é GET-like via POST com URL direta

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("\nResposta da API:");
      Serial.println(response);
    } else {
      Serial.print("\nErro na requisição HTTP: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("\nWiFi não conectado.");
  }
}
