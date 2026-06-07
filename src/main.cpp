#include <Arduino.h>

struct VoltageState
{
    enum State
    {
        START,

        // Jeżeli rozpoznano ten stan, to odbiorniki mogą być podłączone, dopiero gdy jest ładowanie.
        BATTERY_TO_LOW,
        // Jeżeli rozpoznano ten stan, odbiorniki mogą być włączone tylko gdy włączony jest przełącznik
        // SWITCH_2
        BATTERY_LOW,

        // Auto jest zamknięte
        SLEEP,
        // STANDBY to stan pojazdu (Land Rover Discovery 3) po otwarciu zamka centralnego przez około 10 minut
        // bezczynności oraz przez cały czas pracy silnika i kilka minut po wyłączeniu silnika lub do zamknięcia
        // zamka centralnego.
        STANDBY,
        // Rozpoznano ładowanie, trwa ładowanie.
        CHARGING_START,
        // Zakończono ładowanie. Taki sam jak STANDBY, ale akumulator był ładowany w tym cyklu.
        CHARGING_STOP,
    };

    State currentState = START;
    State prevState = START;
};

// aIn Napięcie pojazdu.
const int CAR_VOLTAGE_PIN = A6;
// aIn Napięcie baterii. Poprawne jedynie po rozłączeniu przekaźnika RELAY_CHARGING.
const int BATTERY_VOLTAGE_PIN = A7;

// dOut Żółty -> przekaźnik -> czarny.
const int RELAY_CHARGING = 9;
// dOut Czerwony -> przekaźnik -> biały.
const int RELAY_LOW_POWER = 10;
// dOut Niebieski -> przekaźnik -> zielony.
const int RELAY_HIGH_POWER = 11;
// dOut Czarny -> przekaźnik -> brązowy.
const int RELAY_HAM = 12;

// Numery przełączników na panelu.
// dIn Pomarańczowy
const int SWITCH_1 = 8;
// dIn Żółty
const int SWITCH_2 = 7;
// dIn Zielony
const int SWITCH_3 = 6;
// dIn Niebieski
const int SWITCH_4 = 5;

const int SWITCH_PERMAMENT_ON = SWITCH_1;
const int SWITCH_HAM = SWITCH_2;
const int SWITCH_3_NOT_USED = SWITCH_3;
const int SWITCH_LOW_POWER = SWITCH_4;

const bool SWITCH_ON = LOW;
const bool SWITCH_OFF = HIGH;

const bool RELAY_ON = LOW;
const bool RELAY_OFF = HIGH;
// const bool RELAY_ON = HIGH;
// const bool RELAY_OFF = LOW;

// Napięcie powyżej, którego uznajemy, że samochód został włączony.
const double STANDBY_VOLTAGE = 2;
// Napięcie w samochodzie powyżej, którego uznajemy, że jest ładowanie.
const double CAR_CHARGING_VOLTAGE = 13.00;
const double BATTERY_LOW = 11.0;
const double BATTERY_TO_LOW = 10.5;

// Ilość kolejnych odczytów, w których stwierdzono za niskie napięcie baterii: BATTERY_LOW
// Utrzymujące się stale zbyt niskie napięcie powoduje przejście w stan BATTERY_TO_LOW, w którym konieczne jest
// pojawienie się napięcia ładowania, by mogły być podane napięcie na odbiorniki.
// Częstotliwość odczytu zależy od timeDelay.
const unsigned BATTERY_LOW_COUNT = 100;

const unsigned BATTERY_TO_LOW_COUNT = 100;

// Napięcie mierzone jest za pośrednictwem dziennika pozwalającego zredukować napięcie w zakresie 0-25V do 0-5V
// Przetwornik A/C w arduino przetwarza z rozdzielczością 10bitów.
// Należy wyznaczyć współczynnik pozwalający określić rzeczywistą wartość mierzonego napięcia przepuszczonego przez
// dziennik i przetwórnik AC.
// Factor służy do skorygowania wartości zwróconej przez przetwornik A/C do rzeczywistej wartości napięcia.
// Przy idealnych wartościach faktor powinien mieć wartość 25/1023.0.
// Wartości muszą być dobrane przy każdej zmianie arduino i dzielnika napięcia.
const double FACTOR_CAR_VOLTAGE = 25.9 / 1023.0;
const double FACTOR_BATTERY_VOLTAGE = 26.9 / 1023.0;


