#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <ArduinoJson.h>

// Configuração do Wi-Fi
const char* ssid = "<SSID_DA_SUA_REDE_WIFI>";
const char* password = "<SENHA_DA_SUA_REDE_WIFI>";

// Configuração do sensor DHT
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Configuração do cliente NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Inicialização do servidor web
ESP8266WebServer server(3333);

void handleSensorRequest() {
  // Obter a data e hora atual
  time_t currentEpochTime = timeClient.getEpochTime() - 10800; //atrasando 3 horas
  
  // Formatando a data e hora no formato brasileiro
  char currentDateTime[20];
  sprintf(currentDateTime, "%02d/%02d/%04d %02d:%02d:%02d",
          day(currentEpochTime), month(currentEpochTime), year(currentEpochTime),
          hour(currentEpochTime), minute(currentEpochTime), second(currentEpochTime));

  // Leitura da temperatura e umidade do sensor DHT22
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  // Verificar se a leitura do sensor foi bem-sucedida
  if (isnan(temperatura) || isnan(umidade)) {
    server.send(500, "text/plain", "Falha na leitura do sensor DHT22");
    return;
  }

  // Criar o objeto JSON com os dados
  StaticJsonDocument<200> jsonDocument;
  jsonDocument["temperatura"] = temperatura;
  jsonDocument["umidade"] = umidade;
  jsonDocument["readAt"] = currentDateTime;

  // Serializar o JSON para uma string
  String jsonString;
  serializeJson(jsonDocument, jsonString);

  // Enviar a resposta com o JSON no corpo
  server.send(200, "application/json", jsonString);
}

void setup() {
  Serial.begin(115200);

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao Wi-Fi...");
  }
  Serial.println("Conexão Wi-Fi estabelecida");

  // Inicializar o sensor DHT
  dht.begin();

  // Configurar o cliente NTP
  timeClient.begin();

  // Definir as rotas do servidor web
  server.on("/sensores", handleSensorRequest);

  // Iniciar o servidor web
  server.begin();
  Serial.println("Servidor web iniciado");
}

void loop() {
  // Lidar com as requisições do servidor web
  server.handleClient();

  // Atualizar o cliente NTP
  timeClient.update();
}
