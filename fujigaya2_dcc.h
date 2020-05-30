//200529 fujigaya2_dcc


#include <Arduino.h>

#define BIT_ONE_US 56
#define BIT_ZERO_US 110
#define PREAMBLE_NUM 15
#define REPEAT_PACKET 3

#define DECODER_ADDRESS 0x03

#define STEP128   0b00111111

#define F0_MASK   0b00010000 

#define F0F4MASK    0b00011111
#define F0F4ORDER   0b10000000 
#define F5F8MASK    0b00001111
#define F5F8ORDER   0b10110000
#define F9F12MASK   0b00001111
#define F9F12ORDER  0b10100000
#define F13F20ORDER 0b11011110
#define F21F28ORDER 0b11011111

#define LOCO_FORWARD 0b10000000
#define LOCO_REVERSE 0b00000000

#define RAW_PACKET_LENGTH_DAFAULT 10

#define OUTPIN1_DEFAULT 9
#define OUTPIN2_DEFAULT 10

class dcc_cont
{
  public:
    dcc_cont(uint8_t out_pin1 = OUTPIN1_DEFAULT,uint8_t out_pin2 = OUTPIN2_DEFAULT);
    virtual void write_func_packet(unsigned int address,byte function,bool on_off);
    virtual void write_speed_packet(unsigned int address,bool loco_direction,byte loco_speed);
    virtual void write_idle_packet();
    virtual void write_reset_packet()    ;
    virtual void set_repeat_preamble(uint8_t repeat_num);
    virtual void set_repeat_packet(uint8_t repeat_num);
    virtual void set_pulse_us(uint32_t one_us,uint32_t zero_us);
    
  private:
    virtual void write_preamble();
    virtual void write_byte(byte b);
    virtual void bit_one();
    virtual void bit_zero();
    virtual void raw_packet_reset();
    virtual void raw_packet_add(uint8_t value);
    virtual uint8_t write_packet();
    virtual bool loco_address_convert_add(int loco_address);
    virtual bool loco_speed_convert_add(bool loco_direction,byte loco_speed);
    virtual void loco_func_convert_add(uint8_t function_no,bool on_off);

    uint8_t preamble_num = PREAMBLE_NUM;
    uint8_t repeat_packet = REPEAT_PACKET;
    uint32_t bit_one_us = BIT_ONE_US;
    uint32_t bit_zero_us = BIT_ZERO_US;
    uint8_t raw_packet[RAW_PACKET_LENGTH_DAFAULT];
    uint8_t raw_packet_length; 
};
