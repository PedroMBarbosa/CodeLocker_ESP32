#include <WiFi.h>
#include <HTTPClient.h>
#include <Keypad.h>

// Protótipo das funções
void enviarCodigoParaAPI(String codigo);
void enviarStatusSala(int status);

// === CONFIG WIFI ===
char ssid[] = "WIFI-EDUC-NB";        
char password[] = "ac7ce9ss2@educanb"; 

// === CONFIG LED ===
const int ledRed = 2;
const int ledGreen = 4;

// === CONFIG RELAY ===
const int pinRelay = 22;

bool fechado = true;
int indice = 0;

// === ENDPOINTS DA API ===
const String apiUrlVerificaQR = "http://10.90.146.23:7010/api/Usuarios/VerificaQRCode/";
const String apiUrlStatusSala = "http://10.90.146.23:7010/api/Salas/EditarStatus/1/status/";

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
  delay(500);
  pinMode(ledRed, OUTPUT);
  digitalWrite(ledRed, HIGH); 
  pinMode(ledGreen, OUTPUT);
  digitalWrite(ledGreen, LOW);
  pinMode(pinRelay, OUTPUT);
  digitalWrite(pinRelay, HIGH); 
  
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi...");
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
    delay(1500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFalha ao conectar ao WiFi!");
  } else {
    Serial.println("\nConectado ao WiFi com sucesso!");
  }
  
  Serial.println("Digite o código e pressione '#' para enviar:");
}

void loop() {
  char tecla = keypad.getKey();

  // Atualiza LEDs de acordo com o estado
  if (fechado) {
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
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
    } 
    else if (tecla == '*') {
      codigoDigitado = "";
      Serial.println("\nCódigo apagado.");
    } 
    else {
      codigoDigitado += tecla;
    }

    delay(500);
  }
  
  delay(200);
}

void enviarCodigoParaAPI(String codigo) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nErro: WiFi desconectado!");
    return;
  }

  HTTPClient http;
  String url = apiUrlVerificaQR + codigo;
  
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

  delay(10);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Código HTTP: " + String(httpResponseCode));
    Serial.println("Resposta: " + response);
    
    if (httpResponseCode == 200) {
      fechado = false;
      indice += 1;
      Serial.print("Índice: "); Serial.println(indice);
      
      // Enviar status 2 (sala aberta) quando o QR code for válido
      enviarStatusSala(2);
    }
  } else {
    Serial.println("\nErro detalhado:");
    Serial.println("Código: " + String(httpResponseCode));
    Serial.println("Mensagem: " + http.errorToString(httpResponseCode));
  }
  
  if (indice >= 2) {
    fechado = true;
    indice = 0;
    digitalWrite(ledGreen, LOW);
    Serial.println("Resetando sistema...");
    
    // Enviar status 1 (sala fechada) quando o sistema for resetado
    enviarStatusSala(1);
    delay(1000);
  }

  http.end();
}

void enviarStatusSala(int status) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nErro: WiFi desconectado! Não foi possível enviar status da sala.");
    return;
  }

  HTTPClient http;
  String url = apiUrlStatusSala + String(status);
  
  Serial.println("Enviando status da sala: " + String(status));
  Serial.println("URL: " + url);
  
  if (!http.begin(url)) {
    Serial.println("Erro: Falha ao conectar com o servidor para atualizar status da sala");
    return;
  }

  http.setTimeout(5000);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST("");

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Status sala - Código HTTP: " + String(httpResponseCode));
    Serial.println("Status sala - Resposta: " + response);
  } else {
    Serial.println("Erro ao enviar status da sala:");
    Serial.println("Código: " + String(httpResponseCode));
    Serial.println("Mensagem: " + http.errorToString(httpResponseCode));
  }

  http.end();
}
