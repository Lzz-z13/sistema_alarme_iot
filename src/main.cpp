#include <Arduino.h>
#include "AdafruitIO_WiFi.h"
#include "secrets.h"

const int LED_PIN = 14;
const int BUTTON_PIN = 27;

AdafruitIO_WiFi io(
    IO_USERNAME,
    IO_KEY,
    WIFI_SSID,
    WIFI_PASS);

AdafruitIO_Feed *feedbotaoled = io.feed("botaoled");

bool estadoLed = false;

// Última leitura realizada pelo botão
byte ultimaLeituraBotao = HIGH;

byte estadoBotaoEstavel = HIGH;

// momento da última alteração detectada
unsigned long ultimaMudancaBotao = 0;

// tempo utilizado para eliminar o rúido mêcanicos do botão
const unsigned long TEMPO_DEBOUNCE = 50;

void handleBotaoLed(AdafruitIO_Data *data);

void atualizarLed();

void publicarEstadoLed();

void verificarBotao();

void setup()
{
  Serial.begin(115200);
  io.connect();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.println("");
  Serial.println("Iniciand a ESP...");

  feedbotaoled->onMessage(handleBotaoLed);

  io.connect();

  while (io.status() < AIO_CONNECTED)
  {
    Serial.println(".");
    delay(500);
  }

  Serial.println();

  Serial.println(io.statusText());

  Serial.println("Adafruit IO Conectado!");

  feedbotaoled->get();
}

void loop()
{
  io.run();

  verificarBotao();
}

void handleBotaoLed(AdafruitIO_Data *data)
{
  String valorRecebido = data->toString();

  valorRecebido.trim();

  valorRecebido.toLowerCase();

  Serial.println("Valor recebido do Adafruit IO: ");
  Serial.println(valorRecebido);

  if (valorRecebido == "true")
  {
    estadoLed = true;
    atualizarLed();
    Serial.println("Comando Remoto: LED LIGADO");
  }
  else if (valorRecebido == "false")
  {
    estadoLed = false;
    atualizarLed();
    Serial.println("Comando Remoto: LED DESLIGADO");
  }
  else
  {
    Serial.println("Valor recebido não reconhecido!");
  }
}

void atualizarLed()
{
  digitalWrite(LED_PIN, estadoLed ? HIGH : LOW);
}

void publicarEstadoLed()
{
  if (estadoLed)
  {
    feedbotaoled->save("true");
    Serial.println("Enviado ao Adafruit IO: true");
  } else{
    feedbotaoled -> save("false");
    Serial.println("Enviado ao Adafruit IO: false");
  }
}

void verificarBotao(){
  byte leituraAtual = digitalRead(BUTTON_PIN);
  if(leituraAtual != ultimaLeituraBotao){
    ultimaMudancaBotao = millis();
    ultimaLeituraBotao = leituraAtual;
  }

  if(millis() - ultimaMudancaBotao > TEMPO_DEBOUNCE){
    if(leituraAtual != estadoBotaoEstavel){
      estadoBotaoEstavel = leituraAtual;
      if(estadoBotaoEstavel == LOW){
        Serial.println();
        Serial.println("Botão físico pressionado");
        estadoLed = !estadoLed;
        atualizarLed();

        if(estadoLed){
          Serial.println("LED ligado!");
        } else {
          Serial.println("LED desligado!");
        }
        publicarEstadoLed();
      }
    }
  }
}