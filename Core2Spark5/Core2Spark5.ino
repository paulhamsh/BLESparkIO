#include <M5Core2.h> 

#include "Spark.h"
#include "SparkIO.h"

SparkIO spark_io(true);

unsigned int cmdsub;
SparkMessage msg;
SparkPreset preset;

//bool triggered_pedal;
int  curr_preset;

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(4);
  M5.Lcd.println("Core2Spark v5");
  M5.Lcd.println("-------------");
  Serial.println("Started");
  M5.Lcd.setTextSize(3);

  curr_preset = 0;
  connect_to_all(true); // true means use BLE for all comms (iOS), false means use BLE to the amp but bluetooth classic to the app (for Android :-) )
}


void loop() {
  // put your main code here, to run repeatedly:
  M5.update();

  if (M5.BtnA.wasPressed() ) {
    Serial.println("BUTTON PRESS");
    curr_preset++;
    if (curr_preset > 3) curr_preset=0;
    spark_io.sp_change_hardware_preset(curr_preset);
    spark_io.app_change_hardware_preset(curr_preset);
  }

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
