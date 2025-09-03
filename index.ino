/* 
Participantes:
Rafael Ali Oliveira Khalil 561240
Bruno Lisboa Rezende 562228
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// --- Definições do DHT22 ---
#define DHTPIN 15      // Pino GPIO15 conectado ao DHT22
#define DHTTYPE DHT22  // Tipo de sensor: DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- Definições do LDR ---
#define LDR_PIN 34 // Pino GPIO34 (entrada analógica) para o LDR

// --- Credenciais WiFi ---
const char* ssid = "Wokwi-GUEST";   // Nome da rede Wi-Fi
const char* password = "";          // Senha da rede Wi-Fi

// --- Configuração do ThingSpeak ---
const char* apiKey = "8JHTITC5CKIDA6P0"; // Write API Key do canal
const char* server = "http://api.thingspeak.com";

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(LDR_PIN, INPUT);

  // Conexão ao Wi-Fi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    // --- Leitura dos sensores ---
    float t = NAN;
    float h = NAN;

    // Tenta ler o DHT22 até 5 vezes se der erro
    int tentativas = 0;
    while ((isnan(t) || isnan(h)) && tentativas < 5) {
      h = dht.readHumidity();
      t = dht.readTemperature();
      Serial.println(h);
      if (!isnan(t) && !isnan(h)) break;
      tentativas++;
      delay(2000); // espera antes de tentar novamente
    }

    if (isnan(t) || isnan(h)) {
      Serial.println("Falha ao ler o DHT22!");
    } else {

      // Leitura do LDR e normalização
      int ldrValue = analogRead(LDR_PIN); 
      float luminosity = (ldrValue / 4095.0) * 100.0; // % de luminosidade

      // --- Debug no Serial ---
      Serial.println("=== Dados Coletados ===");
      Serial.print("Temperatura: ");
      Serial.print(t);
      Serial.println(" °C");

      Serial.print("Umidade: ");
      Serial.print(h);
      Serial.println(" %");

      Serial.print("Luminosidade: ");
      Serial.print(luminosity);
      Serial.println(" %");
      Serial.println("========================");

      // --- Envio ao ThingSpeak ---
      HTTPClient http;
      String url = String(server) + "/update?api_key=" + apiKey +
                   "&field1=" + String(t, 2) +      // 2 casas decimais
                   "&field2=" + String(h, 2) +
                   "&field3=" + String(luminosity, 2);

      http.begin(url);
      int httpCode = http.GET();

      if (httpCode > 0) {
        Serial.println("Dados enviados ao ThingSpeak!");
        Serial.print("Código HTTP: ");
        Serial.println(httpCode);
      } else {
        Serial.print("Erro ao enviar dados. Código HTTP: ");
        Serial.println(httpCode);
      }
      http.end();
    }

  } else {
    Serial.println("WiFi desconectado. Tentando reconectar...");
    WiFi.begin(ssid, password);
  }

  // Intervalo
  delay(15000);
}
