#include <ESP8266WiFi.h>
#include <DHT.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

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

// Configuração da API
const char* apiEndpoint = "http://192.168.15.123:5555/sensores";

// Variáveis para controle do tempo de envio dos dados
unsigned long previousMillis = 0;
const unsigned long interval = 600000; // 10 minutos

// Função para enviar os dados em formato JSON para a API
void sendJSONData() {
  // Obter a data e hora atual
  time_t currentEpochTime = timeClient.getEpochTime() - 10800; // Atraso de 3 horas

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
    Serial.println("Falha na leitura do sensor DHT22");
    return;
  }

  // Criar o objeto JSON com os dados
  StaticJsonDocument<200> jsonDocument;
  jsonDocument["temperatura"] = temperatura;
  jsonDocument["umidade"] = umidade;
  jsonDocument["read_at"] = currentDateTime;

  // Serializar o JSON para uma string
  String jsonString;
  serializeJson(jsonDocument, jsonString);

  // Criar uma instância do objeto HTTPClient
  WiFiClient client;
  HTTPClient http;

  // Enviar a requisição POST com o JSON
  http.begin(client, apiEndpoint);
  http.addHeader("Content-Type", "application/json");

  // Enviar o corpo da requisição
  int httpCode = http.POST(jsonString);

  // Verificar o código de resposta
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Dados enviados com sucesso para a API");
  } else {
    Serial.println("Falha ao enviar dados para a API");
  }

  // Encerrar a conexão HTTP
  http.end();
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

  // Aguardar a sincronização do horário
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  // Enviar a primeira requisição imediatamente
  sendJSONData();

  // Aguardar 10 minutos antes da próxima requisição
  delay(600000);

  Serial.println("Configuração concluída.");
}

void loop() {
  // Verificar se passou o intervalo definido
  unsigned long currentMillis = millis();
  static unsigned long previousMillis = 0;
  const unsigned long interval = 600000; // 10 minutos

  if (currentMillis - previousMillis >= interval) {
    // Armazenar o tempo atual
    previousMillis = currentMillis;

    // Enviar os dados
    sendJSONData();
  }

  // Lidar com as requisições do cliente NTP
  timeClient.update();
}


