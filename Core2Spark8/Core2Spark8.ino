
#define M5_BRD

#ifdef M5_BRD
#include <M5Core2.h> 
#else
#include "heltec.h"
#endif

#include "Spark.h"
#include "SparkIO.h"

byte preset_spk_cmd[] = {
  0x01, 0xFE, 0x00, 0x00,
  0x53, 0xFE, 0x1A, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0xF0, 0x01, 0x24, 0x00,
  0x01, 0x38, 0x00, 0x00,
  0x00, 0xF7
};

byte preset_app_cmd[] = {
  0x01, 0xFE, 0x00, 0x00,
  0x41, 0xFF, 0x1A, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0xF0, 0x01, 0x50, 0x00,
  0x03, 0x38, 0x00, 0x00,
  0x00, 0xF7
};

const int preset_cmd_size = 26;

SparkIO spark_io(true);  // true = passthru

unsigned int cmdsub;
SparkMessage msg;
SparkPreset preset;

bool triggered_pedal;
int  curr_preset;
int i;

void setup() {
  // put your setup code here, to run once:
  
#ifdef M5_BRD
  M5.begin();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(4);
  M5.Lcd.println("Sparker v8");
 
#else
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Sparker v7");
  Heltec.display->display();
  Heltec.display->display();
#endif

  Serial.println("Started");
  curr_preset = 0;
  connect_to_all(); 
}


void loop() {
  // put your main code here, to run repeatedly:
#ifdef M5_BRD
M5.update();
#endif

#ifdef M5_BRD
  if (M5.BtnA.wasPressed() ) {
    Serial.println("BUTTON PRESS");
    curr_preset++;
    if (curr_preset > 3) curr_preset=0;
    spark_io.sp_change_hardware_preset(curr_preset);
    spark_io.app_change_hardware_preset(curr_preset);
  }
#endif

  spark_io.sp_process();
  spark_io.app_process();
 
  if (spark_io.sp_get_message(&cmdsub, &msg, &preset)) { //there is something there
    Serial.print("From Spark: ");
    Serial.println(cmdsub, HEX);


  }
  
  if (spark_io.app_get_message(&cmdsub, &msg, &preset)) { //there is something there
    Serial.print("From App: ");
    Serial.println(cmdsub, HEX);

    if (cmdsub == 0x0101) {
       Serial.println(preset.Name);
       Serial.println(preset.UUID); 
       Serial.println(preset.Description);         
       for (i=0;i<7;i++) {
          Serial.println(preset.effects[i].EffectName);
       }
    }
  }
  

  if (triggered_pedal) {
    triggered_pedal = false;
    
    preset_spk_cmd[preset_cmd_size-2] = curr_preset;
    preset_app_cmd[preset_cmd_size-2] = curr_preset;
    preset_app_cmd[preset_cmd_size-7] = curr_preset; // set the checksum too 
     
    sp_write(preset_spk_cmd, preset_cmd_size);
    app_write(preset_app_cmd, preset_cmd_size);
    Serial.println("Sending preset commands");
  }


}
