#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2017-01-25 15:04:44

#include "Arduino.h"
#include "Arduino.h"
#include "libraries/Keypad_I2C/Keypad_I2C.h"
#include "libraries/Keypad/Keypad.h"
#include "libraries/NewliquidCrystal/LiquidCrystal_I2C.h"
#include "libraries/OMEEPROM/OMEEPROM.h"
#include "libraries/OMMenuMgr/OMMenuMgr.h"
#include "libraries/Interval/interval.h"
void uiDraw(char* p_text, int p_row, int p_col, int len) ;
void uiLcdPrintSpaces8() ;
void uiMain() ;
void loadEEPROM() ;
void saveDefaultEEPROM() ;
void setup() ;
void uiResetParam() ;
void uiMessage(const char* msg) ;
void uiMessageStarted();
void uiMessageStopped();
void uiMessageRestarted();
void X_uiStart() ;
void X_uiStop() ;
void X_uiRestart() ;
void A_uiStart() ;
void A_uiStop() ;
void A_uiRestart() ;
void X_start() ;
void X_stop() ;
void X_restart() ;
void A_start() ;
void A_stop() ;
void A_restart() ;
bool getInstrumentControl(bool a, uint8_t mode) ;
void loop() ;

#include "timer.ino"


#endif