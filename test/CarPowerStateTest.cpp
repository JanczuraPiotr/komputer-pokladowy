//
// Created by piotr@janczura.pl on 2024.07.17
//

#include "CarPowerStateTest.hpp"

namespace test {

TEST_F(CarPowerStateTest, interiorLightingOn)
{
    FAIL() << "Rozpoznaj włączenie oświetlenia. "
              "Oświetlenie jest włączone zaraz po otwarciu samochodu kluczykiem i po wyłączeniu silnika. "
              "Długość świecenia nie jest określona i zależy np od tego czy drzwi są otwarte czy nie."
              "\nTylko włączone radia.";
}

TEST_F(CarPowerStateTest, socket12VOn)
{
    FAIL() << "Rozpoznaj włączenie zasilania pojazdu. "
              "Zasilanie pojazdu włączane jest przez przekręcenie kluczyka w stacyjce ale ograniczone jest "
              "prawdopodobnie przez system zarządzania energią."
              "\nTylko włączone radia.";
}

TEST_F(CarPowerStateTest, socket12VOnAndInteriorLightOn)
{
    FAIL() << "Oświetlenie kabiny jest włączone jakiś czas po otwarciu pojazdu i po wyłączeniu silnika, "
              "równolegle z zasilaniem w gnieździe zapalniczki."
              "\nTylko włączone radia.";
}

TEST_F(CarPowerStateTest, socket12VOnOrInteriorLightOn)
{
    FAIL() << "Włączone jest oświetlenie kabiny lub jest napięcie w socket12V."
              "\nTylko włączone radia.";
}

TEST_F(CarPowerStateTest, socket12VOffAndInteriorLightOff)
{
    FAIL() << "Nie zależnie od poprzedniego stanu należy wyłączyć ładowanie akumulatora i zasilania radiostacji. "
              "Chyba że inaczej zdecydują przełącziki na konsoli - osobny obiekt zarządzający i osobne testy.";
}

TEST_F(CarPowerStateTest, engineOn)
{
    FAIL() << "Uruchomiono silnik. Czyli wzrosło napięcie w socket12V i zgasło oświetlenie w kabinie."
              "\nWłącz ładowanie wewnętrznego akumulatora i włączone radia. (Radia powinne być włączone wcześniej)";
}

TEST_F(CarPowerStateTest, engineOff)
{
    FAIL() << "Wyłączono silnik. Czyli spadło napięcie w socket12V i zaświeciło się światło w kabinie. "
              "\nWyłącz ładowanie wewnętrznego akumulatora ale radia pozostaw włączone.";
}

TEST_F(CarPowerStateTest, socket12VOnAndCarLightOff)
{
    FAIL() << "Oświetlenie kabiny zgasło automatycznie. Zasilanie wewnątrz pojazdu jest włączone. "
              "Wystepuje po jakimś czasie po włączeniu zasilania i ale gdy silnik pozostaje nie uruchomiony. "
              "\nTaka sama sytuacja jak przy uruchomieniu silnika ale napięcie w socket12V nie wzrosło."
              "\nRadia włączone ale nia ładować dodatkowego akumulatora.";
}

TEST_F(CarPowerStateTest, shortSocket12VOff)
{
    FAIL() << "Krótki zanik zasilania pojazdu. Prawdopodobnie rozruch silnika. Jak rozpoznać? Może oświetlenie kabiny nie gaśnie?";
}


}
