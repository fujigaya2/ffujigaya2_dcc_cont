//200529 fujigaya2
//DCC control

#include "fujigaya2_dcc.h"


dcc_cont::dcc_cont(uint8_t out_pin1,uint8_t out_pin2)
{
  //pin config
  pinMode(out_pin1, OUTPUT);   // 出力に設定
  pinMode(out_pin2, OUTPUT);  // 出力に設定
}

void dcc_cont::set_repeat_preamble(uint8_t repeat_num)
{
  //preambleのset関数
  preamble_num = repeat_num;
}

void dcc_cont::set_repeat_packet(uint8_t repeat_num)
{
  //repeat_packet数のset関数
  repeat_packet = repeat_num;
}

void dcc_cont::set_pulse_us(uint32_t one_us,uint32_t zero_us)
{
  //pulse widthのset関数
  bit_one_us = one_us;
  bit_zero_us = zero_us;  
}

void dcc_cont::write_idle_packet()
{
  //send idle packet
  write_2_packet(0xFF,0x00);
}

void dcc_cont::write_Func04_packet(unsigned int address,byte function,bool on_off)
{
  //function f0 - f4
  //現状addressは0-127のみ受け付ける
  static byte past_F0F4 = F0F4MASK;
  //命令開始
  digitalWrite(LED_BUILTIN,HIGH);
  if(on_off == true)//Onの時
  {
    past_F0F4 |= function;
  }
  else
  {
    past_F0F4 ^= function;
  }
  for(int i = 0;i < repeat_packet;i++)
  {
    write_2_packet((byte)address,past_F0F4);
  }
  //命令終了
  digitalWrite(LED_BUILTIN,LOW);    
}

void dcc_cont::write_speed_packet(unsigned int address,bool loco_direction,byte loco_speed)
{
  //現状addressは0-127のみ受け付ける
  //loco_direction forward:true,reverse:false
  //loco_speed は2 - 127,0は停止、1は緊急停止らしいが・・・。
  //命令開始
  digitalWrite(LED_BUILTIN,HIGH);
  byte temp_speed = loco_speed;
  //上限カット
  if(temp_speed > 127)
  {
    temp_speed = 127;
  }
  
  if(loco_direction == true)
  {
    temp_speed |= LOCO_FORWARD;
  }
  else
  {
    temp_speed |= LOCO_REVERSE;
  }
  
  for(int i = 0;i < repeat_packet;i++)
  {
    write_3_packet((byte)address,STEP128,temp_speed);
  }
  //命令終了
  digitalWrite(LED_BUILTIN,LOW);  
}

void dcc_cont::write_3_packet(byte byte_one,byte byte_two,byte byte_three)
{
  //3命令送信用
  write_preamble();
  write_byte(byte_one);
  write_byte(byte_two);
  write_byte(byte_three);
  write_byte(byte_one ^ byte_two ^ byte_three);
  //packet_end_bit
  bit_one();  
}

void dcc_cont::write_2_packet(byte byte_one,byte byte_two)
{
  //Idle Packet,2byte order 送信用
  write_preamble();
  write_byte(byte_one);
  write_byte(byte_two);
  write_byte(byte_one ^ byte_two);
  //packet_end_bit
  bit_one();  
}

void dcc_cont::write_preamble()
{
  for(int i = 0; i < preamble_num;i++)
  {
    bit_one();
  }  
}

void dcc_cont::write_byte(byte b)
{
  //start bit
  bit_zero();
  //data byte
  for(int i = 0;i < 8 ;i ++)
  {
    int current_bit = (b << i) & 0x80;
    if(current_bit == 0)
    {
      bit_zero();
    }
    else
    {
      bit_one();
    }
  }  
}
void dcc_cont::bit_one()
{
    PORTB |= _BV(PB1);  //digitalWrite(9, HIGH);
    PORTB &= ~_BV(PB2); //digitalWrite(10, LOW);
    delayMicroseconds(bit_one_us);         
    PORTB &= ~_BV(PB1); //digitalWrite(9, LOW);   
    PORTB |= _BV(PB2);  //digitalWrite(10, HIGH);   
    delayMicroseconds(bit_one_us);  
}

void dcc_cont::bit_zero()
{
    PORTB |= _BV(PB1);  //digitalWrite(9, HIGH);
    PORTB &= ~_BV(PB2); //digitalWrite(10, LOW);
    delayMicroseconds(bit_zero_us);         
    PORTB &= ~_BV(PB1); //digitalWrite(9, LOW);   
    PORTB |= _BV(PB2);  //digitalWrite(10, HIGH);   
    delayMicroseconds(bit_zero_us);   
}
