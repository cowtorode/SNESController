#include "driver.hpp"

int main()
{
    // start program parsing
    SNESControllerDriver driver;
    driver.start();
    // return result of driver
    return driver._exit;
}
