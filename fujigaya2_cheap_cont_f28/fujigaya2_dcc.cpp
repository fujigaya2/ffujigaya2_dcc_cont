//200529 fujigaya2
//DCC control

#include "fujigaya2_dcc.h"


dcc_cont::dcc_cont(uint8_t out_pin1,uint8_t out_pin2)
{
  //pin config
  pinMode(out_pin1, OUTPUT);  
  pinMode(out_pin2, OUTPUT);
}

void dcc_cont::dcc_on(bool on_off)
{
  if(on_off == true)
  {
      dcc_out = true;
  }
  else
  {
      dcc_out = false;
      bit_off();
  }
}

void dcc_cont::set_function_default(uint32_t func_value)
{
  //functionの初期値を入れる。
  //F0～F4の特殊な順番を考慮する。
  //Serial.println("set_function_default");
  uint32_t temp_f0 =  (func_value & (uint32_t)0b00001) << 4;
  //Serial.println(temp_f0,BIN);
  uint32_t temp_f1_f4 = (func_value & (uint32_t)0b11110) >> 1;
  //Serial.println(temp_f1_f4,BIN);
  uint32_t temp_f5_f28 = func_value & (uint32_t)0xffffffe0;
  //Serial.println(temp_f5_f28,BIN);
  past_func = temp_f5_f28 | temp_f1_f4 | temp_f0;
  //Serial.println(past_func,BIN);
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
  if(dcc_out == false)
  {
    return ;
  }
  //send idle packet
  raw_packet_reset();
  raw_packet_add(0xff);
  raw_packet_add(0x00);
  write_packet();
}

void dcc_cont::write_reset_packet()
{
  if(dcc_out == false)
  {
    return ;
  }
  //send reset packet
  raw_packet_reset();
  raw_packet_add(0x00);
  raw_packet_add(0x00);
  write_packet();
}


void dcc_cont::write_func_packet(unsigned int address,byte function,bool on_off)
{
  //function write
  if(dcc_out == false)
  {
    return ;
  }
 
  //命令開始
  digitalWrite(LED_BUILTIN,HIGH);
  raw_packet_reset();
  //address convert
  loco_address_convert_add(address);
  //function convert
  loco_func_convert_add(function,on_off);
  //送信
  write_packet();
  //命令終了
  digitalWrite(LED_BUILTIN,LOW);    
}

void dcc_cont::write_accessory_packet(unsigned int address,bool on_off)
{
  //accessory write
  if(dcc_out == false)
  {
    return ;
  }
  
  //命令開始
  digitalWrite(LED_BUILTIN,HIGH);
  raw_packet_reset();
  //address & on_off convert
  accessory_address_onoff_convert_add(address,on_off);
  //送信
  write_packet();
  //命令終了
  digitalWrite(LED_BUILTIN,LOW);
}

//private


void dcc_cont::loco_func_convert_add(uint8_t function_no,bool on_off)
{
  //write function
  //送信functionマップに合わせて少々特殊な順番で格納する。
  //static uint32_t past_func = 0x00000000;
  //命令開始
  digitalWrite(LED_BUILTIN,HIGH);
  //格納
  if(on_off == true)//Onの時
  {
    switch(function_no)
    {
      case 0:
        past_func |= (uint32_t)F0_MASK;
        break;
      case 1:
      case 2:
      case 3:
      case 4:
        past_func |= (uint32_t)0x01 << (function_no - 1);
        break;
      default:
        past_func |= (uint32_t)0x01 << function_no;
        break;
    }
  }
  else//on_off = falseのとき
  {
    switch(function_no)
    {
      case 0:
        past_func ^= (uint32_t)F0_MASK;
        break;
      case 1:
      case 2:
      case 3:
      case 4:
        past_func ^= (uint32_t)0x01 << (function_no - 1);
        break;
      default:
        past_func ^= (uint32_t)0x01 << function_no;
        break;
    }
  }
  //送信
  if(function_no <= 4)//f0-f4
  {
    raw_packet_add((uint8_t)past_func & F0F4MASK | F0F4ORDER);
  }
  else if(function_no <= 8)//f5-f8
  {
    raw_packet_add((uint8_t)(past_func >> 5) & F5F8MASK | F5F8ORDER);
  }
  else if(function_no <= 12)//f9-f12
  {
    raw_packet_add((uint8_t)(past_func >> 9) & F9F12MASK | F9F12ORDER);
  }
  else if(function_no <= 20)//f13-f20
  {
    raw_packet_add(F13F20ORDER);
    raw_packet_add((uint8_t)(past_func >> 13));
  }
  else//f21-f28
  {
    raw_packet_add(F21F28ORDER);
    raw_packet_add((uint8_t)(past_func >> 21));
  }
  //Serial.print(past_func,HEX);
  //Serial.print(",");
  //Serial.println((uint8_t)past_func & F0F4MASK | F0F4ORDER,HEX);
  //Serial.println(past_func,BIN);
}


