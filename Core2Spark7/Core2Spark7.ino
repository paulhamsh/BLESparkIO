
//#define IOS
#define M5_BRD

#ifdef M5_BRD
#include <M5Core2.h> 
#else
#include "heltec.h"
#endif

#include "Spark.h"
#include "SparkIO.h"

SparkIO spark_io(true);  // true = passthru

unsigned int cmdsub;
SparkMessage msg;
SparkPreset preset;

bool triggered_pedal;
int  curr_preset;

void setup() {
  // put your setup code here, to run once:

  Serial.println("Started");
  
#ifdef M5_BRD
  M5.begin();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(4);
  M5.Lcd.println("Sparker v7");
#ifdef IOS
  M5.Lcd.println("IOS");
#else
  M5.Lcd.println("Android");
#endif  
  M5.Lcd.println("-------------");
  M5.Lcd.setTextSize(3);
#else
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Sparker v7");
#ifdef IOS
  Heltec.display->drawString(0, 20, "IOS");
#else
  Heltec.display->drawString(0, 20, "Android");
#endif  
  Heltec.display->display();
#endif

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

#else
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
  }
  
/*
  if (triggered_pedal) {
    triggered_pedal = false;
    if (connected_sp) {
      preset_cmd[preset_cmd_size-2] = curr_preset;
      pSender_sp->writeValue(preset_cmd, preset_cmd_size, false);
      Serial.println("Sending preset command to Spark");
    }
  }
*/

}
