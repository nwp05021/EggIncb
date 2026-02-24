#include "RelayHal.h"
#include <Arduino.h>

void RelayHal::begin(int heaterPin, int motorPin, int fanPin, int humidPin, bool activeHigh) {
  _heaterPin=heaterPin; _motorPin=motorPin; _fanPin=fanPin; _humidPin=humidPin;
  _activeHigh=activeHigh;

  pinMode(_heaterPin, OUTPUT);
  pinMode(_motorPin, OUTPUT);
  pinMode(_fanPin, OUTPUT);
  pinMode(_humidPin, OUTPUT);

  setHeater(false);
  setMotor(false);
  setFan(false);
  setHumidifier(false);
}

void RelayHal::writePin(int pin, bool on) {
  if (pin < 0) return;
  bool level = _activeHigh ? on : !on;
  digitalWrite(pin, level ? HIGH : LOW);
}

void RelayHal::setHeater(bool on){ _heater=on; writePin(_heaterPin,on); }
void RelayHal::setMotor(bool on){ _motor=on; writePin(_motorPin,on); }
void RelayHal::setFan(bool on){ _fan=on; writePin(_fanPin,on); }
void RelayHal::setHumidifier(bool on){ _humid=on; writePin(_humidPin,on); }
