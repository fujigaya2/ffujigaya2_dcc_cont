//200216 fujigaya2
//200529 クラス化とか

#include "fujigaya2_dcc.h"
#include "fujigaya2_ds_serial_master.h"

//CheapController用 190908 fujigaya2
#define SPEED_REF A2
#define BTN_DIR A3
#define BTN_F0 2
#define BTN_F1 4
#define BTN_F2 7
#define BTN_F3 8
#define BTN_F4 9

#define DCCPIN1 6
#define DCCPIN2 5

dcc_cont DCC(DCCPIN1,DCCPIN2);
ds_serial_master ds_serial;

void setup()
{
  // put your setup code here, to run once:

  //ボタン関連
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BTN_F0, INPUT_PULLUP);
  pinMode(BTN_F1, INPUT_PULLUP);
  pinMode(BTN_F2, INPUT_PULLUP);
  pinMode(BTN_F3, INPUT_PULLUP);
  pinMode(BTN_F4, INPUT_PULLUP);
  pinMode(BTN_DIR, INPUT_PULLUP); 
  ds_serial.begin(115200);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if(!user_program())
  {
    //false時はせっせとIdlepacketを送る。
    DCC.write_idle_packet();
  }
  /*
  digitalWrite(DCCPIN1,LOW);  
  digitalWrite(DCCPIN2,HIGH);
  Serial.println("1");
  delay(2000);
  digitalWrite(DCCPIN1,HIGH);  
  digitalWrite(DCCPIN2,LOW);
  Serial.println("2");
  delay(2000);
  */    
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
    //DCC.write_func_packet(DECODER_ADDRESS,0,state_btn_f0);
    DCC.write_accessory_packet(0,true);
    ds_serial.write_func_packet(DECODER_ADDRESS,0,state_btn_f0);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_F1) == 0)
  {
    state_btn_f1 = !state_btn_f1;
    //DCC.write_func_packet(DECODER_ADDRESS,1,state_btn_f1);
    DCC.write_accessory_packet(1,true);
    ds_serial.write_accessory_packet(1,state_btn_f1);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_F2) == 0)
  {
    state_btn_f2 = !state_btn_f2;
    //DCC.write_func_packet(DECODER_ADDRESS,2,state_btn_f2);
    DCC.write_accessory_packet(2,true);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_F3) == 0)
  {
    state_btn_f3 = !state_btn_f3;
    //DCC.write_func_packet(DECODER_ADDRESS,3,state_btn_f3);
    DCC.write_accessory_packet(3,true);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_F4) == 0)
  {
    state_btn_f4 = !state_btn_f4;
    //DCC.write_func_packet(DECODER_ADDRESS,4,state_btn_f4);
    DCC.write_accessory_packet(4,true);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_DIR) == 0)
  {
    state_btn_dir = !state_btn_dir;
    //DCC.write_speed_packet(DECODER_ADDRESS,state_btn_dir,0);
    DCC.write_accessory_packet(5,true);
    ds_serial.write_direction_packet(DECODER_ADDRESS,state_btn_dir);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }

  if( (millis() - gPreviousL2) >= 200)
  {
    unsigned int current_speed_volume = analogRead(SPEED_REF);
    current_speed_volume = current_speed_volume / 8;
    DCC.write_speed_packet(DECODER_ADDRESS,state_btn_dir,current_speed_volume);
    ds_serial.write_speed_packet(DECODER_ADDRESS,current_speed_volume);
    gPreviousL2 =  millis();
    return true;
  }
  return false;
}
