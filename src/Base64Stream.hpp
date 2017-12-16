#ifndef __BASE64_WRITER_HPP_
#define __BASE64_WRITER_HPP_

#include <Arduino.h>

class Base64Stream : public Stream {
  public:

    Base64Stream( Stream &p_Out ) : Output( p_Out ), Cursor( 0x00 ) {}
    ~Base64Stream( void ) { flush(); }

    void flush();
    size_t write( uint8_t u_Data );
    
    virtual int available();
    virtual int read();
    virtual int peek();

  protected:

    void Step( void );
    void Convert( void );

    uint8_t Data[ 0x04 ];
    Stream &Output;
    char Cursor;
};

#endif // __BASE64_WRITER_HPP_
