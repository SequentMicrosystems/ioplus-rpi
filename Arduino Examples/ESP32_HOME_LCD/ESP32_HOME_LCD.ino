/*
 * This example demonstrare the features of the Sequent Microsystems Home Automation HAT and LCD Adaptor HAT.
 * Both cards are connected to the Sequent Microsystems ESP32_PI card.
 * Select the "DOIT ESP32 DEVKIT V1" board from the Tools menu before Upload.
 * Navigate trought the menus and read the inputs and set the outputs.
 */
#include "SM_Home_Automation.h"
#include "SM_LCDAdapter.h"

SM_Home_Automation card(0);// Home Automation HAT with stack level 0 (no jumpers installed)
SM_LCDAdapter lcd = SM_LCDAdapter(); // 
int menuLevel = 0;
int sel1 = 0;
int channel = 0;
int analogOutVal[4] = {0, 0, 0, 0};
int relayVal = 0;


void setup() {

  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  //set the backlight
  lcd.writeBl(20);
  // Print a message to the LCD.
  Serial.println("Sequent display init successful");
  lcd.setCursor(1, 0);
  lcd.print("FOUR LINES 20 CHAR");
  lcd.setCursor(0, 1);
  lcd.print("ENCODER & 6 SWITCHES");
  lcd.setCursor(1, 2);
  lcd.print("USING ESP32-PI FOR");
  lcd.setCursor(0, 3);
  lcd.print("COMPLEX EMBEDDED APP");
  lcd.resetEncoder();

  if (card.begin())
  {
    Serial.print("Home Automation Card detected\n");
  }
  else
  {
    Serial.print("Home Automation Card NOT detected!\n");

  }
  delay(3000);
  lcd.clear();
  //Print main menu
  lcd.setCursor(0, 0);
  lcd.print("> Analog IN");
  lcd.setCursor(2, 1);
  lcd.print("Analog OUT");
  lcd.setCursor(2, 2);
  lcd.print("Digital I/O");

  lcd.setCursor(0, 3);
  lcd.print("Enter           Exit");
}

void goMainMenu()
{
  lcd.clear();
  //Print main menu
  lcd.setCursor(0, 0);
  lcd.print("> Analog IN");
  lcd.setCursor(2, 1);
  lcd.print("Analog OUT");
  lcd.setCursor(2, 2);
  lcd.print("Digital I/O");

  lcd.setCursor(0, 3);
  lcd.print("Enter");
  lcd.resetEncoder();
  menuLevel = 0;
  sel1 = 0;
}
void goAnalogInMenu()
{
  channel = 1;
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Analog IN Menu");
  lcd.setCursor(0, 2);
  lcd.printf("Ch:%d = %d mV  ", channel, card.readAanalogMv(channel));
  lcd.setCursor(16, 3);
  lcd.print("Exit");
  lcd.resetEncoder();
  menuLevel = 1;
}

void goAnalogOutMenu()
{
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Analog OUT Menu");
  lcd.setCursor(0, 2);
  lcd.resetEncoder();
  channel = 1;
  lcd.printf("Ch:%d = %d mV  ", channel, analogOutVal[channel - 1]);
  lcd.setCursor(0, 3);
  lcd.print("SW              Exit");
  menuLevel = 1;
}

void goDigitalMenu()
{
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("DIGITAL Menu");
  lcd.setCursor(0, 2);
  lcd.resetEncoder();
  card.writeRelay(relayVal);
  lcd.setCursor(0, 1);
  lcd.printf("RELAY: %d", relayVal);
  lcd.setCursor(0, 2);
  lcd.printf("OPTO: %d", card.readOpto());
  lcd.setCursor(16, 3);
  lcd.print("Exit");
  menuLevel = 1;
}

void menuProcess(void)
{
  int auxSel = 0;
  bool refresh = false;
  if (menuLevel == 0)
  {
    auxSel = (lcd.readEncoder() + 100) % 3;

    if (sel1 != auxSel)
    {
      lcd.setCursor(0, sel1);
      lcd.print(" ");
      lcd.setCursor(0, auxSel);
      lcd.print(">");
      sel1 = auxSel;
    }
    if (lcd.readButtonLatch(1))
    {
      lcd.readButtonLatch();//clear all the button state
      switch (sel1)
      {
        case 0:
          goAnalogInMenu();
          break;
        case 1:
          goAnalogOutMenu();
          break;
        case 2:
          goDigitalMenu();
          break;
      }
    }
  }
  else
  {
    if (lcd.readButtonLatch(6))
    {
      lcd.readButtonLatch();//clear all the button state
      goMainMenu();
    }
    else switch (sel1)
      {
        case 0:
          channel = 1 + (100 + lcd.readEncoder()) % 8;
          lcd.setCursor(0, 2);
          lcd.printf("Ch:%d = %d mV  ", channel, card.readAanalogMv(channel));
          break;
        case 1:
          if(lcd.readButtonLatch(1))
          {
            channel += 1;
            if(channel > 4) channel = 1; 
            refresh = true;         
          }
          auxSel = lcd.readEncoder();
          if(auxSel != 0)
          {
            lcd.resetEncoder();
            analogOutVal[channel - 1] += auxSel * 10;
            // Limit the analog output to the 3.2V in case of loopback cable is installed
            if(analogOutVal[channel - 1] > 3200) analogOutVal[channel - 1] = 3200;
            if(analogOutVal[channel - 1] < 0) analogOutVal[channel - 1] = 0;
            card.writeAnalogMv(channel, analogOutVal[channel - 1]);
            refresh = true;
          }
          if(refresh)
          {
            lcd.setCursor(0, 2);
            lcd.printf("Ch:%d = %d mV  ", channel, analogOutVal[channel - 1]);
          }
          break;
        case 2:
          auxSel = lcd.readEncoder();
          if (auxSel != 0)
          {
            lcd.resetEncoder();
            relayVal += auxSel;
            if (relayVal < 0) relayVal = 0;
            if (relayVal > 255) relayVal = 255;
            card.writeRelay(relayVal);
            lcd.setCursor(0, 1);
            lcd.printf("RELAY: %d ", relayVal);

          }
          lcd.setCursor(0, 2);
          lcd.printf("OPTO: %d ", card.readOpto());
          break;
        default:
          goMainMenu();
          break;
      }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  menuProcess();
  delay(100);
}
