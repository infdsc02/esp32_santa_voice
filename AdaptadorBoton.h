#pragma once
/*
  Adaptador para un pulsador, útil para hacer pruebas sin sensor.

  Hardware:  pulsador entre el pin indicado y GND (usa el pull-up interno).
  Devuelve "presencia" solo en el momento de pulsar, no mientras se mantiene.
*/

#include <Arduino.h>
#include "DetectorPresencia.h"

class AdaptadorBoton : public DetectorPresencia {
public:
  explicit AdaptadorBoton(int pin) : pin_(pin) {}

  void iniciar() override {
    pinMode(pin_, INPUT_PULLUP);
  }

  void actualizar() override {
    bool ahora = digitalRead(pin_) == LOW;      // pulsado = LOW
    if (ahora != estable_ && millis() - cambio_ > 30) {  // antirrebote 30 ms
      estable_ = ahora;
      cambio_ = millis();
      if (estable_) pulsacionPendiente_ = true;
    }
  }

  bool presenciaDetectada() override {
    bool p = pulsacionPendiente_;
    pulsacionPendiente_ = false;  // cada pulsación cuenta una sola vez
    return p;
  }

  const char* nombre() const override { return "Botón"; }

private:
  int pin_;
  bool estable_ = false;
  bool pulsacionPendiente_ = false;
  unsigned long cambio_ = 0;
};
