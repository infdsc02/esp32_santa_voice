#pragma once
/*
  Adaptador para un sensor PIR tipo HC-SR501 (o AM312, SR602...).

  Hardware:  VCC -> 5V   GND -> GND   OUT -> pin indicado (HIGH = movimiento)
  - Necesita ~1 minuto para estabilizarse tras encender (HC-SR501).
  - Ajusta en el propio sensor: puente en modo H y tiempo al mínimo.
*/

#include <Arduino.h>
#include "DetectorPresencia.h"

class AdaptadorPIR : public DetectorPresencia {
public:
  explicit AdaptadorPIR(int pinOut, unsigned long arranqueMs = 60000)
    : pin_(pinOut), arranqueMs_(arranqueMs) {}

  void iniciar() override {
    pinMode(pin_, INPUT_PULLDOWN);
    inicio_ = millis();
  }

  bool estabilizado() const override {
    return millis() - inicio_ >= arranqueMs_;
  }

  bool presenciaDetectada() override {
    return digitalRead(pin_) == HIGH;
  }

  const char* nombre() const override { return "PIR HC-SR501"; }

private:
  int pin_;
  unsigned long arranqueMs_;
  unsigned long inicio_ = 0;
};
