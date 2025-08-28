#include <WiFi.h>
#include <HTTPClient.h>
#include <Keypad.h>

// Protótipo da função
void enviarCodigoParaAPI(String codigo);

// === CONFIG WIFI === (CORRIGIDO)
char ssid[] = "WIFI-EDUC-NB";
char password[] = "ac7ce9ss2@educanb";

// === CONFIG LED ===
const int ledRed = 2;
const int ledGreen = 4;

// === CONIFG RELAY ===
const int pinRelay = 23;

bool fechado = true;

// === ENDPOINT DA API ===
const String apiUrl = "http://10.90.146.23:7010/api/Usuarios/VerificaQRCode/";

// === CONFIG TECLADO ===
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {5, 18, 19, 21};
byte colPins[COLS] = {32, 33, 25, 26};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String codigoDigitado = "";

void setup() {
  Serial.begin(115200);
  pinMode(ledRed, OUTPUT);
  digitalWrite(ledRed, HIGH); // LED ligado inicialmente
  pinMode(ledGreen, OUTPUT);
  digitalWrite(ledGreen, LOW); // LED desligado inicialmente
  digitalWrite(pinRelay, HIGH); // Liga o Rele
  
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

  if (fechado) {
    digitalWrite(ledRed, HIGH); // Mantém LED ligado se fechado
  } else {
    digitalWrite(ledRed, LOW); 
    digitalWrite(ledGreen, HIGH);
  }

  if (tecla) {
    Serial.print(tecla);

    if (tecla == '#') {
      if (codigoDigitado.length() > 0) {
        enviarCodigoParaAPI(codigoDigitado);
        codigoDigitado = "";
        Serial.println("\nDigite o código e pressione '#' para enviar:");
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
  static int indice = 0;  // Variável agora é estática e modificável
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nErro: WiFi desconectado!");
    return;
  }

  HTTPClient http;
  String url = apiUrl + codigo;
  
  // Verificação adicional da URL
  url.trim();
  if (url.length() == 0 || !url.startsWith("http")) {
    Serial.println("\nErro: URL inválida!");
    return;
  }

  Serial.println("\nEnviando para: " + url);
  
  if (!http.begin(url)) {
    Serial.println("\nErro: Falha ao conectar com o servidor");
    return;
  }

  http.setTimeout(5000);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST("");

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Código HTTP: " + String(httpResponseCode));
    Serial.println("Resposta: " + response);
    
    if (httpResponseCode == 200) {
      fechado = false;
      indice += 1;
      Serial.print("Índice: "); Serial.println(indice);
    }
  } else {
    Serial.println("\nErro detalhado:");
    Serial.println("Código: " + String(httpResponseCode));
    Serial.println("Mensagem: " + http.errorToString(httpResponseCode));
  }
  
  if(indice >= 2) {
    fechado = true;
    indice = 0;
    digitalWrite(ledGreen, LOW);
    Serial.println("Resetando sistema...");
  }

  http.end();
}
