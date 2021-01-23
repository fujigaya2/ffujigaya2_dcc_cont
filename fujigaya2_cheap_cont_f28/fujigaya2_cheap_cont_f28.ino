//20200626 fujigaya2

//#include "Keyboard.h"
#include "KeyLEDCont.h"
#include "fujigaya2_dcc.h"

#define WAIT_KEY  333
#define WAIT_KEY_SPD  200

//注意！fujigaya2_DCC.cpp内で固定で使っています！変更する場合は、Cpp内のbit_one(),bit_zero()内も変更すること！
#define DCCPIN1 6
#define DCCPIN2 5

KeyLEDCont KLC;
dcc_cont DCC(DCCPIN1,DCCPIN2);

static uint8_t keysLast_0 = 0;
static uint8_t keysLast_1 = 0;

//global buffer
bool state_btn_dir = true; //false = reverse,true =forward
int prev_speed = 0;  // Read keys
uint16_t function_state[2] = {1,0};

//Task Schedule
unsigned long gPreviousL1 = 0; // 250ms interval(Packet Task)

//temp_text
char aText[16 + 1];

void setup()
{
  Serial.begin(115200);
  // initialize control over the keyboard:
  //Keyboard.begin();
  
  //KeyLEDCont init
  KLC.Init();
 
  //write Loco
  KLC.seg_led_emit(SEG_L,SEG_O,SEG_C,SEG_O);

}

void loop()
{
  if(!user_program())
  {
    //false時はせっせとIdlepacketを送る。
    DCC.write_idle_packet();
  }
}

bool user_program()
{
  
  uint8_t keys_0;
  uint8_t keys_1;
  uint8_t keys_2;


  
  keys_0 = KLC.get_main_key();
  keys_1 = KLC.getKeys();


  if( (millis() - gPreviousL1) >= 200)
  {
    //speed 値の確認！
    int joy_loco_speed = KLC.volume_speed();
    //値が違う時だけ送信！ 電圧が振れる対策でabsを入れる
    if(abs(prev_speed - joy_loco_speed) > 5)
    {
      prev_speed = joy_loco_speed;
      KLC.seg_number_emit2(prev_speed / 8,state_btn_dir);
      sprintf(aText, "P%04d",prev_speed);
      //Serial.print("Speed:");
      Serial.println(aText);
      //Keyboard.print(aText);
      DCC.write_speed_packet(DECODER_ADDRESS,state_btn_dir,prev_speed / 8);
      gPreviousL1 = millis();
      return true;
    }
  } 
  
     

  if( keysLast_0 != keys_0)
  {
    //Serial.print(F("Keys_0: 0x"));
    //Serial.println(keys_0, HEX);
    if (keys_0 == 0xFF)
    {
      Serial.print(F("Press: 0x"));
      Serial.println(keysLast_0, HEX);
      KLC.disp_seg(keysLast_0);
      keyboard_send_main(keysLast_0);
    }
    keysLast_0 = keys_0;
    return true;
  }

  // Check key up
  if (keysLast_1 != keys_1)
  {
    Serial.print(F("Keys_1: 0x"));
    Serial.println(keys_1, HEX);
    if (keys_1 == 0xFF)
    {
      KLC.ButtonLED(keysLast_1);
      keyboard_send_func(keysLast_1);
      function_trans_send(keysLast_1);
    }
    keysLast_1 = keys_1;
    return true;
  }
  return false;

}

void function_trans_send(int num)
{
  if(num < 16)
  {
    uint16_t temp = 0x0001 << num;
    function_state[0] ^= 0x0001 << num;
    uint16_t judge = function_state[0] & (0x0001 << num);
    DCC.write_func_packet(DECODER_ADDRESS,num,(bool)judge);
  }  
  else
  {
    uint16_t temp = 0x0001 << (num - 16);
    function_state[1] ^= 0x0001 << (num - 16);
    uint16_t judge = function_state[1] & (0x0001 << (num - 16));
    DCC.write_func_packet(DECODER_ADDRESS,num,(bool)judge);
  }
   
}

void keyboard_send_main(uint8_t num)
{
  //Keyboard出力だが、zで方向転換のため、ちょっと拡張にしたい！
  switch(num)
  {
    case 0:  break;  //loco
    case 1:  break;  //switch
    case 2:  break;      //t
    case 3:  break;      //c
    case 4:  
      //Keyboard.press(KEY_LEFT_SHIFT);
      //Keyboard.press('z');
      //Keyboard.releaseAll();
      state_btn_dir = true;
      DCC.write_speed_packet(DECODER_ADDRESS,state_btn_dir,prev_speed / 8);
      break;  //for
    case 5:
      //Keyboard.press(KEY_LEFT_ALT);
      //Keyboard.press('z');
      //Keyboard.releaseAll();
      state_btn_dir = false;
      DCC.write_speed_packet(DECODER_ADDRESS,state_btn_dir,prev_speed / 8);
      break;  //rev
    default:      break;                
  }
}



void keyboard_send_func(uint8_t num)
{
  uint8_t sel_num = num / 10;
  uint8_t char_num = num % 10 + 0x30;
  //29-31は特殊
  switch(num)
  {
    case 29:
    case 30:
      return;
    case 31:
      //Keyboard.press(32);
      //Keyboard.releaseAll();return;
    default:
      break;
  }
  //F0-F28
  switch(sel_num)
  {
    case 0:
      //Keyboard.press(char_num);
      //Keyboard.releaseAll();
      break;
    case 1:
      //Keyboard.press(KEY_LEFT_SHIFT);
      //Keyboard.press(char_num);
      //Keyboard.releaseAll();
      break;
    case 2:
      //Keyboard.press(KEY_LEFT_ALT) ;
      //Keyboard.press(char_num);
      //Keyboard.releaseAll();
      break;
    default:
      break;
  }
}
