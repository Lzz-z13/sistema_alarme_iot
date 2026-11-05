#include <Arduino.h>
#include "AdafruitIO_WiFi.h"
#include <NewPing.h>
#include "secrets.h"

const int LED_ALARME = 13;
const int BUZZER_PIN = 27;
const int BOTAO_FISICO = 26;

// Pinos do sensor ultra sonico
const int TRIG_PIN = 12;
const int ECHO_PIN = 14;

// Distancia máxima
const int MAX_DISTANCE = 100;
int LIMITE_DISTANCIA = 15;

// Controle da última distância publicada
int distanciaAnteriror = -1;

NewPing sonar(
    TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

AdafruitIO_WiFi io(
    IO_USERNAME,
    IO_KEY,
    WIFI_SSID,
    WIFI_PASS);

AdafruitIO_Feed *feedbotao = io.feed("botao");
AdafruitIO_Feed *feeddistancia = io.feed("distanciaultrassom");

AdafruitIO_Feed *feedLimite =
    io.feed("limitealarme");

AdafruitIO_Feed *feedIntrusao =
    io.feed("intrusao");

// -----------------------------------
// Estado do sistema
bool alarmeAtivo = false;
unsigned long ultimaLeituraSensor = 0;
const unsigned long INTERVALO_SENSOR = 2500; // 2 segundo
// -----------------------------------

// Controle do botão
byte ultimaLeituraBotao = HIGH;
byte estadobotaoestavel = HIGH;
unsigned long ultimamudancabotao = 0;
const unsigned long TEMPO_DEBOUNCE = 250;
// -----------------------------------

bool intrusaoAnterior = false;

void verificarBotao();

void testeGeral();

void handleAlarme(AdafruitIO_Data *data);

void verificaSensor();

void ativarAlerta();

void desligarAlerta();

void handleLimite(AdafruitIO_Data *data);

void setup()
{
  Serial.begin(115200);

  pinMode(LED_ALARME, OUTPUT);
  digitalWrite(LED_ALARME, LOW);

  pinMode(BOTAO_FISICO, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);

  desligarAlerta();

  feedbotao->onMessage(handleAlarme);
  feedLimite->onMessage(handleLimite);

  Serial.println("");
  Serial.println("Iniciando a ESP...");

  io.connect();

  while (io.status() < AIO_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();

  Serial.println(io.statusText());

  Serial.println("Adafruit IO Conectado");

  feedbotao->get();

  feedLimite->get();

  Serial.println("Sistema de alarme iniciado...");
  delay(2000);
}

void loop()
{
  io.run();

  verificarBotao();
  verificaSensor();

  // --- Código de teste geral
  // testeGeral();
}

void handleAlarme(AdafruitIO_Data *data)
{
  String valor = data->toString();

  valor.trim();
  valor.toLowerCase();

  if (valor == "true")
  {
    alarmeAtivo = true;
    Serial.println("Alarme ARMADO remotamente");
  }
  else if (valor == "false")
  {
    alarmeAtivo = false;
    Serial.println("Alarme DESARMADO remotamente");
  }
  else
  {
    Serial.println("Valor inválida do FEED...");
  }
}

void handleLimite(
    AdafruitIO_Data *data)
{
  int novoLimite =
      data->toInt();

  // Aceita somente valores
  // dentro de uma faixa válida
  if (
      novoLimite >= 5 &&
      novoLimite <= 100)
  {
    LIMITE_DISTANCIA = novoLimite;

    Serial.print(
        "Novo limite: ");

    Serial.print(
        LIMITE_DISTANCIA);

    Serial.println(
        " cm");
  }
}

void verificarBotao()
{
  int leituraAtual = digitalRead(BOTAO_FISICO);

  if (leituraAtual != ultimaLeituraBotao)
  {
    ultimamudancabotao = millis();
    ultimaLeituraBotao = leituraAtual;
  }

  if (millis() - ultimamudancabotao > TEMPO_DEBOUNCE)
  {
    estadobotaoestavel = leituraAtual;
    if (estadobotaoestavel == LOW)
    {
      alarmeAtivo = !alarmeAtivo;
      feedbotao->save(String(alarmeAtivo ? "true" : "false"));
      Serial.println(alarmeAtivo ? "Alarme ATIVADO pelo botão físico" : "Alarme DESATIVADO pelo botão físico");
    }
    if (!alarmeAtivo)
    {
      desligarAlerta();
    }
  }
}

void verificaSensor()
{
  if (millis() - ultimaLeituraSensor < INTERVALO_SENSOR)
  {
    return;
  }

  ultimaLeituraSensor = millis();

  unsigned int distancia = sonar.ping_cm();

  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // Publica a distância somente quando mudar
  if (distancia > 0 && distancia != distanciaAnteriror)
  {
    feeddistancia->save(distancia);
    distanciaAnteriror = distancia;

    Serial.println("Distância enviada ao Adafruit IO.");
  }

  // Verifica se existe intrusão
  bool intrusao = alarmeAtivo &&
                  (distancia > 0 && distancia < LIMITE_DISTANCIA);

  if (intrusao != intrusaoAnterior)
  {
    feedIntrusao->save(
        intrusao ? "true" : "false");

    intrusaoAnterior = intrusao;

    Serial.println(
        intrusao
            ? "INTRUSÃO DETECTADA!"
            : "Intrusão encerrada.");
  }

  // Controla LED e buzzer
  if (intrusao)
  {
    ativarAlerta();
  }
  else
  {
    desligarAlerta();
  }
}

void ativarAlerta()
{
  digitalWrite(LED_ALARME, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
}

void desligarAlerta()
{
  digitalWrite(LED_ALARME, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

void testeGeral()
{
  // teste do ultrassônico
  unsigned int distancia = sonar.ping_cm();
  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // teste do botão
  Serial.print("Estado do botão: ");
  Serial.println(digitalRead(BOTAO_FISICO));

  // teste led / buzzer
  digitalWrite(LED_ALARME, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(LED_ALARME, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  delay(500);
}