#include "esphome.h"

class JuraCoffee : public PollingComponent, public UARTDevice {
 Sensor *xsensor1 {nullptr};
 Sensor *xsensor2 {nullptr};
 Sensor *xsensor3 {nullptr};
 Sensor *xsensor4 {nullptr};
 Sensor *xsensor5 {nullptr};
 Sensor *xsensor6 {nullptr};
 Sensor *xsensor7 {nullptr};
 Sensor *xsensor8 {nullptr};
 Sensor *xsensor9 {nullptr};
 TextSensor *xsensor10 {nullptr};
 TextSensor *xsensor11 {nullptr};
 Sensor *xsensor12 {nullptr};
 Sensor *xsensor13 {nullptr};
 Sensor *xsensor14 {nullptr};
 Sensor *xsensor15 {nullptr};
 Sensor *xsensor16 {nullptr};
 Sensor *xsensor17 {nullptr};

 public:
  JuraCoffee(UARTComponent *parent, Sensor *sensor1, Sensor *sensor2, Sensor *sensor3, Sensor *sensor4, Sensor *sensor5, Sensor *sensor6, Sensor *sensor7, Sensor *sensor8, Sensor *sensor9, TextSensor *sensor10, TextSensor *sensor11, Sensor *sensor12, Sensor *sensor13,Sensor *sensor14,Sensor *sensor15, Sensor *sensor16,Sensor *sensor17) : UARTDevice(parent) , xsensor1(sensor1) , xsensor2(sensor2) , xsensor3(sensor3) , xsensor4(sensor4) , xsensor5(sensor5) , xsensor6(sensor6) , xsensor7(sensor7), xsensor8(sensor8), xsensor9(sensor9), xsensor10(sensor10), xsensor11(sensor11),xsensor12(sensor12),xsensor13(sensor13),xsensor14(sensor14),xsensor15(sensor15),xsensor16(sensor16), xsensor17(sensor17) {}

  long num_single_cup, num_double_cup, num_cup_L, num_cup_M, num_cup_S, unknown_descale, num_cup_powder, num_rinses, num_clean;
  std::string tray_status, tank_status;

  // Jura communication function taken in entirety from cmd2jura.ino, found at https://github.com/hn/jura-coffee-machine
  String cmd2jura(String outbytes) {
    String inbytes;
    int w = 0;

    while (available()) {
      read();
    }

    outbytes += "\r\n";
    for (int i = 0; i < outbytes.length(); i++) {
      for (int s = 0; s < 8; s += 2) {
        char rawbyte = 255;
        bitWrite(rawbyte, 2, bitRead(outbytes.charAt(i), s + 0));
        bitWrite(rawbyte, 5, bitRead(outbytes.charAt(i), s + 1));
        write(rawbyte);
      }
      delay(8);
    }

    int s = 0;
    char inbyte;
    while (!inbytes.endsWith("\r\n")) {
      if (available()) {
        byte rawbyte = read();
        bitWrite(inbyte, s + 0, bitRead(rawbyte, 2));
        bitWrite(inbyte, s + 1, bitRead(rawbyte, 5));
        if ((s += 2) >= 8) {
          s = 0;
          inbytes += inbyte;
        }
      } else {
        delay(10);
      }
      if (w++ > 500) {
        return "";
      }
    }

    return inbytes.substring(0, inbytes.length() - 2);
  }

  void setup() override {
    this->set_update_interval(60000); // 600000 = 10 minutes // Now 60 seconds
  }

  void loop() override {
  }

