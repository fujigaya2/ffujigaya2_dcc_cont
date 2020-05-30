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
  raw_packet_reset();
  raw_packet_add(0xff);
  raw_packet_add(0x00);
  write_packet_auto();
}

void dcc_cont::write_reset_packet()
{
  //send reset packet
  raw_packet_reset();
  raw_packet_add(0x00);
  raw_packet_add(0x00);
  write_packet_auto();
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

//private

void dcc_cont::raw_packet_reset()
{
  for(int i = 0;i < RAW_PACKET_LENGTH_DAFAULT;i++)
  {
    raw_packet[i] = 0x00;
  }
  raw_packet_length = 0;
}

void dcc_cont::raw_packet_add(uint8_t value)
{
  raw_packet[raw_packet_length] = value;
  raw_packet_length++;
}

uint8_t dcc_cont::write_packet_auto()
{
  //raw_Packet送信用
  //可変送信対応のため
  //repet_packet回　繰り返す。
  for(int i = 0;i < repeat_packet;i++)
  {
    uint8_t checksum = 0x00;
    write_preamble();
    for(int j = 0 ;j< raw_packet_length;j++)
    {
      //write
      write_byte(raw_packet[j]);
      //CheckSum
      checksum ^= raw_packet[j];
    }
    //write checksum
    write_byte(checksum);
    //packet_end_bit
    bit_one();
  }   
}

bool dcc_cont::loco_address_convert_add(int loco_address)
{
  //loco addressを命令に変更し加える。
  if(loco_address <= 127)
  {
    //127以下
    raw_packet_add(uint8_t(loco_address));
  }
  else
  {
    //128以上
    uint8_t address_upper = ((uint16_t)loco_address >> 8) | 0b11000000;
    uint8_t address_lower = (uint16_t)loco_address & 0x00ff;
    raw_packet_add(address_upper);
    raw_packet_add(address_lower);
  }
  return true;
}

bool dcc_cont::loco_speed_convert_add(bool loco_direction,byte loco_speed)
{
  //speedとdirectionを命令に変換し加える。
  //128step限定とする。
  //上限カット
  byte temp_speed = loco_speed;  
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
  //step128 only
  raw_packet_add(STEP128);
  //speed and direction
  raw_packet_add(temp_speed);

  return true;
}

void dcc_cont::write_speed_packet(unsigned int address,bool loco_direction,byte loco_speed)
{
  //loco_direction forward:true,reverse:false
  //loco_speed は2 - 127,0は停止、1は緊急停止らしいが・・・。
  //命令開始
  digitalWrite(LED_BUILTIN,HIGH);


  raw_packet_reset();
  //address convert
  loco_address_convert_add(address);
  //speed direction
  loco_speed_convert_add(loco_direction,loco_speed);

  //send
  write_packet_auto();

  //命令終了
  digitalWrite(LED_BUILTIN,LOW);  
}

void dcc_cont::write_3_packet(byte byte_one,byte byte_two,byte byte_three)
{
  //3命令送信用
  raw_packet_reset();
  raw_packet_add(byte_one);
  raw_packet_add(byte_two);
  raw_packet_add(byte_three);
  write_packet_auto();
}

void dcc_cont::write_2_packet(byte byte_one,byte byte_two)
{
  raw_packet_reset();
  raw_packet_add(byte_one);
  raw_packet_add(byte_two);
  write_packet_auto();
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
