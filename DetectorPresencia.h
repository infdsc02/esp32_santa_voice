#pragma once
/*
  DetectorPresencia: la interfaz común (el "Target" del patrón Adapter).

  El programa principal SOLO conoce esta clase. Cada sensor concreto
  (RCWL-0516, PIR, botón, otro radar...) tiene su propio adaptador que
  traduce su forma de funcionar a estos métodos.

  Para añadir un sensor nuevo:
    1. Crea "AdaptadorLoQueSea.h" con una clase que herede de DetectorPresencia.
    2. Implementa iniciar(), presenciaDetectada() y nombre()
       (y si quieres estabilizado() y actualizar()).
    3. En papa_noel_adapter.ino cambia UNA línea: la que crea el detector.
*/

class DetectorPresencia {
public:
  virtual ~DetectorPresencia() {}

  // Configura pines, buses, etc. Se llama una vez en setup().
  virtual void iniciar() = 0;

  // Se llama en cada vuelta de loop(). Útil para sensores que necesitan
  // leer un bus (UART/I2C) o filtrar la señal. Por defecto no hace nada.
  virtual void actualizar() {}

  // true cuando el sensor ya puede dar lecturas fiables
  // (el PIR, por ejemplo, necesita ~1 minuto tras encenderse).
  virtual bool estabilizado() const { return true; }

  // true si ahora mismo hay alguien delante.
  virtual bool presenciaDetectada() = 0;

  // Nombre para mostrar en el Monitor Serie.
  virtual const char* nombre() const = 0;
};
