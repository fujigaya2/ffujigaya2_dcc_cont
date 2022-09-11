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

static uint8_t keysLast_0 = 0xFF;
static uint8_t keysLast_1 = 0xFF;
static uint8_t keysLast_onoff = 0xFF; //電源OnOff用

//出力のOnOff F31でオンとする
bool dcc_onoff_flag = false;//false:off true:on

//global buffer
//ここをBankにする。
bool state_btn_dir = false; //false = reverse,true =forward
int prev_speed = 0;  // Read keys
uint32_t function_state_32 = 0;
int loco_address = DECODER_ADDRESS;

//10こだけ覚えておくことにする。
#define LOCO_BANK_NUM 10
typedef struct
{   
  bool state_btn_dir; //false = reverse,true =forward
  int prev_speed;  // Read keys
  uint32_t function_state_32;
  int loco_address;
  bool loco_exist;//存在するかどうかのフラグ
} loco_bank_struct;
loco_bank_struct LB[LOCO_BANK_NUM];
int loco_bank_count = 0;//次に格納する番地

//表示関係
int disp_flag = 0;

//Mode関係
int mode_loco_flag = false;//false:通常,true:Loco設定
int temp_loco_num = 0;

//Task Schedule
unsigned long gPreviousL1 = 0; // 250ms interval(Packet Task)
unsigned long gPreviousL2 = 0; // 250ms interval(Packet Task)

//temp_text
char aText[16 + 1];

//idle時の送信用パケット制御用の変数 idle_send_ → is_と略す
int is_num = -1;//-1で現状Loco、0 ～ LOCO_BANK_NUMがBank内の切り替え用変数
int is_num_sub = 0;//0～4 0:speed,1func(4),1:func(8),2:func(12),3:func(20),4:func(28)とする切り替え変数。

void setup()
{
  Serial.begin(115200);
  // initialize control over the keyboard:
  //Keyboard.begin();
  
  //KeyLEDCont init
  KLC.Init();
 
  //write Off
  KLC.seg_led_emit(SEG_None,SEG_0,SEG_F,SEG_F);

  //loco_bank_reset
  loco_bank_reset();

}

void loop()
{
  if(!user_program())
  {
    //false時はせっせとIdlepacketを送る。
    //DCC.write_idle_packet();
    idle_send_program();
  }
}

void print_idle(int a,int b)
{
  Serial.print("(");
  Serial.print(a);
  Serial.print(",");
  Serial.print(b);
  Serial.println(")");
}

void idle_send_program()
{
  //アイドルパケットではなく、現状のLocoのリピートパケットや、
  //バンク内のパケットを順繰りに出す。
  //is_num(-1～9)で -1:現状と0～9(bank)の切り替え
  //is_sub_num(0～5)で　0:speed,1:func(4),2:func(8),3:func(12),4:func(20),5:func(28)とする切り替え。 
  
  //現状の変数を一時退避
  bool is_state_btn_dir = state_btn_dir;
  int is_prev_speed = prev_speed;
  uint32_t is_function_state_32 = function_state_32;
  int is_loco_address = loco_address;
  //bankの変数を現在のDCCへロード
  if(is_num != -1)
  {
    //バンクに存在するか？
    if(LB[is_num].loco_exist == true)
    {
      //０～９はバンク
      if(LB[is_num].loco_address != loco_address)
      {
        //存在して、現状ロコでない場合
        //コピー
        state_btn_dir = LB[is_num].state_btn_dir;
        prev_speed = LB[is_num].prev_speed;
        function_state_32 = LB[is_num].function_state_32;
        loco_address = LB[is_num].loco_address;
        //DCC側のファンクションもセット
        DCC.set_function_default(function_state_32);      
      }
      else
      {
        //存在するが、現状ロコと同じ場合はここ。（何もしない）
      }
    }
    else
    {
      //bankにない場合は次は現状ロコ(-1)戻す。
      //今回は現状ロコの値を流す。
      is_num = -1;
      //現状ロコの値が入っているのをそのまま使う
      //is_numは次のSwitch文でis_num_subが5になるごとにインクリメントされて
      //順繰りに進む。
    }
  }
  
  //debug
  print_idle(is_num,is_num_sub);
  //送信
  switch(is_num_sub)
  {
    case 0:
      DCC.write_speed_packet(loco_address,state_btn_dir,prev_speed / 8);
      is_num_sub ++;
      break;
    case 1:
      DCC.write_func_packet(loco_address,4);
      is_num_sub ++;
      break;
    case 2:
      DCC.write_func_packet(loco_address,8);
      is_num_sub ++;
      break;
    case 3:
      DCC.write_func_packet(loco_address,12);
      is_num_sub ++;
      break;
    case 4:
      DCC.write_func_packet(loco_address,20);
      is_num_sub ++;
      break;
    case 5:
      DCC.write_func_packet(loco_address,28);
      is_num_sub = 0;
      if(is_num < LOCO_BANK_NUM)
      {
        is_num ++;
      }
      else
      {
        is_num = 0;
      }
      break;
    default:
      //来ないはず。
      break;
  }

  //退避変数を元に戻す（-１だった場合はやらなくてもよいが）
  state_btn_dir = is_state_btn_dir;
  prev_speed = is_prev_speed;
  function_state_32 = is_function_state_32;
  loco_address = is_loco_address;
  //DCC側も現状に復旧
  DCC.set_function_default(function_state_32);

  return;
}

