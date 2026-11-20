/*
  Papá Noel parlante - detección de presencia desacoplada (patrón Adapter)
  ESP32 (normal) + DFPlayer Mini + detector de presencia intercambiable + altavoz

  Archivos de este sketch (todos en la misma carpeta):
    DetectorPresencia.h   -> interfaz común que usa el programa
    AdaptadorRCWL0516.h   -> adaptador del radar RCWL-0516 (el que se usa ahora)
    AdaptadorPIR.h        -> adaptador de un PIR HC-SR501
    AdaptadorBoton.h      -> adaptador de un pulsador (para pruebas)

  Para cambiar de sensor solo hay que tocar la sección "DETECTOR" de abajo.

  Librería necesaria: "DFRobotDFPlayerMini" de DFRobot
  Placa: "ESP32 Dev Module" (core esp32 de Espressif)
*/

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

#include "DetectorPresencia.h"
#include "AdaptadorRCWL0516.h"
#include "AdaptadorPIR.h"
#include "AdaptadorBoton.h"

// ---------------- AJUSTES ----------------
const int  NUM_AUDIOS             = 8;    // nº de archivos en /mp3 (0001.mp3 ... 0012.mp3)
const int  VOLUMEN                = 25;    // 0..30
const unsigned long ESPERA_MS     = 30000; // tiempo mínimo entre reproducciones
const unsigned long TRAS_AUDIO_MS = 3000;  // ignora el detector 3 s tras acabar un audio

// ---------------- PINES ------------------
const int PIN_DETECTOR = 27;  // salida del sensor (o pulsador)
const int PIN_BUSY     = 26;  // BUSY del DFPlayer (LOW = sonando)
const int PIN_DF_RX    = 16;  // RX del ESP32  <- TX del DFPlayer
const int PIN_DF_TX    = 17;  // TX del ESP32  -> RX del DFPlayer (con resistencia 1 kΩ)
const int PIN_LED      = 2;   // LED integrado

// ---------------- DETECTOR ---------------
// Descomenta SOLO UNA línea. Es lo único que cambia al cambiar de hardware.
AdaptadorRCWL0516 adaptador(PIN_DETECTOR);
// AdaptadorPIR      adaptador(PIN_DETECTOR);
// AdaptadorBoton    adaptador(PIN_DETECTOR);

DetectorPresencia& detector = adaptador;  // a partir de aquí solo se usa la interfaz

// -----------------------------------------
HardwareSerial dfSerial(2);
DFRobotDFPlayerMini df;

unsigned long ultimaReproduccion = 0;
unsigned long finUltimoAudio = 0;
int ultimoAudio = -1;
bool listo = false;
bool sonandoAntes = false;
bool avisoEstabilizado = false;

int elegirAudio() {
  if (NUM_AUDIOS <= 1) return 1;
  int n;
  do {
    n = random(1, NUM_AUDIOS + 1);
  } while (n == ultimoAudio);  // evita repetir el mismo dos veces seguidas
  ultimoAudio = n;
  return n;
}

bool estaSonando() {
  return digitalRead(PIN_BUSY) == LOW;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUSY, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  randomSeed(esp_random());

  detector.iniciar();
  Serial.printf("Detector: %s\n", detector.nombre());

  dfSerial.begin(9600, SERIAL_8N1, PIN_DF_RX, PIN_DF_TX);
  delay(1000);  // el DFPlayer tarda en arrancar

  for (int intento = 0; intento < 5 && !listo; intento++) {
    listo = df.begin(dfSerial, /*isACK=*/true, /*doReset=*/true);
    if (!listo) {
      Serial.println("DFPlayer no responde, reintentando...");
      delay(1000);
    }
  }
  if (!listo) {
    Serial.println("ERROR: revisa cableado y tarjeta microSD.");
    return;
  }

  df.volume(VOLUMEN);
  df.EQ(DFPLAYER_EQ_BASS);
  Serial.println("Papá Noel listo. Esperando a que el detector se estabilice...");
}

void loop() {
  if (!listo) {
    digitalWrite(PIN_LED, millis() / 250 % 2);  // parpadeo rápido = error
    return;
  }

  detector.actualizar();

  // Apunta cuándo terminó el último audio
  bool sonando = estaSonando();
  if (sonandoAntes && !sonando) finUltimoAudio = millis();
  sonandoAntes = sonando;

  if (!detector.estabilizado()) {
    delay(50);
    return;
  }
  if (!avisoEstabilizado) {
    Serial.println("Detector estabilizado. Esperando presencia...");
    avisoEstabilizado = true;
  }

  bool presencia = detector.presenciaDetectada();
  digitalWrite(PIN_LED, presencia);

  bool enEspera = (ultimaReproduccion != 0) &&
                  (millis() - ultimaReproduccion < ESPERA_MS);
  bool recienAcabado = (finUltimoAudio != 0) &&
                       (millis() - finUltimoAudio < TRAS_AUDIO_MS);

  if (presencia && !sonando && !enEspera && !recienAcabado) {
    int n = elegirAudio();
    Serial.printf("[%s] Presencia: reproduciendo %04d.mp3\n", detector.nombre(), n);
    df.playMp3Folder(n);  // reproduce /mp3/000n.mp3
    ultimaReproduccion = millis();
    delay(500);  // deja que BUSY baje antes de volver a comprobar
  }

  delay(20);
}
