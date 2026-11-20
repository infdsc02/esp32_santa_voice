#pragma once
/*
  Adaptador para el radar de microondas RCWL-0516.

  Hardware:  VIN -> 5V   GND -> GND   OUT -> pin indicado (HIGH = movimiento)
             (3V3 y CDS sin conectar)
  - Salida a 3,3 V, segura para el ESP32.
  - Se estabiliza en unos segundos tras encender.
  - Mantiene OUT en HIGH ~2 s tras el último movimiento.
*/

#include <Arduino.h>
#include "DetectorPresencia.h"

class AdaptadorRCWL0516 : public DetectorPresencia {
public:
  explicit AdaptadorRCWL0516(int pinOut, unsigned long arranqueMs = 5000)
    : pin_(pinOut), arranqueMs_(arranqueMs) {}

  void iniciar() override {
    pinMode(pin_, INPUT_PULLDOWN);  // si se suelta el cable, no se dispara solo
    inicio_ = millis();
  }

  bool estabilizado() const override {
    return millis() - inicio_ >= arranqueMs_;
  }

  bool presenciaDetectada() override {
    return digitalRead(pin_) == HIGH;
  }

  const char* nombre() const override { return "Radar RCWL-0516"; }

private:
  int pin_;
  unsigned long arranqueMs_;
  unsigned long inicio_ = 0;
};
