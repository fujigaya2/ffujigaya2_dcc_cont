//200216 fujigaya2
//200529 クラス化とか

#include "fujigaya2_dcc.h"



//CheapContriller用 190908 fujigaya2
#define SPEED_REF A2
#define BTN_DIR A3
#define BTN_F0 6
#define BTN_F1 7
#define BTN_F2 8
#define BTN_F3 11
#define BTN_F4 12

#define DCCPIN1 9
#define DCCPIN2 10

dcc_cont DCC(DCCPIN1,DCCPIN2);

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
  //Serial.begin(115200);

}

void loop()
{
  // put your main code here, to run repeatedly:
  if(!user_program())
  {
    //false時はせっせとIdlepacketを送る。
    DCC.write_idle_packet();
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
    DCC.write_Func04_packet(DECODER_ADDRESS,F0_MASK,state_btn_f0);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_F1) == 0)
  {
    state_btn_f1 = !state_btn_f1;
    DCC.write_Func04_packet(DECODER_ADDRESS,F1_MASK,state_btn_f1);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_F2) == 0)
  {
    state_btn_f2 = !state_btn_f2;
    DCC.write_Func04_packet(DECODER_ADDRESS,F2_MASK,state_btn_f2);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_F3) == 0)
  {
    state_btn_f3 = !state_btn_f3;
    DCC.write_Func04_packet(DECODER_ADDRESS,F3_MASK,state_btn_f3);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_F4) == 0)
  {
    state_btn_f4 = !state_btn_f4;
    DCC.write_Func04_packet(DECODER_ADDRESS,F4_MASK,state_btn_f4);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }
  if( digitalRead(BTN_DIR) == 0)
  {
    state_btn_dir = !state_btn_dir;
    DCC.write_speed_packet(DECODER_ADDRESS,state_btn_dir,0);
    chattering_flag = true;
    gPreviousL1 = millis();
    return true;
  }

  if( (millis() - gPreviousL2) >= 200)
  {
    unsigned int current_speed_volume = analogRead(SPEED_REF);
    current_speed_volume = current_speed_volume / 8;
    DCC.write_speed_packet(DECODER_ADDRESS,state_btn_dir,current_speed_volume);
    gPreviousL2 =  millis();
    return true;
  }
  return false;
}
