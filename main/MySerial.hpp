#include <SerialStream.h>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <string>

using namespace std;
using namespace LibSerial;
    
class MySerial {
public:
  SerialStream serial_port;
  
public:
  MySerial(char* serialName) {
    serial_port.Open( serialName ) ;
    if ( ! serial_port.good() )
      {
	std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] "
		  << "Error: Could not open serial port."
		  << std::endl ;
	exit(1) ;
      }
    //
    // Set the baud rate of the serial port.
    //
    serial_port.SetBaudRate( SerialStreamBuf::BAUD_9600 ) ;
    if ( ! serial_port.good() )
      {
	std::cerr << "Error: Could not set the baud rate." <<  
	  std::endl ;
	exit(1) ;
      }
    //
    // Set the number of data bits.
    //
    serial_port.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 ) ;
    if ( ! serial_port.good() )
      {
	std::cerr << "Error: Could not set the character size." <<  
	  std::endl ;
	exit(1) ;
      }
    //
    // Disable parity.
    //
    serial_port.SetParity( SerialStreamBuf::PARITY_NONE ) ;
    if ( ! serial_port.good() )
      {
	std::cerr << "Error: Could not disable the parity." <<  
	  std::endl ;
	exit(1) ;
      }
    //
    // Set the number of stop bits.
    //
    serial_port.SetNumOfStopBits( 1 ) ;
    if ( ! serial_port.good() )
      {
	std::cerr << "Error: Could not set the number of stop bits."
		  << std::endl ;
	exit(1) ;
      }
    //
    // Turn off hardware flow control.
    //
    serial_port.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_NONE ) ;
    if ( ! serial_port.good() )
      {
	std::cerr << "Error: Could not use hardware flow control."
		  << std::endl ;
	exit(1) ;
      }
    //
    // Do not skip whitespace characters while reading from the
    // serial port.
    //
    // serial_port.unsetf( std::ios_base::skipws ) ;
    //
    // Wait for some data to be available at the serial port.
    //
    //
    // Keep reading data from serial port and print it to the screen.
    //
    // Wait for some data to be available at the serial port.
    //
    while( serial_port.rdbuf()->in_avail() == 0 )
      {
	usleep(100) ;
      } 
  }
  

  char readSerial() {
    char next_byte;
    serial_port.get(next_byte); // HERE I RECEIVE THE FIRST ANSWER
    return next_byte;
  }
 
};
