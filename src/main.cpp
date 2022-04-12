#include <Arduino.h>

const int SUPPLY_LONG = 14;               // Zasilanie na dłuższym przewodzie
const int SUPPLY_SHORT = 15;              // Zasilane na krutszym przewodzie
const int SUPPLY_RADIO = 16;              // Zasilanie radia
const int BATERY_CHARGING = 17;           // Ładowanie baterii
const int BATTERY_VOLTAGE = 18;           // Napięcie baterii
const int CAR_VOLTAGE = 19;               // Napięcie w samochodzie
const double CAR_OFF_VOLTAGE = 10.0;      // Napięcie poniżej którego uznajemy że samochód został wyłączony
const double CAR_ON_VOLTAGE = 11.0;       // Napięcie powyżej którego uznajemy że samochód został włączony.
const double CAR_CHARGING_VOLTAGE = 12.30;// Napięcie w samochodzie powyżej którego uznajemy że jest ładowanie.
const double CAR_LOW_VOLTAGE = 12.0





    0;
// Napięcie w instalacji samochodu po niżej którego dołączamy baterię.
// powinno być 25/1023.0 ale zmiana wartości pozwala wykalibrować pomiar.
const double FACTOR = 23.5 / 1023.0;
const int SECONDS_WITHOUT_SUPPLY = 10;
const int SECONDS_WITHOUT_CHARGING = 10;

enum class State {
  SLEEP, // 0 Auto jest wyłączone dłużej niż SECONDS_WITHOUT_SUPPLY
  CAR_OFF, // 1 Poprzedni stan to CAR_ON
  CAR_ON, // 2
  ENGINE_OFF, // 3 Poprzedni stan to ENGINE_ON
  ENGINE_ON // 4 Poprzedni stan to CAR_ON
};

State prevState;
State currentState;

int relayNr;
int secondsCarOff;
int secondsEngineOff;
double batteryVoltage;
double carVoltage;

bool servedCarOff;
bool servedCarOn;
bool servedEngineStart;
bool servedEngineStop;


void carOff();
void carOn();
void engineOff();
void engineOn();
State recognitionState();
bool supplyLong;
bool supplyShort;
bool supplyRadio;
bool batteryCharge;

void setup()
{
  supplyLong = false;
  supplyShort = false;
  supplyRadio = false;
  batteryCharge = false;
  secondsCarOff = 0;
  secondsEngineOff = 0;
  prevState = State::CAR_OFF;
  currentState = State::CAR_OFF;
  pinMode(SUPPLY_LONG, OUTPUT);
  pinMode(SUPPLY_SHORT, OUTPUT);
  pinMode(SUPPLY_RADIO, OUTPUT);
  pinMode(BATERY_CHARGING, OUTPUT);
  digitalWrite(SUPPLY_LONG, HIGH);
  digitalWrite(SUPPLY_SHORT, HIGH);
  digitalWrite(SUPPLY_RADIO, HIGH);
  digitalWrite(BATERY_CHARGING, HIGH);

  pinMode(BATTERY_VOLTAGE, INPUT);
  pinMode(CAR_VOLTAGE, INPUT);

  Serial.begin(9600);

}


void loop()
{
  batteryVoltage = analogRead(BATTERY_VOLTAGE) * FACTOR;
  carVoltage = analogRead(CAR_VOLTAGE) * FACTOR;

  if (recognitionState() != prevState) {
    switch (currentState) {
      case State::CAR_OFF:
        carOff();
        break;
      case State::CAR_ON:
        carOn();
        break;
      case State::ENGINE_OFF:
        engineOff();
        break;
      case State::ENGINE_ON:
        engineOn();
        break;
    }
  }

 if (currentState != State::CAR_OFF) {
  Serial.print("Bateria : ");
  Serial.print(batteryVoltage);
  Serial.print(" Samochod : ");
  Serial.print(carVoltage);
  Serial.print(" State : ");
  Serial.print((int)currentState );
  Serial.print(" / ");
  Serial.print((int)prevState );
  Serial.print(" przekazniki : ");
  Serial.print(supplyLong);
  Serial.print(" / ");
  Serial.print(supplyShort);
  Serial.print(" / ");
  Serial.print(supplyRadio);
  Serial.print(" / ");
  Serial.println(batteryCharge);
  Serial.println(secondsCarOff);
 }

  delay(1000);
}

State recognitionState()
{
  if (carVoltage < CAR_OFF_VOLTAGE && currentState != State::CAR_OFF ) {
    currentState = State::CAR_OFF;
    secondsCarOff = 0;
  } else if (carVoltage > CAR_CHARGING_VOLTAGE && currentState != State::ENGINE_OFF) {
    currentState = State::ENGINE_ON;
    secondsEngineOff = 0;
  } else if(carVoltage > CAR_ON_VOLTAGE && carVoltage < CAR_LOW_VOLTAGE) {
    if (currentState == State::ENGINE_ON) {
      currentState = State::ENGINE_OFF;
    } else {
      if (currentState != State::CAR_ON) {
        currentState = State::CAR_ON;
      }

    }
  }
  return currentState;
}

void carSleep()
{
// digitalWrite(SUPPLY_LONG, HIGH);
// digitalWrite(SUPPLY_SHORT, HIGH);
// digitalWrite(SUPPLY_RADIO, HIGH);
// digitalWrite(BATERY_CHARGING, HIGH);
// supplyLong = false;
// supplyShort = false;
// supplyRadio = false;
// batteryCharge = false;
// prevState = currentState;
// secondsEngineOff = 0;
// secondsCarOff = 0;
}

void carOff()
{
  secondsCarOff++;
  if (secondsCarOff > SECONDS_WITHOUT_SUPPLY) {
    digitalWrite(SUPPLY_LONG, HIGH);
    digitalWrite(SUPPLY_SHORT, HIGH);
    digitalWrite(SUPPLY_RADIO, HIGH);
    digitalWrite(BATERY_CHARGING, HIGH);
    supplyLong = false;
    supplyShort = false;
    supplyRadio = false;
    batteryCharge = false;
    prevState = currentState;
  }
}

void carOn()
{
// secondsCarOff = 0;
  digitalWrite(SUPPLY_LONG, LOW);
  digitalWrite(SUPPLY_SHORT, LOW);
  digitalWrite(SUPPLY_RADIO, LOW);
  digitalWrite(BATERY_CHARGING, HIGH);
  prevState = currentState;
  supplyLong = true;
  supplyShort = true;
  supplyRadio = true;
  batteryCharge = false;
}

void engineOff()
{
  secondsEngineOff++;
  if (secondsEngineOff > SECONDS_WITHOUT_CHARGING) {
    digitalWrite(SUPPLY_LONG, LOW);
    digitalWrite(SUPPLY_SHORT, LOW);
    digitalWrite(SUPPLY_RADIO, LOW);
    digitalWrite(BATERY_CHARGING, HIGH);
    prevState = currentState;
    currentState = State::CAR_ON;
    supplyLong = true;
    supplyShort = true;
    supplyRadio = true;
    batteryCharge = false;
  }
}

void engineOn()
{
  secondsEngineOff = 0;
  secondsCarOff = 0;
  digitalWrite(SUPPLY_LONG, LOW);
  digitalWrite(SUPPLY_SHORT, LOW);
  digitalWrite(SUPPLY_RADIO, LOW);
  digitalWrite(BATERY_CHARGING, LOW);
  prevState = currentState;
  supplyLong = true;
  supplyShort = true;
  supplyRadio = true;
  batteryCharge = true;

}
