//
// Created by cory on 3/19/24.
//

#include <thread>

#include <iostream>
#include "driver.hpp"

#include <linux/uinput.h>

#include <cstring>

#include <fcntl.h> // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <bitset>

SNESControllerDriver::SNESControllerDriver() : terminators(
        { "exit", "quit", "terminate", "close", "stop", "shutdown" } ),
                                               portname( "/dev/ttyUSB0" ),
                                               buttonMappings( new unsigned char[12]{ KEY_B, KEY_Y, KEY_E /*select*/,
                                                                                      KEY_M /*start*/, KEY_UP, KEY_DOWN,
                                                                                      KEY_LEFT,
                                                                                      KEY_RIGHT, KEY_A, KEY_X, KEY_L,
                                                                                      KEY_R
                                               } ),
                                               keys( new key[12] )
{
    // yybaaaaaabyxxabybxyaaaaaaaaaaaaaaaaa
    // INITIATE KEYBOARD OUTPUTS

    // file descriptor
    struct uinput_user_dev uidev;

    // Open uinput device
    fd = open( "/dev/uinput", O_WRONLY | O_NONBLOCK );
    if ( fd < 0 )
    {
        perror( "Unable to open /dev/uinput" );
        exit( EXIT_FAILURE );
    }

    // Set up the uinput device
    ioctl( fd, UI_SET_EVBIT, EV_KEY );
    ioctl( fd, UI_SET_EVBIT, EV_SYN );

    // Set up the keyboard events
    for ( int i = KEY_ESC; i <= KEY_RIGHT; i++ )
    {
        ioctl( fd, UI_SET_KEYBIT, i );
    }

    // Create the uinput device
    memset( &uidev, 0, sizeof( uidev ));
    snprintf( uidev.name, UINPUT_MAX_NAME_SIZE, "virtual_keyboard" );

    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;
    write( fd, &uidev, sizeof( uidev ));
    ioctl( fd, UI_DEV_CREATE);

    // INITIATE SERIAL PORT

    serial_port = open( portname, O_RDWR );
    if ( serial_port == -1 )
    {
        std::cerr << "Error opening serial port\n";
    }

    struct termios tty;
    memset( &tty, 0, sizeof( tty ));

    if ( tcgetattr( serial_port, &tty ) != 0 )
    {
        std::cerr << "Error getting serial port attributes\n";
    }

    // Set baud rate
    cfsetospeed( &tty, B9600 );
    cfsetispeed( &tty, B9600 );

    // Set other attributes (8N1)
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_iflag &= ~( IXON | IXOFF | IXANY );
    tty.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG );
    tty.c_oflag &= ~OPOST;

    // Set the new attributes
    if ( tcsetattr( serial_port, TCSANOW, &tty ) != 0 )
    {
        std::cerr << "Error setting serial port attributes\n";
    }

    for ( int i = 0; i < 12; i++ )
    {
        struct key event;

        event.code = buttonMappings[ i ];
        event.value = false;

        keys[ i ] = event;
    }
}

SNESControllerDriver::~SNESControllerDriver()
{
    // Destroy the uinput device
    ioctl( fd, UI_DEV_DESTROY);
    close( fd );

    // destroy serial port io
    close( serial_port );

    // Release all the keys if any are still being pushed.
    for ( int i = 0; i < 12; i++ )
    {
        simulate( keys[ i ] );
    }

    // memory cleanup
    delete[] buttonMappings;
    delete[] keys;
}

void SNESControllerDriver::simulate( key &key ) const
{
    struct input_event event;
    memset( &event, 0, sizeof( event ));
    event.type = EV_KEY;
    event.code = key.code;
    event.value = key.value; // Press
    write( fd, &event, sizeof( event ));

    memset( &event, 0, sizeof( event ));
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write( fd, &event, sizeof( event ));
}

void SNESControllerDriver::readSerialPort()
{
    ssize_t bytesRead;

    // Read 16 bits of data from serial port
    uint16_t data;

    bool dataBit;

    while ( running )
    {
        // read the bytes
        bytesRead = read( serial_port, &data, sizeof( data ));

        if ( bytesRead != sizeof( data ))
        {
            continue;
        }

        /* SNES input protocol sends 16 bits, but the last 4 bits are
           always 1, so we only need to read 12 out of the 16 bits. */
        for ( int i = 0; i < 12; i++ )
        {
            key &key = keys[ i ];


            // Selecting the ith bit in data, ...
            dataBit = data & ( 0b1 << i );

            // key.value represents whether to press (true) or release (false)
            // If the dataBit is different from the key.value bit,
            // (We not key.value because when true it refers to pressing,
            // but when dataBit is true it refers to releasing)
            if ( dataBit ^ !key.value )
            {
                // std::cout << "- - - - - " << std::endl;
                // std::cout << "data: " << std::bitset< 16 >( data ) << std::endl;
                // std::cout << "dataBit: " << dataBit << std::endl;
                // std::cout << "key.value: " << ( key.value ? "RELEASING" : "PRESSING" ) << std::endl;
                /* There was a change in the inputs, and we need to either
                   press or release a key now to compensate */

                // static std::string buttons[] = { "B", "Y", "Select", "Start", "Up", "Down", "Left", "Right", "A", "X","Left", "Right"};
                // Releasing a key if 1
                if ( dataBit )
                {
                    // std::cout << "Releasing " << buttons[ i ] << std::endl;
                    key.value = false;
                    simulate( key );
                    // Pressing a key if 0
                } else
                {
                    // std::cout << "Pressing " << buttons[ i ] << std::endl;
                    key.value = true;
                    simulate( key );
                }

            }
        }

    }
}

bool SNESControllerDriver::parse( std::string in )
{
    for ( const std::string_view &terminator : terminators )
    {
        if ( in == terminator )
        {
            return false;
        }
    }

    return true;
}

void SNESControllerDriver::start()
{
    std::thread input( [ & ]
                       {
                           std::string in;

                           do
                           {
                               std::cout << "> ";
                               std::getline( std::cin, in );

                           } while ( _exit == 0 && parse( in ));
                           running = false;
                       } );

    SNESControllerDriver::readSerialPort();

    input.join();
}
