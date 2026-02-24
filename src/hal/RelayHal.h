#pragma once
class RelayHal {
public:
  void begin(int heaterPin, int motorPin, int fanPin, int humidPin, bool activeHigh = true);

  void setHeater(bool on);
  void setMotor(bool on);
  void setFan(bool on);
  void setHumidifier(bool on);

  bool heater() const { return _heater; }
  bool motor() const { return _motor; }
  bool fan() const { return _fan; }
  bool humidifier() const { return _humid; }

private:
  int _heaterPin=-1, _motorPin=-1, _fanPin=-1, _humidPin=-1;
  bool _activeHigh=true;
  bool _heater=false, _motor=false, _fan=false, _humid=false;

  void writePin(int pin, bool on);
};
