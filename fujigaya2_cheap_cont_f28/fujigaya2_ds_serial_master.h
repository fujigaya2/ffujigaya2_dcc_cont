//200603 fujigaya2_ds_serial_master
//desktopstation softwareの代わりにdsmainなどを制御するシリアルコマンドを送るライブラリ

#include <Arduino.h>

#define DECODER_ADDRESS_OFFSET 0xC000
#define ACC_DECORDER_ADDRESS_OFFSET 0x3800
#define SEND_STR_LENGTH 30

class ds_serial_master
{
  public:
    ds_serial_master();
    virtual void begin(uint32_t baud_rate);
    virtual int write_func_packet(unsigned int address,byte function,bool on_off);
    virtual int write_speed_packet(unsigned int address,byte loco_speed);
    virtual int write_direction_packet(unsigned int address,bool loco_direction);
    virtual int write_accessory_packet(unsigned int address,bool on_off);
    virtual int write_power_packet(bool on_off);
    virtual bool read_power_status();   
    virtual int write_ping();
};
