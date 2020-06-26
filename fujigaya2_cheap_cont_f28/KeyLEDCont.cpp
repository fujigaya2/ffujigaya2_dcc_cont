//200626 fujigaya2 

#include "KeyLEDCont.h"
//#include "ErriezTM1637.h"

KeyLEDCont::KeyLEDCont()
{
  //constructor
 tm1637_1 = new TM1637(TM1637_1_CLK_PIN, TM1637_1_DIO_PIN);
 tm1637_2 = new TM1637(TM1637_2_CLK_PIN, TM1637_2_DIO_PIN);
 function_state[0] = 1;
 function_state[1] = 0;
}

void KeyLEDCont::Init()
{
    // Initialize TM1637
  tm1637_1->begin();
  tm1637_2->begin();

  // Turn display off (All LED's off)
  tm1637_1->displayOff();
  tm1637_2->displayOff();

  // Clear display
  tm1637_1->clear();
  tm1637_2->clear();

  // Set brightness 0..7
  tm1637_1->setBrightness(1);
  tm1637_2->setBrightness(1);

  // Turn display on
  tm1637_1->displayOn();
  tm1637_2->displayOn();

  //main 基板側Init
  main_key_init();
}

void KeyLEDCont::disp_seg(uint8_t num)
{
  //キーボード送信
  switch(num)
  {
    case 0:   seg_led_emit(SEG_L,SEG_O,SEG_C,SEG_O);      break;     //loco
    case 1:      seg_led_emit(SEG_S,SEG_W,SEG_T,SEG_H);      break;      //switch
    case 2:      seg_led_emit(SEG_T,SEG_None,SEG_None,SEG_None);      break;      //t
    case 3:      seg_led_emit(SEG_C,SEG_None,SEG_None,SEG_None);      break;      //c
    case 4:      seg_led_emit(SEG_F,SEG_O,SEG_R,SEG_None);      break;  //for
    case 5:      seg_led_emit(SEG_R,SEG_E,SEG_V,SEG_None);      break;  //rev
    default:      seg_led_emit(SEG_None,SEG_None,SEG_None,SEG_None);      break;                
  }
}

void KeyLEDCont::seg_number_emit(int num)
{
  //4桁数値表示
  seg_led_emit(seg_trans_num(num / 1000 % 10),seg_trans_num(num / 100 % 10),seg_trans_num(num / 10 % 10),seg_trans_num(num % 10));
}


void KeyLEDCont::seg_number_emit2(int num)
{
  //4桁数値表示 0は表示なし
  uint8_t seg0 = 0b00000000;
  uint8_t seg1 = 0b00000000;
  uint8_t seg2 = 0b00000000;
  uint8_t seg3 = 0b00000000;
  if(num / 1000 != 0)
  {
    seg0 = seg_trans_num(num / 1000 % 10);
  }
  if(num / 100 != 0)
  {
    seg1 = seg_trans_num(num / 100 % 10);
  }
  if(num / 10 != 0)
  {
    seg2 = seg_trans_num(num / 10 % 10);
  }
  seg3 = seg_trans_num(num % 10);
  seg_led_emit(seg0,seg1,seg2,seg3);
}


int8_t KeyLEDCont::seg_trans_num(char num)
{
  //数値→SEG変換
  switch(num)
  {
    case 0: return SEG_0;
    case 1: return SEG_1;
    case 2: return SEG_2;
    case 3: return SEG_3;
    case 4: return SEG_4;
    case 5: return SEG_5;
    case 6: return SEG_6;
    case 7: return SEG_7;
    case 8: return SEG_8;
    case 9: return SEG_9;
    default: return SEG_0;
  }
}

void KeyLEDCont::seg_led_emit(uint8_t char0,uint8_t char1,uint8_t char2,uint8_t char3)
{
  //7seg 表示
  tm1637_1->writeData(0x0, char0);
  tm1637_1->writeData(0x1, char1);
  tm1637_1->writeData(0x2, char2);
  tm1637_1->writeData(0x3, char3);
}

void KeyLEDCont::ButtonLED(int num)
{
  //buttonLEDのどこを光らせるか？
  //16bitマイコンのため16bit以上のシフトはバグる！のでFunctionは16ビットごとに分けておいた。
  if(num < 16)
  {
    uint16_t temp = 0x0001 << num;
    function_state[0] ^= 0x0001 << num;
    tm1637_1->writeData(5,(function_state[0] >>  8)& 0xff);
    tm1637_1->writeData(4,(function_state[0]      )& 0xff);
  }  
  else
  {
    uint16_t temp = 0x0001 << (num - 16);
    function_state[1] ^= 0x0001 << (num - 16);
    tm1637_2->writeData(5,(function_state[1] >>  8)& 0xff);
    tm1637_2->writeData(4,(function_state[1]      )& 0xff);
  }  
}

uint8_t KeyLEDCont::getKeys()
{
  //0-31のKeyStateを返す。
  uint8_t key1  = tm1637_1->getKeys();
  uint8_t key2  = tm1637_2->getKeys();
  if(key1 != 0xff){
    return key1;
  }
  else if(key2 != 0xff){
    return key2 + 0x10;
  }
  else
  {
    return 0xff;
  }
  
}

void KeyLEDCont::main_key_init()
{
  //mainkey部分のイニシャライズ
  //pinMode(BTN_STOP, INPUT);
  pinMode(BTN_REV, INPUT);
  pinMode(BTN_LOCO, INPUT);
  pinMode(BTN_SWITCH, INPUT);
  pinMode(BTN_T, INPUT);
  pinMode(BTN_C, INPUT);
  pinMode(BTN_FOR, INPUT);
  
  //Internal PULL UP
  //digitalWrite(BTN_STOP, HIGH);
  digitalWrite(BTN_REV, HIGH);
  digitalWrite(BTN_LOCO, HIGH);
  digitalWrite(BTN_SWITCH, HIGH);
  digitalWrite(BTN_T, HIGH);
  digitalWrite(BTN_C, HIGH);
  digitalWrite(BTN_FOR, HIGH);  
}

uint8_t KeyLEDCont::get_main_key()
{
  //main基板側のキー確認用
  if(digitalRead(BTN_LOCO) == 0)
  {
    return 0;
  }
  else if(digitalRead(BTN_SWITCH) == 0)
  {
    return 1;
  }
  else if(digitalRead(BTN_T) == 0)
  {
    return 2;
  }
  else if(digitalRead(BTN_C) == 0)
  {
    return 3;
  }
  else if(digitalRead(BTN_FOR) == 0)
  {
    return 4;
  }
  else if(digitalRead(BTN_REV) == 0)
  {
    return 5;
  }
  else
  {
    return 0xff;
  }
}

int KeyLEDCont::volume_speed()
{
  return analogRead(SPEED_REF);
}
