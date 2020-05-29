//200216 fujigaya2

#define BIT_ONE_US 56
#define BIT_ZERO_US 110
#define PREAMBLE_NUM 15
#define REPEAT_PACKET 3

#define DECODER_ADDRESS 0x03

#define STEP128   0b00111111
#define F0F4MASK  0b10000000 
#define F0_MASK   0b00010000 
#define F1_MASK   0b00000001 
#define F2_MASK   0b00000010 
#define F3_MASK   0b00000100 
#define F4_MASK   0b00001000 

#define F5F8MASK  0b10111111
#define F9F12MASK 0b10101111
#define LOCO_FORWARD 0b10000000
#define LOCO_REVERSE 0b00000000

//CheapContriller用 190908 fujigaya2
#define SPEED_REF A2
#define BTN_DIR A3
#define BTN_F0 6
#define BTN_F1 7
#define BTN_F2 8
#define BTN_F3 11
#define BTN_F4 12

void setup()
{
  // put your setup code here, to run once:
  pinMode(9, OUTPUT);   // 出力に設定
  pinMode(10, OUTPUT);  // 出力に設定

  //ボタン関連
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BTN_F0, INPUT_PULLUP);
  pinMode(BTN_F1, INPUT_PULLUP);
  pinMode(BTN_F2, INPUT_PULLUP);
  pinMode(BTN_F3, INPUT_PULLUP);
  pinMode(BTN_F4, INPUT_PULLUP);
  pinMode(BTN_DIR, INPUT_PULLUP); 
  //Serial.begin(115200);

}

void loop()
{
  // put your main code here, to run repeatedly:
  if(!user_program())
  {
    //false時はせっせとIdlepacketを送る。
    write_2_packet(0xFF,0x00);
  }
}

bool user_program(void)
{
  //CheapController用 200216 fujigaya2
  //ボタンが押されたとき、digitalReadは0を返す。押さないときは1を返す。
  //変更時　true
  
  static boolean chattering_flag = false;
  static unsigned long gPreviousL1 = millis();
  static unsigned long gPreviousL2 = millis();

  static bool state_btn_f0 = false;
  static bool state_btn_f1 = false;
  static bool state_btn_f2 = false;
  static bool state_btn_f3 = false;
  static bool state_btn_f4 = false;
  static bool state_btn_dir = true; //false = reverse,true =forward

  //チャタリング回避　200ms
  if(  chattering_flag == true)
  {
    if( (millis() - gPreviousL1) >= 200 ) 
    {
      chattering_flag = false;
    }
    else
    {
      return false;
    }
  }
  
  if( digitalRead(BTN_F0) == 0)
  {
    state_btn_f0 = !state_btn_f0;
    write_Func04_packet(DECODER_ADDRESS,F0_MASK,state_btn_f0);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_F1) == 0)
  {
    state_btn_f1 = !state_btn_f1;
    write_Func04_packet(DECODER_ADDRESS,F1_MASK,state_btn_f1);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_F2) == 0)
  {
    state_btn_f2 = !state_btn_f2;
    write_Func04_packet(DECODER_ADDRESS,F2_MASK,state_btn_f2);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_F3) == 0)
  {
    state_btn_f3 = !state_btn_f3;
    write_Func04_packet(DECODER_ADDRESS,F3_MASK,state_btn_f3);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_F4) == 0)
  {
    state_btn_f4 = !state_btn_f4;
    write_Func04_packet(DECODER_ADDRESS,F4_MASK,state_btn_f4);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_DIR) == 0)
  {
    state_btn_dir = !state_btn_dir;
    write_speed_packet(DECODER_ADDRESS,state_btn_dir,0);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }

  if( (millis() - gPreviousL2) >= 200)
  {
    unsigned int current_speed_volume = analogRead(SPEED_REF);
    current_speed_volume = current_speed_volume / 8;
    write_speed_packet(DECODER_ADDRESS,state_btn_dir,current_speed_volume);
    gPreviousL2 =  millis();
    return true;
  }
  return false;
}


void write_speed_packet(unsigned int address,bool loco_direction,byte loco_speed)
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
  
  for(int i = 0;i < REPEAT_PACKET;i++)
  {
    write_3_packet((byte)address,STEP128,temp_speed);
  }
  //命令終了
  digitalWrite(LED_BUILTIN,LOW);

}

void write_Func04_packet(unsigned int address,byte function,bool on_off)
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
  for(int i = 0;i < REPEAT_PACKET;i++)
  {
    write_2_packet((byte)address,past_F0F4);
  }
  //命令終了
  digitalWrite(LED_BUILTIN,LOW);
}


void write_3_packet(byte byte_one,byte byte_two,byte byte_three)
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


void write_2_packet(byte byte_one,byte byte_two)
{
  //Idle Packet,2byte order 送信用
  write_preamble();
  write_byte(byte_one);
  write_byte(byte_two);
  write_byte(byte_one ^ byte_two);
  //packet_end_bit
  bit_one();

}

void write_preamble()
{
  for(int i = 0; i < PREAMBLE_NUM;i++)
  {
    bit_one();
  }
}

void write_byte(byte b)
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

void bit_one()
{
    PORTB |= _BV(PB1);  //digitalWrite(9, HIGH);
    PORTB &= ~_BV(PB2); //digitalWrite(10, LOW);
    delayMicroseconds(BIT_ONE_US);         
    PORTB &= ~_BV(PB1); //digitalWrite(9, LOW);   
    PORTB |= _BV(PB2);  //digitalWrite(10, HIGH);   
    delayMicroseconds(BIT_ONE_US);
}

void bit_zero()
{
    PORTB |= _BV(PB1);  //digitalWrite(9, HIGH);
    PORTB &= ~_BV(PB2); //digitalWrite(10, LOW);
    delayMicroseconds(BIT_ZERO_US);         
    PORTB &= ~_BV(PB1); //digitalWrite(9, LOW);   
    PORTB |= _BV(PB2);  //digitalWrite(10, HIGH);   
    delayMicroseconds(BIT_ZERO_US);        
}
