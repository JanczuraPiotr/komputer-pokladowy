//
// Created by piotr@janczura.pl on 2024.07.17
//

#include "CarStateTest.hpp"

#include "../src/config.hpp"
#include "../src/CarState.cpp"

namespace test {


TEST_F(CarStateTest, enginOn)
{
    GTEST_SKIP() << "Na linii ConstantlyPowered jest napięcie sugerujące pracę silnika."
              "\nWłączone ładowanie wewnętrznego akumulatora i włączone radia.";
}

TEST_F(CarStateTest, enginOff)
{
    GTEST_SKIP() << "Na linii ConstantlyPowered jest napięcie sugerujące pracę silnika."
              "\nWłączone ładowanie wewnętrznego akumulatora i włączone radia.";
}


}
