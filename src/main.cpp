#include <Arduino.h>
#include "AdafruitIO_WiFi.h"
#include <NewPing.h>
#include "secrets.h"

const int LED_ALARME = 13;
const int BUZZER_PIN = 27;
const int BOTAO_FISICO = 26;

// ----------------------------

// Pinos sensor ultrassônico
const int TRIG_PIN = 12;
const int ECHO_PIN = 14;

// Distância máxima de range
const int MAX_DISTANCE = 50;

NewPing sonar(
    TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

AdafruitIO_WiFi io(
    IO_USERNAME,
    IO_KEY,
    WIFI_SSID,
    WIFI_PASS);

AdafruitIO_Feed *feedbotaoled = io.feed("botaoled");

bool estadoled = false;

// última leitura realizada pelo botao
byte ultimaLeituraBotao = HIGH;

// estado consideravel estável após o debounce
byte estadobotaoestavel = HIGH;

// momento da última alteração detectada
unsigned long ultimamudancabotao = 0;

// tempo ultilizado para eliminar rúido mecanicos do botao
const unsigned long TEMPO_DEBOUNCE = 50;

void handlebotaoled(AdafruitIO_Data *data);

void atualizarled();

void publicarestadoled();

void verificarbotao();

void testeGeral();

void setup()
{
  Serial.begin(115200);

  pinMode(LED_ALARME, OUTPUT);
  digitalWrite(LED_ALARME, LOW);

  pinMode(BOTAO_FISICO, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  // Serial.println("");
  // Serial.println("Iniciando a ESP...");

  // feedbotaoled->onMessage(handlebotaoled);

  // io.connect();

  // while (io.status() < AIO_CONNECTED)
  // {
  //   Serial.print(".");
  //   delay(500);
  // }

  // Serial.println();

  // Serial.println(io.statusText());

  // Serial.println("Adafruit IO Conectado!");

  // feedbotaoled->get();

  Serial.println("Sistema de alarme iniciado...");
  delay(2000);
}

void loop()
{
  // io.run();
  // verificarbotao();

  testeGeral();
}

void testeGeral(){
  // Teste do ultrassônico
  unsigned int distancia = sonar.ping_cm();
  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // Teste do botão
  Serial.print("Estado do botão: ");
  Serial.println(digitalRead(BOTAO_FISICO));

  // Teste do LED / Buzzer
  digitalWrite(LED_ALARME, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(LED_ALARME, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  delay(500);
}

void handlebotaoled(AdafruitIO_Data *data)
{
  String valorrecebido = data->toString();

  // remove possiveis espaços eantes ou depois do texto
  valorrecebido.trim();

  // converte para minúsculo
  valorrecebido.toLowerCase();

  Serial.print("Valor recebido do Adafruit IO: ");
  Serial.println(valorrecebido);

  if (valorrecebido == "true")
  {
    estadoled = true;
    atualizarled();
    Serial.println("Comando remoto: LED LIGADO");
  }
  else if (valorrecebido == "false")
  {
    estadoled = false;
    atualizarled();
    Serial.println("Comando remoto: LED DESLIGADO");
  }
  else
  {
    Serial.println("Valor recebido não reconhecido!");
  }
}

void atualizarled()
{
  // digitalWrite(LED_PIN, estadoled ? HIGH : LOW);
}

void publicarestadoled()
{
  if (estadoled)
  {
    feedbotaoled->save(String("true"));
    Serial.println("Enviado ao Adafruit IO: true");
  }
  else
  {
    feedbotaoled->save(String("false"));
    Serial.println("Enviado ao Adafruit IO: false");
  }
}

void verificarbotao()
{
  // byte leituraatual = digitalRead(BOTAO_PIN);
  // if(leituraatual != ultimaLeituraBotao){
  //   ultimamudancabotao = millis();
  //   ultimaLeituraBotao = leituraatual;
  // }

  // if(millis() - ultimamudancabotao > TEMPO_DEBOUNCE){
  //   if(leituraatual != estadobotaoestavel){
  //     estadobotaoestavel = leituraatual;
  //     if(estadobotaoestavel == LOW){
  //       Serial.println();
  //       Serial.println("Botão físico pressionado!");
  //       estadoled = !estadoled;

  //       if(estadoled){
  //         Serial.println("LED Ligado!");
  //       }
  //       else{
  //         Serial.println("LED Desligado!");
  //       }
  //       publicarestadoled();
  //     }
  //   }
  // }
}