// TODO Zstąpić odczytem z CAN BUS

// Napięcie na przewodzie, na którym rozpoznawany jest stan STANDBY. Pojawienie się napięcia wyższego niż STANDBY_VOLTAGE
// informuje o otwarciu pojazdu. Pojawienie się napięcia wyższego niż CAR_CHARGING_VOLTAGE informuje o pracy silnika.
double carVoltage = 0.0;

double batteryVoltage = 0.0; // Napięcie na baterii. Jest napięciem własnym baterii gdy nie jest ładowana.
                             // Obecny układ nie pozwala określić stanu naładowania baterii.

int relayNr;

bool servedCarOff;
bool servedCarOn;
bool servedEngineStart;
bool servedEngineStop;

bool switchHAMAutoStart;
bool switchPermanentOn;
bool switchHighPowerOn;
bool switchLowPowerOn;

const unsigned long TIME_DELAY_SLEEP = 5000;
const unsigned long TIME_DELAY_ON = 1000;
unsigned long timeDelay = TIME_DELAY_ON;
unsigned batteryToLowCount = 0;
unsigned batteryLowCount = 0;

VoltageState voltageState(double batteryVoltage, double carVoltage);

void batteryLow();

void batteryToLow();

void carSleep(bool permanentOn, bool HAMRadioOn, bool switchNotUsedOn, bool lowPowerOn);

void standby(bool permanentOn, bool HAMRadioOn, bool switchNotUsedOn, bool lowPowerOn);

void charging(bool HAMRadioOn, bool switchNotUsedOn, bool lowPowerOn);


void relaySet(int relayNr, bool value);
bool readSwitchHAMRadio();
bool readSwitchPermanentOn();
bool readSwitchNotUsed();
bool readSwitchLowPowerOn();


void setup()
{
    pinMode(RELAY_CHARGING, OUTPUT);
    pinMode(RELAY_HAM, OUTPUT);
    pinMode(RELAY_LOW_POWER, OUTPUT);
    pinMode(RELAY_HIGH_POWER, OUTPUT);

    pinMode(SWITCH_1, INPUT);
    pinMode(SWITCH_2, INPUT);
    pinMode(SWITCH_3, INPUT);
    pinMode(SWITCH_4, INPUT);

    relaySet(RELAY_CHARGING, SWITCH_OFF);
    relaySet(RELAY_HAM, SWITCH_OFF);
    relaySet(RELAY_LOW_POWER, SWITCH_OFF);
    relaySet(RELAY_HIGH_POWER, SWITCH_OFF);

    Serial.begin(9600);
}

