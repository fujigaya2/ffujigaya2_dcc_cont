//200603 fujigaya2_ds_serial_cont
//desktopstation softwareの代わりにdsmainなどを制御するシリアルコマンドを送るライブラリ
//命令間は100ms間隔を開けること　とのこと。

#include "fujigaya2_ds_serial_master.h"

ds_serial_master::ds_serial_master()
{
  //init ds_serial_master
  //やることなし！
}

void ds_serial_master::begin(uint32_t baud_rate)
{
  //Serialを開始する。
  Serial.begin(baud_rate);
  Serial.print("##debug## start\n");
}

int ds_serial_master::write_func_packet(unsigned int address,byte function,bool on_off)
{
  String temp_str = "";
  temp_str.reserve(SEND_STR_LENGTH);
  temp_str += "setLocoFunction(";
  temp_str += address + DECODER_ADDRESS_OFFSET;
  temp_str += ",";
  temp_str += function;
  temp_str += ",";
  temp_str += on_off;
  temp_str += ")\n";
  Serial.print(temp_str);

  //将来的に戻り値を入れるようにする
  return 0;
}

int ds_serial_master::write_speed_packet(unsigned int address,byte loco_speed)
{
  //direction無し！
  String temp_str = "";
  temp_str.reserve(SEND_STR_LENGTH);
  temp_str += "setLocoSpeed(";
  temp_str += address + DECODER_ADDRESS_OFFSET;
  temp_str += ",";
  temp_str += loco_speed;
  temp_str += ",";
  temp_str += 2;//step128固定
  temp_str += ")\n";
  Serial.print(temp_str);

  //将来的に戻り値を入れるようにする
  return 0;
}

int ds_serial_master::write_direction_packet(unsigned int address,bool loco_direction)
{
  //speed無し！
  String temp_str = "";
  temp_str.reserve(SEND_STR_LENGTH);
  temp_str += "setLocoDirection(";
  temp_str += address + DECODER_ADDRESS_OFFSET;
  temp_str += ",";
  temp_str += loco_direction;
  temp_str += ")\n";
  Serial.print(temp_str);

  //将来的に戻り値を入れるようにする
  return 0;
}

int ds_serial_master::write_accessory_packet(unsigned int address,bool on_off)
{
  String temp_str = "";
  temp_str.reserve(SEND_STR_LENGTH);
  temp_str += "setTurnout(";
  temp_str += address + ACC_DECORDER_ADDRESS_OFFSET;
  temp_str += ",";
  temp_str += on_off;
  temp_str += ")\n";
  Serial.print(temp_str);

  //将来的に戻り値を入れるようにする
  return 0;
}

int ds_serial_master::write_power_packet(bool on_off)
{
  String temp_str = "";
  temp_str.reserve(SEND_STR_LENGTH);
  temp_str += "setPower(";
  temp_str += on_off;
  temp_str += ")\n";
  Serial.print(temp_str);

  //将来的に戻り値を入れるようにする
  return 0;
}

bool ds_serial_master::read_power_status()
{
  //作っていない
}

int ds_serial_master::write_ping()
{
  String temp_str = "";
  temp_str.reserve(SEND_STR_LENGTH);
  temp_str += "setPing()\n";
  Serial.print(temp_str);

  //将来的に戻り値を入れるようにする
  return 0;  
}
