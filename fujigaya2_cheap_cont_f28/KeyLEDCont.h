//200626 fujigaya2 

#include <Arduino.h>
#include "ErriezTM1637.h"

//tm1637
#define TM1637_1_CLK_PIN      15
#define TM1637_1_DIO_PIN      14
#define TM1637_2_CLK_PIN      16
#define TM1637_2_DIO_PIN      10

//メイン基板のPin配置
#define BTN_LOCO    2
#define BTN_SWITCH    4
#define BTN_T    7
#define BTN_C    8
#define BTN_FOR    9
#define BTN_REV   A3
#define SPEED_REF A2


// -A-
// | |
// F B
// | |
// -G-
// | |
// E C
// | |
// -D- DP
// DP G F E D C B A 

#define SEG_0 0b00111111
#define SEG_1 0b00000110
#define SEG_2 0b01011011
#define SEG_3 0b01001111
#define SEG_4 0b01100110
#define SEG_5 0b01101101
#define SEG_6 0b01111101
#define SEG_7 0b00000111
#define SEG_8 0b01111111
#define SEG_9 0b01101111

#define SEG_L 0b00111000
#define SEG_O 0b01011100
#define SEG_C 0b01011000
#define SEG_E 0b01111001
#define SEG_F 0b01110001
#define SEG_R 0b01010000
#define SEG_None 0x00
#define SEG_S 0b01101100
#define SEG_I 0b00000100
#define SEG_T 0b01111000
#define SEG_H 0b01110100
#define SEG_V 0b00111110
#define SEG_W 0b01111110


class KeyLEDCont
{
  public:
    KeyLEDCont();
    void Init();
    void disp_On_Off(boolean OnOff);
    void disp_seg(uint8_t num);
    void seg_number_emit(int num);
    void seg_number_emit2(int num,boolean dir);
    int8_t seg_trans_num(char num);
    void seg_led_emit(uint8_t char0,uint8_t char1,uint8_t char2,uint8_t char3);
    void button_led_emit(uint32_t button);
    void ButtonLED(int num);
    uint8_t getKeys();
    void main_key_init();
    uint8_t get_main_key();
    int volume_speed();

    
    TM1637* tm1637_1;
    TM1637* tm1637_2;
    
   
  private:
    uint32_t function_state_32;//初期値で０にするため！  
};
