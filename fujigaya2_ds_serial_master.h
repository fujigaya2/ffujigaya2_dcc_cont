//200603 fujigaya2_ds_serial_master
//desktopstation softwareの代わりにdsmainなどを制御するシリアルコマンドを送るライブラリ

#include <Arduino.h>

#define DECODER_ADDRESS_OFFSET 0xC000
#define ACC_DECORDER_ADDRESS_OFFSET 0c3800

class ds_serial_master
{
  public:
    ds_serial_cont(uint8_t out_pin1 = OUTPIN1_DEFAULT,uint8_t out_pin2 = OUTPIN2_DEFAULT);
    virtual void write_func_packet(unsigned int address,byte function,bool on_off);
    virtual void write_speed_packet(unsigned int address,bool loco_direction,byte loco_speed);
    virtual void write_accessory_packet(unsigned int address,bool on_off);
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
    virtual void accessory_address_onoff_convert_add(unsigned int address,bool on_off);

    uint8_t preamble_num = PREAMBLE_NUM;
    uint8_t repeat_packet = REPEAT_PACKET;
    uint32_t bit_one_us = BIT_ONE_US;
    uint32_t bit_zero_us = BIT_ZERO_US;
    uint8_t raw_packet[RAW_PACKET_LENGTH_DAFAULT];
    uint8_t raw_packet_length; 
};
