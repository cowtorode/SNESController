//
// Created by cory on 3/19/24.
//

#ifndef SNESCONTROLLERDRIVER_DRIVER_HPP
#define SNESCONTROLLERDRIVER_DRIVER_HPP

#include <linux/uinput.h>

#include <string>
#include <vector>

class SNESControllerDriver
{
private:
    std::vector< std::string_view > terminators;

    /**
     * @return true if continue, false if terminate
     */
    bool parse( std::string in );

    struct key {
        __u16 code;
        __s32 value;
    };

    key* keys;

    int fd;

    unsigned char* buttonMappings;

    // simulate, unpress, and sync the input char. KEY_A, should be used, or another KEY_..., not the actual char
    void simulate( key& key ) const;

    const char* portname; // Adjust this based on Arduino Nano's port

    int serial_port;

    void readSerialPort();

    volatile bool running = true;
public:
    SNESControllerDriver();

    ~SNESControllerDriver();

    int _exit = 0;

    void start();
};


#endif //SNESCONTROLLERDRIVER_DRIVER_HPP