void loop()
{
    batteryVoltage = static_cast<double>(analogRead(BATTERY_VOLTAGE_PIN)) * FACTOR_BATTERY_VOLTAGE;
    carVoltage = static_cast<double>(analogRead(CAR_VOLTAGE_PIN)) * FACTOR_CAR_VOLTAGE;

    static bool permanent = false;
    static bool HAMRadio = false;
    static bool notUsed = false;
    static bool lowPower = false;

    bool switchPermanent = readSwitchPermanentOn();
    bool switchHAMRadio = readSwitchHAMRadio();
    bool switchNotUsed = readSwitchNotUsed();
    bool switchLowPower = readSwitchLowPowerOn();

    bool switchChanged = switchPermanent != permanent ||
                         switchHAMRadio != HAMRadio ||
                         switchNotUsed != notUsed ||
                         switchLowPower != lowPower;

    permanent = switchPermanent;
    HAMRadio = switchHAMRadio;
    notUsed = switchNotUsed;
    lowPower = switchLowPower;

    auto state = voltageState(batteryVoltage, carVoltage);
    if (state.currentState != state.prevState || switchChanged)
    {
        switch (state.currentState)
        {
        case VoltageState::State::BATTERY_TO_LOW:
            timeDelay = TIME_DELAY_SLEEP;
            batteryToLow();
            break;

        case VoltageState::State::BATTERY_LOW:
            timeDelay = TIME_DELAY_SLEEP;
            batteryLow();
            break;

        case VoltageState::State::STANDBY:
        case VoltageState::State::CHARGING_STOP:
            timeDelay = TIME_DELAY_ON;
            standby(permanent, HAMRadio, notUsed, lowPower);
            break;

        case VoltageState::State::CHARGING_START:
            timeDelay = TIME_DELAY_ON;
            charging(HAMRadio, notUsed, lowPower);
            break;

        case VoltageState::State::SLEEP:
            timeDelay = TIME_DELAY_SLEEP;
            carSleep(permanent, HAMRadio, notUsed, lowPower);
            break;

        default:
            batteryLow();

        }
    }

    if (1 || state.currentState > VoltageState::SLEEP)
    {
        if (Serial.availableForWrite())
        {
            Serial.print("Bateria : ");
            Serial.print(batteryVoltage);
            Serial.print(" Samochod : ");
            Serial.print(carVoltage);

            Serial.print(" SW_1: ");
            Serial.print(switchPermanent);

            Serial.print(" SW_2: ");
            Serial.print(switchHAMRadio);

            Serial.print(" SW_3: ");
            Serial.print(switchNotUsed);

            Serial.print(" SW_4: ");
            Serial.print(switchLowPower);

            Serial.print(" BatteryToLow: ");
            Serial.print(batteryToLowCount);

            Serial.print(" BatteryLow: ");
            Serial.print(batteryLowCount);

            Serial.print(" Stan: ");
            switch (state.currentState)
            {
                case VoltageState::State::START:
                    Serial.print(" START");
                break;

                case VoltageState::State::BATTERY_TO_LOW:
                    Serial.print(" BATTERY_TO_LOW");
                break;

                case VoltageState::State::BATTERY_LOW:
                    Serial.print(" BATTERY_LOW");
                break;

                case VoltageState::State::SLEEP:
                    Serial.print(" SLEEP");
                break;

                case VoltageState::State::STANDBY:
                    Serial.print(" STANDBY");
                break;

                case VoltageState::State::CHARGING_START:
                    Serial.print(" CHARGING_START");
                    break;

                case VoltageState::State::CHARGING_STOP:
                    Serial.print(" CHARGING_STOP");
                    break;
            }
            Serial.println("");
        }
    }

    delay(timeDelay);
}

VoltageState voltageState(double batteryVoltage, double carVoltage)
{
    static VoltageState state;
    state.prevState = state.currentState;

    if (batteryVoltage < BATTERY_LOW)
    {
        ++batteryLowCount;
        if (batteryVoltage < BATTERY_TO_LOW)
        {
            ++batteryToLowCount;
            if (batteryToLowCount > BATTERY_TO_LOW_COUNT)
            {
                state.currentState = VoltageState::State::BATTERY_TO_LOW;
            }
            else
            {
                state.currentState = VoltageState::State::BATTERY_LOW;
            }
        }
        else
        {
            batteryToLowCount > 0 ? --batteryToLowCount : batteryToLowCount = 0;
            state.currentState = VoltageState::State::BATTERY_LOW;
        }

        if (carVoltage < CAR_CHARGING_VOLTAGE)
        {
            return state;
        }
        else
        {
            batteryToLowCount = 0;
            batteryLowCount = 0;
            state.currentState = VoltageState::State::CHARGING_START;
            return state;
        }
    }
    else
    {
        batteryLowCount > 0 ? --batteryLowCount : batteryLowCount = 0;
        batteryToLowCount > 0 ? --batteryToLowCount : batteryToLowCount = 0;
    }


    if (carVoltage < STANDBY_VOLTAGE)
    {
        state.currentState = VoltageState::State::SLEEP;
        return state;
    }

    if (carVoltage > STANDBY_VOLTAGE && carVoltage < CAR_CHARGING_VOLTAGE)
    {
        if (state.prevState == VoltageState::State::CHARGING_START)
        {
            state.currentState = VoltageState::State::CHARGING_STOP;
        }
        else
        {
            if (state.prevState == VoltageState::State::BATTERY_LOW)
            {
                state.currentState = VoltageState::State::BATTERY_LOW;
            }
            else
            {
                state.currentState = VoltageState::State::STANDBY;
            }
        }
        return state;
    }

    if (carVoltage >= CAR_CHARGING_VOLTAGE)
    {
        state.currentState = VoltageState::State::CHARGING_START;
        return state;
    }

    return state;
}