  void update() override {
    String result, hexString, substring, tmp;
    byte hex_to_byte;
    int trayBit, tankBit;
    int read_bit0, read_bit1, read_bit2, read_bit3, read_bit4, read_bit5, read_bit6, read_bit7;
    // For Testing
    // int read_bit0,read_bit1,read_bit2,read_bit3,read_bit4,read_bit5,read_bit6,read_bit7;

    // Fetch our line of EEPROM
    result = cmd2jura("RT:0000");

    // ESP_LOGD("main", "Raw Line of EEPROM 3-7: %s", result.substring(3,7).c_str());
    // ESP_LOGD("main", "Raw Line of EEPROM 7-11: %s", result.substring(7,11).c_str());
    // ESP_LOGD("main", "Raw Line of EEPROM 11-15: %s", result.substring(11,15).c_str());
    // ESP_LOGD("main", "Raw Line of EEPROM 15-19: %s", result.substring(15,19).c_str());
    // ESP_LOGD("main", "Raw Line of EEPROM 19-23: %s", result.substring(19,23).c_str());
    // ESP_LOGD("main", "Raw Line of EEPROM 23-27: %s", result.substring(23,27).c_str());
    // ESP_LOGD("main", "Raw Line of EEPROM 27-31: %s", result.substring(27,31).c_str());
    // ESP_LOGD("main", "Raw Line of EEPROM 31-35: %s", result.substring(31,35).c_str());
    // ESP_LOGD("main", "Raw Line of EEPROM 35-39: %s", result.substring(35,39).c_str());

    // Get Single Cup
    substring = result.substring(3,7);
    num_single_cup = strtol(substring.c_str(),NULL,16);

    // Get Double Cup
    substring = result.substring(7,11);
    num_double_cup = strtol(substring.c_str(),NULL,16);

    // Get Coffee Small
    substring = result.substring(11,15);
    num_cup_S = strtol(substring.c_str(),NULL,16);

    // Get Coffee Medium
    substring = result.substring(15,19);
    num_cup_M = strtol(substring.c_str(),NULL,16);
    
    // Get Coffee Large
    substring = result.substring(19,23);
    num_cup_L = strtol(substring.c_str(),NULL,16);
    
    // Get Unknown. Descale?
    substring = result.substring(23,27);
    unknown_descale = strtol(substring.c_str(),NULL,16);
    
    // Get Coffee from Powder
    substring = result.substring(27,31);
    num_cup_powder = strtol(substring.c_str(),NULL,16);
    
    // Get Number of Rinses
    substring = result.substring(31,35);
    num_rinses = strtol(substring.c_str(),NULL,16);

    // Get Cleanings
    substring = result.substring(35,39);
    num_clean = strtol(substring.c_str(),NULL,16);

    // Tray & water tank status
    // Much gratitude to https://www.instructables.com/id/IoT-Enabled-Coffee-Machine/ for figuring out how these bits are stored
    result = cmd2jura("IC:");
    hexString = result.substring(3,5);
    hex_to_byte = strtol(hexString.c_str(),NULL,16);
    trayBit = bitRead(hex_to_byte, 4);
    tankBit = bitRead(hex_to_byte, 5);
    if (trayBit == 1) { tray_status = "Missing"; } else { tray_status = "Present"; }
    if (tankBit == 1) { tank_status = "Fill Tank"; } else { tank_status = "OK"; }

    // For Testing
    read_bit0 = bitRead(hex_to_byte, 0);
    read_bit1 = bitRead(hex_to_byte, 1);
    read_bit2 = bitRead(hex_to_byte, 2);
    read_bit3 = bitRead(hex_to_byte, 3);
    read_bit6 = bitRead(hex_to_byte, 6);
    read_bit7 = bitRead(hex_to_byte, 7);
    // ESP_LOGD("main", "Raw IC result: %s", result.c_str());
    // ESP_LOGD("main", "Substringed: %s", hexString.c_str());
    // ESP_LOGD("main", "Converted_To_Long: %li", hex_to_byte);
    // ESP_LOGD("main", "As Bits: %d%d%d%d%d%d%d%d", read_bit7,read_bit6,read_bit5,read_bit4,read_bit3,read_bit2,read_bit1,read_bit0);

    if (xsensor1 != nullptr)   xsensor1->publish_state(num_single_cup);
    if (xsensor2 != nullptr)   xsensor2->publish_state(num_double_cup);
    if (xsensor3 != nullptr)   xsensor3->publish_state(num_cup_S);
    if (xsensor4 != nullptr)   xsensor4->publish_state(num_cup_M);
    if (xsensor5 != nullptr)   xsensor5->publish_state(num_cup_L);
    if (xsensor6 != nullptr)   xsensor6->publish_state(unknown_descale);
    if (xsensor7 != nullptr)   xsensor7->publish_state(num_cup_powder);
    if (xsensor8 != nullptr)   xsensor8->publish_state(num_rinses);
    if (xsensor9 != nullptr)   xsensor9->publish_state(num_clean);
    if (xsensor10 != nullptr)   xsensor10->publish_state(tray_status);
    if (xsensor11 != nullptr)   xsensor11->publish_state(tank_status);
    if (xsensor12 != nullptr)   xsensor12->publish_state(read_bit0);
    if (xsensor13 != nullptr)   xsensor13->publish_state(read_bit1);
    if (xsensor14 != nullptr)   xsensor14->publish_state(read_bit2);
    if (xsensor15 != nullptr)   xsensor15->publish_state(read_bit3);
    if (xsensor16 != nullptr)   xsensor16->publish_state(read_bit6);
    if (xsensor17 != nullptr)   xsensor17->publish_state(read_bit7);

  }
};