//
// Created by piotr@janczura.pl on 2024.07.17
//

#include "CarPowerStateTest.hpp"

namespace test {

// ConstantlyPowered - Linia, na której jest napięcie 12V przez cały czas, gdy samochód jest otwarty pilotem do zamknięcia pilotem.
// Napięcie na tej linii powinno wystarczyć do rozpoznania czy silnik jest włączony.

TEST_F(CarPowerStateTest, constantlyPoweredOn)
{
    FAIL() << "Jest napięcie."
              "\nJest \"jakieś\" napięcie czyli auto jest otwarte."
              "\nTylko włączone radia.";
}

TEST_F(CarPowerStateTest, enginOn)
{
    FAIL() << "Na linii ConstantlyPowered jest napięcie sugerujące pracę silnika."
              "\nWłączone ładowanie wewnętrznego akumulatora i włączone radia.";
}


}