bool user_program()
{

  uint8_t keys_0;
  uint8_t keys_1;
  uint8_t keys_onoff;
  
  keys_0 = KLC.get_main_key();
  keys_1 = KLC.getKeys();
  keys_onoff = keys_1;
  
  //電源OnOffは一番最初に別口で確認する
  if (keysLast_onoff != keys_onoff)
  {
    if ((keys_onoff == 0xFF) && (keysLast_onoff == 31))
    {
      Serial.print(F("Press: keys_onoff"));
      Serial.println(keysLast_onoff, HEX);
      //強制オフ
      mode_loco_flag = false;
      if(dcc_onoff_flag == false)
      {
        //On
        KLC.seg_led_emit(SEG_None,SEG_None,SEG_0,SEG_N);
        delay(500);
        //初期化！
        //Function表示　on表示
        KLC.button_led_emit((uint32_t)0x80000000);        

        //loco_bank_reset
        loco_bank_reset();

        //Keyバッファリセット
        keysLast_0 = 0xFF;
        keysLast_1 = 0xFF;
        keysLast_onoff = 0xFF;

        //Locoデータをロード
        loco_bank_load(DECODER_ADDRESS);
        loco_address = DECODER_ADDRESS;
        //address表示
        KLC.seg_number_emit2(loco_address,state_btn_dir);
        //Function表示
        KLC.button_led_emit(function_state_32);
        //FunctionをDCC側にもダウンロード
        DCC.set_function_default(function_state_32);
                        
        dcc_onoff_flag = true;
      }
      else
      {
        //Off
        KLC.seg_led_emit(SEG_None,SEG_0,SEG_F,SEG_F);
        //Function表示
        KLC.button_led_emit((uint32_t)0x00000000);        
        dcc_onoff_flag = false;
      }
      DCC.dcc_on(dcc_onoff_flag);
    }
    keysLast_onoff = keys_onoff;    
    return true;
  }

  //dcc_onoff_flagがOffの時はこの後ろには進まない！
  if(dcc_onoff_flag == false)
  {
    return true;
  }

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
      DCC.write_speed_packet(loco_address,state_btn_dir,prev_speed / 8);
      return true;
    }
    gPreviousL1 = millis();
  } 
  
  if( (millis() - gPreviousL2) >= 100)
  {
    //disp blink
    if(mode_loco_flag == true)
    {
      disp_flag += 1;
    }
    else
    {
      disp_flag = 0;      
    }

    if(disp_flag < 3)
    {
      KLC.disp_On_Off(true);
    }
    else
    {
      KLC.disp_On_Off(false);
    }
    disp_flag &= 0x03; 
    gPreviousL2 = millis();     
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
      if(mode_loco_flag == false)
      {
        KLC.ButtonLED(keysLast_1);
        keyboard_send_func(keysLast_1);
        function_trans_send(keysLast_1);
      }
      else
      {
        int temp_num = temp_loco_num_add(keysLast_1);
        //KLC.ButtonLED(0x07);
        KLC.seg_number_emit2(temp_num,state_btn_dir);
      }
    }
    keysLast_1 = keys_1;
    return true;
  }
  return false;

}

void function_trans_send(int num)
{
  function_state_32 ^= (uint32_t)0x01 << num;
  uint32_t judge = function_state_32 & ((uint32_t)0x01 << num);
  DCC.write_func_packet(loco_address,num,(bool)judge);
}

