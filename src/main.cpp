/*
 * File: main.cpp
 * Author: Cory Torode
 * Creation Date: 03/19/2024
 * Description: Starts up the driver
 */

#include "driver.hpp"

int main()
{
    // start program parsing
    SNESControllerDriver driver;
    driver.start();
    // return result of driver
    return driver._exit;
}