void dcc_cont::accessory_address_onoff_convert_add(unsigned int address,bool on_off)
{
  //全部一緒くたに入れないといけない・・・。
  //addressは1～511以外を取らないこと！
  //addressは1からとする
  //1の時address = 1 DD = 0b00
  //2の時address = 1 DD = 0b01
  //3の時address = 1 DD = 0b10
  //4の時address = 1 DD = 0b11
  //5の時address = 2 DD = 0b00
  //とする
  uint8_t address_lower = (address / 4 + 1) & ACCESSORY_ADDRESS_MASK1;
  uint8_t address_upper = ~((address / 4 + 1) >> 6) ;
  uint8_t byte1 = ACCESSORY_ORDER | address_lower;
  uint8_t byte2 = address_upper << 4 |(address % 4) << 1;
  if(on_off == true)
  {
    byte2 |= ACCESSORY_ON;
  }
  else
  {
    byte2 |= ACCESSORY_OFF;
  }
  raw_packet_add(byte1);
  raw_packet_add(byte2);
  //Serial.print(byte1,BIN);
  //Serial.print(",");
  //Serial.println(byte2,BIN);  
}


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

uint8_t dcc_cont::write_packet()
{
  //raw_Packet送信用
  //可変送信対応
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
    //railcom対応
    bit_cutout();
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
  //Speed１は緊急停止なので、２にこっそり変えておく。
  if(temp_speed == 1)
  {
    temp_speed = 2;
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
  write_packet();

  //命令終了
  digitalWrite(LED_BUILTIN,LOW);  
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
  //atmega328P 9,10pin
  //PORTB |= _BV(PB1);  //digitalWrite(9, HIGH);
  //PORTB &= ~_BV(PB2); //digitalWrite(10, LOW);
  //32u4 6,5pin  pd7,pc6
  PORTD |= _BV(PD7);  //digitalWrite(9, HIGH);
  PORTC &= ~_BV(PC6); //digitalWrite(10, LOW);
  delayMicroseconds(bit_one_us);         
  //PORTB &= ~_BV(PB1); //digitalWrite(9, LOW);   
  //PORTB |= _BV(PB2);  //digitalWrite(10, HIGH);   
  //32u4 6,5pin  pd7,pc6
  PORTD &= ~_BV(PD7);  //digitalWrite(9, HIGH);
  PORTC |= _BV(PC6); //digitalWrite(10, LOW);
  delayMicroseconds(bit_one_us);  
}

void dcc_cont::bit_zero()
{
  //atmega328P 9,10pin
  //PORTB |= _BV(PB1);  //digitalWrite(9, HIGH);
  //PORTB &= ~_BV(PB2); //digitalWrite(10, LOW);
  //32u4 6,5pin  pd7,pc6
  PORTD |= _BV(PD7);  //digitalWrite(9, HIGH);
  PORTC &= ~_BV(PC6); //digitalWrite(10, LOW);
  delayMicroseconds(bit_zero_us);         
  //PORTB &= ~_BV(PB1); //digitalWrite(9, LOW);   
  //PORTB |= _BV(PB2);  //digitalWrite(10, HIGH);   
  //32u4 6,5pin  pd7,pc6
  PORTD &= ~_BV(PD7);  //digitalWrite(9, HIGH);
  PORTC |= _BV(PC6); //digitalWrite(10, LOW);
  delayMicroseconds(bit_zero_us);   
}

void dcc_cont::bit_cutout()
{
  //Railcom cutout
  PORTD |= _BV(PD7);  //digitalWrite(9, HIGH);
  PORTC &= ~_BV(PC6); //digitalWrite(10, LOW);  
  delayMicroseconds(bit_cutoff_start_us);
  bit_off();
  delayMicroseconds(bit_cutoff_width_us);
  PORTD &= ~_BV(PD7);  //digitalWrite(9, HIGH);
  PORTC |= _BV(PC6); //digitalWrite(10, LOW);
  delayMicroseconds(bit_cutoff_end_us);
}

void dcc_cont::bit_off()
{
  //Railcom cutout と DCC off用
  //atmega328P 9,10pin
  //PORTB |= _BV(PB1);  //digitalWrite(9, HIGH);
  //PORTB &= ~_BV(PB2); //digitalWrite(10, LOW);
  //32u4 6,5pin  pd7,pc6
  PORTD |= _BV(PD7);
  PORTC |= _BV(PC6);
}