void keyboard_send_main(uint8_t num)
{
  //Keyboard出力だが、zで方向転換のため、ちょっと拡張にしたい！
  switch(num)
  {
    case 0:
      //loco　address変更対応
      mode_loco_flag = ! mode_loco_flag;
      if(mode_loco_flag == true)
      {
        //Address変更モード突入
        //現状LocoDataを格納
        loco_bank_save(loco_address);
        //0-10の値だけ打ち込める旨の表示
        //合わせて、LocoBankにすでにある場合はショートカットを発動させる。
        uint32_t led_emit_temp = 0x000007ff;
        for(int loco_num = 0; loco_num < LOCO_BANK_NUM;loco_num++)
        {
          //ショートカットがある場合のLEDを点灯
          if(LB[loco_num].loco_exist == true)
          {
            //32bitを処理しようとするとなんか内部的にしくじるので、下記のように書いているだけ。
            led_emit_temp = led_emit_temp + (uint32_t)((0x00000001) << loco_num) * 65536; 
          }
        }
        //KLC.button_led_emit((uint32_t)0x000007ff);
        KLC.button_led_emit(led_emit_temp);
        //KLC.seg_led_emit2(0xff,0x07,0x00,0x00);
      }
      if(mode_loco_flag == false)
      {
        //復帰
        if ((temp_loco_num == 0) || (temp_loco_num == loco_address))
        {
          //Address 変更無し
        }
        else
        {
          //Address 変更
          loco_address = temp_loco_num;
          //Function reset
          //function_state_32 = 0;
        }
        temp_loco_num_reset();
        //Locoデータをロード
        loco_bank_load(loco_address);
        //address表示
        KLC.seg_number_emit2(loco_address,state_btn_dir);
        //Function表示
        KLC.button_led_emit(function_state_32);
        //FunctionをDCC側にもダウンロード
        uint32_t temp = function_state_32; 
        Serial.println(temp);
        DCC.set_function_default(function_state_32);
      }
      break;  
    case 1:  break;  //switch
    case 2:  break;      //t
    case 3:  break;      //c
    case 4:  
      //Keyboard.press(KEY_LEFT_SHIFT);
      //Keyboard.press('z');
      //Keyboard.releaseAll();
      state_btn_dir = true;
      DCC.write_speed_packet(loco_address,state_btn_dir,prev_speed / 8);
      break;  //for
    case 5:
      //Keyboard.press(KEY_LEFT_ALT);
      //Keyboard.press('z');
      //Keyboard.releaseAll();
      state_btn_dir = false;
      DCC.write_speed_packet(loco_address,state_btn_dir,prev_speed / 8);
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
      //DCC_OnOffする
      
    default:
      break;
  }
  //F0-F28
  switch(sel_num)
  {
    case 0:
      //Keyboard.press(char_num) ;
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

void temp_loco_num_reset()
{
  //temp番号をリセット
  temp_loco_num = 0;
}

int temp_loco_num_add(uint16_t num)
{
  //temp番号を入れていく
  if(num <= 9 )
  {
    temp_loco_num %= 1000; 
    temp_loco_num = temp_loco_num * 10 + num;   
  }
  if(num == 10)
  {
    temp_loco_num = 0;
  }
  //16～25番のショートカットの扱い
  //番号があったらtemp_loco_numへ入れる。
  if((num >=16) && (num <= 16 + LOCO_BANK_NUM) )
  {
    int loco_bank_num = num - 16;
    if(LB[loco_bank_num].loco_exist == true )
    {
      temp_loco_num = LB[loco_bank_num].loco_address;
    }
  }
  Serial.println(temp_loco_num);
  return temp_loco_num; 
}

void loco_bank_reset()
{
  //全て初期アドレスでリセットする。
  for(int i = 0;i < LOCO_BANK_NUM;i++)
  {
    LB[i].state_btn_dir = false; //false = reverse,true =forward 
    LB[i].prev_speed = 0;  // Read keys
    LB[i].function_state_32 = (uint32_t)0x80000000;//この時はF31だけOn状態0
    LB[i].loco_address = 0;//DECODER_ADDRESS;
    LB[i].loco_exist = false;//最初は存在なし。
  }
  loco_bank_count = 0;
}

void loco_bank_save(int loco_address)
{
  //現状のaddressをbankに入れる。
  boolean exist_flag = false;
  //すでにBankにあるかを探す
  for(int i = 0;i < LOCO_BANK_NUM;i++)
  {
    if(loco_address == LB[i].loco_address)
    {
      LB[i].state_btn_dir = state_btn_dir; 
      LB[i].prev_speed = prev_speed;
      LB[i].function_state_32 = function_state_32;
      LB[i].loco_address = loco_address;
      LB[i].loco_exist = true;//一応存在しているので、上書き（たぶん要らないと思うんだが）
      exist_flag = true;
      Serial.print("loco_bank_save_exist:");
      Serial.println(i);
      break;
    }
  }
  //もしBankになかった場合
  if(exist_flag == false)
  {
    LB[loco_bank_count].state_btn_dir = state_btn_dir; 
    LB[loco_bank_count].prev_speed = prev_speed;
    LB[loco_bank_count].function_state_32 = function_state_32;
    LB[loco_bank_count].loco_address = loco_address;
    LB[loco_bank_count].loco_exist = true;//存在を登録
    Serial.print("loco_bank_save_not_e:");
    Serial.println(loco_bank_count);
    loco_bank_count++;
    if(loco_bank_count == LOCO_BANK_NUM)
    {
      loco_bank_count = 0;
    }
  }
  
}

void loco_bank_load(int loco_address)
{
  //メインにロード、なかった場合は、新規作成
  boolean exist_flag = false;
  for(int i = 0 ;i < LOCO_BANK_NUM;i++)
  {
    if(loco_address == LB[i].loco_address)
    {
      state_btn_dir = LB[i].state_btn_dir; 
      prev_speed = LB[i].prev_speed;
      function_state_32 = LB[i].function_state_32;
      loco_address = LB[i].loco_address;
      exist_flag = true;
      Serial.print("loco_bank_load_exist:");
      Serial.println(i);
      break;
    }
  }
  if(exist_flag ==false)
  {
    //無い場合は初期化しておく
    state_btn_dir = true; 
    prev_speed = 0;
    function_state_32 = (uint32_t)0x80000000;//この時はF31だけOn状態
    //loco_address = loco_address;    
    Serial.print("loco_bank_load_not_e:");
    Serial.println(loco_address);
  }
}