void batteryLow()
{
    if (switchPermanentOn)
    {
        relaySet(RELAY_HAM, RELAY_ON);
        relaySet(RELAY_HIGH_POWER, RELAY_OFF);
        relaySet(RELAY_LOW_POWER, RELAY_OFF);
        relaySet(RELAY_CHARGING, RELAY_OFF);
    }
    else if (switchHighPowerOn)
    {
        relaySet(RELAY_HAM, RELAY_OFF);
        relaySet(RELAY_HIGH_POWER, RELAY_ON);
        relaySet(RELAY_LOW_POWER, RELAY_OFF);
        relaySet(RELAY_CHARGING, RELAY_OFF);
    }
    else if (switchLowPowerOn)
    {
        relaySet(RELAY_HAM, RELAY_OFF);
        relaySet(RELAY_HIGH_POWER, RELAY_OFF);
        relaySet(RELAY_LOW_POWER, RELAY_ON);
        relaySet(RELAY_CHARGING, RELAY_OFF);
    }
    else
    {
        relaySet(RELAY_HAM, RELAY_OFF);
        relaySet(RELAY_HIGH_POWER, RELAY_OFF);
        relaySet(RELAY_LOW_POWER, RELAY_OFF);
        relaySet(RELAY_CHARGING, RELAY_OFF);
    }
}

void batteryToLow()
{
    relaySet(RELAY_HAM, RELAY_OFF);
    relaySet(RELAY_HIGH_POWER, RELAY_OFF);
    relaySet(RELAY_LOW_POWER, RELAY_OFF);
    relaySet(RELAY_CHARGING, RELAY_OFF);
}

void carSleep( bool permanentOn, bool HAMRadioOn, bool switchNotUsedOn, bool lowPowerOn)
{
    if (HAMRadioOn && permanentOn)
    {
        relaySet(RELAY_HAM, RELAY_ON);
    }
    else
    {
        relaySet(RELAY_HAM, RELAY_OFF);
    }

    if (switchNotUsedOn && permanentOn)
    {
        relaySet(RELAY_HIGH_POWER, RELAY_ON);
    }
    else
    {
        relaySet(RELAY_HIGH_POWER, RELAY_OFF);
    }

    if (lowPowerOn && permanentOn)
    {
        relaySet(RELAY_LOW_POWER, RELAY_ON);
    }
    else
    {
        relaySet(RELAY_LOW_POWER, RELAY_OFF);
    }

    relaySet(RELAY_CHARGING, RELAY_OFF);
}

void standby( bool permanentOn, bool HAMRadioOn, bool switchNotUsedOn, bool lowPowerOn)
{
    if (HAMRadioOn)
    {
        relaySet(RELAY_HAM, RELAY_ON);
    }
    else
    {
        relaySet(RELAY_HAM, RELAY_OFF);
    }

    if (switchNotUsedOn)
    {
        relaySet(RELAY_HIGH_POWER, RELAY_ON);
    }
    else
    {
        relaySet(RELAY_HIGH_POWER, RELAY_OFF);
    }

    if (lowPowerOn)
    {
        relaySet(RELAY_LOW_POWER, RELAY_ON);
    }
    else
    {
        relaySet(RELAY_LOW_POWER, RELAY_OFF);
    }

    relaySet(RELAY_CHARGING, RELAY_OFF);
}

void charging(bool HAMRadioOn, bool switchNotUsedOn, bool lowPowerOn)
{
    if (HAMRadioOn)
    {
        relaySet(RELAY_HAM, RELAY_ON);
    }
    else
    {
        relaySet(RELAY_HAM, RELAY_OFF);
    }

    if (switchNotUsedOn)
    {
        relaySet(RELAY_HIGH_POWER, RELAY_ON);
    }
    else
    {
        relaySet(RELAY_HIGH_POWER, RELAY_OFF);
    }

    if (lowPowerOn)
    {
        relaySet(RELAY_LOW_POWER, RELAY_ON);
    }
    else
    {
        relaySet(RELAY_LOW_POWER, RELAY_OFF);
    }

    relaySet(RELAY_CHARGING, RELAY_ON);
}

void relaySet(int relayNr, bool value)
{
    digitalWrite(relayNr, value);
}

bool switchRead(int pin)
{
    return digitalRead(pin) > 0 ? SWITCH_ON : SWITCH_OFF;
}

bool readSwitchHAMRadio()
{
    return switchRead(SWITCH_HAM);
}

bool readSwitchPermanentOn()
{
    return switchRead(SWITCH_PERMAMENT_ON);
}

bool readSwitchNotUsed()
{
    return switchRead(SWITCH_3_NOT_USED);
}

bool readSwitchLowPowerOn()
{
    return switchRead(SWITCH_LOW_POWER);
}


