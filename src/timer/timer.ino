#include "Arduino.h"

#include "libraries/Keypad_I2C/Keypad_I2C.h"
#include "libraries/Keypad/Keypad.h"
#include "libraries/NewliquidCrystal/LiquidCrystal_I2C.h"
#include "libraries/OMEEPROM/OMEEPROM.h"
#include "libraries/OMMenuMgr/OMMenuMgr.h"
#include "libraries/Interval/interval.h"

#define PIN_LED 13


Interval secInterval;
unsigned long secCnt;

uint8_t X_state;

uint8_t A_mode, A_state, A_init;
uint A_onHour, A_onMin, A_onSec, A_offHour, A_offMin, A_offSec;
uint A_cyclesLimit, A_halfCycles;
unsigned long A_set, A_sec;
bool  A_out, A_outPin;

uint8_t uiPage;


#define LCD_I2CADDR 0x20
const byte LCD_ROWS = 2;
const byte LCD_COLS = 16;

#define KPD_I2CADDR 0x20
const byte KPD_ROWS = 4;
const byte KPD_COLS = 4;

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

char keys[KPD_ROWS][KPD_COLS] = {
  {'D','C','B','A'},
  {'#','9','6','3'},
  {'0','8','5','2'},
  {'*','7','4','1'}
};

class Keypad_I2C2 : public Keypad_I2C {
	unsigned long kTime;
public:
    Keypad_I2C2(char *userKeymap, byte *row, byte *col, byte numRows, byte numCols, byte address, byte width = 1) : Keypad_I2C(userKeymap, row, col, numRows, numCols, address, width) {
    };
    char Keypad_I2C2::getRawKey() {
    	getKeys();
    	for(int r=0; r<sizeKpd.rows; r++)
    		for(int c=0; c<sizeKpd.columns; c++)
    			if((bitMap[r] >> c) & 1)
    				return keymap[r * sizeKpd.columns + c];
    	return NO_KEY;
    };
    char Keypad_I2C2::getKey2() {
    	getKeys();

    	//TODO !!! Dirty trick !!!
    	if(bitMap[3] == 8) {
    		if(bitMap[2] == 8) X_start();
    	    if(bitMap[1] == 8) X_stop();
    	    if(bitMap[0] == 8) X_restart();
    		//return NO_KEY;
    	}
    	if(bitMap[3] == 2) {
    		if(bitMap[2] == 8) A_mode=1;
    		if(bitMap[1] == 8) A_mode=2;
    		if(bitMap[0] == 8) A_mode=0;

    		if(bitMap[2] == 4) A_mode=1;
    		if(bitMap[1] == 4) A_mode=2;
    		if(bitMap[0] == 4) A_mode=0;

    		if(bitMap[2] == 2) A_mode=1;
    		if(bitMap[1] == 2) A_mode=2;
    		if(bitMap[0] == 2) A_mode=0;

    		if(bitMap[2] == 1) A_mode=1;
    		if(bitMap[1] == 1) A_mode=2;
    		if(bitMap[0] == 1) A_mode=0;
    		//return NO_KEY;
    	}
    	if(bitMap[3] == 4) {
    		if(bitMap[2] == 8) A_start();
    	    if(bitMap[1] == 8) A_stop();
    	    if(bitMap[0] == 8) A_restart();

    	    if(bitMap[2] == 4) A_start();
    	    if(bitMap[1] == 4) A_stop();
    	    if(bitMap[0] == 4) A_restart();

    	    if(bitMap[2] == 2) A_start();
    	    if(bitMap[1] == 2) A_stop();
    	    if(bitMap[0] == 2) A_restart();

    	    if(bitMap[2] == 1) A_start();
    	    if(bitMap[1] == 1) A_stop();
    	    if(bitMap[0] == 1) A_restart();
    	    //return NO_KEY;
    	}


    	//Serial.println(kTime);
    	// For menu system, makes delay between first and next
    	if(bitMap[0] || bitMap[1] || bitMap[2] || bitMap[3]) {
    		if(!kTime)
    			kTime = millis();
    		if((kTime + 500) > millis())
    			if((kTime + 200) < millis())
    				return NO_KEY;
    	}
        else
        	kTime = 0;
    	return getRawKey();
    }
};

byte rowPins[KPD_ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[KPD_COLS] = {4, 5, 6, 7}; //connect to the column pinouts of the keypad
Keypad_I2C2 kpd( makeKeymap(keys), rowPins, colPins, KPD_ROWS, KPD_COLS, KPD_I2CADDR, PCF8574 );

class OMMenuMgr2 : public OMMenuMgr {
public:
    OMMenuMgr2(const OMMenuItem* c_first, uint8_t c_type, Keypad_I2C2* c_kpd) :OMMenuMgr( c_first, c_type) {
      kpd = c_kpd;
    };
    int OMMenuMgr2::_checkDigital() {
    	char k = kpd->getKey2();
    	if(k == 'A') return BUTTON_INCREASE;
    	if(k == 'B') return BUTTON_DECREASE;
    	if(k == 'D') return BUTTON_FORWARD;
    	if(k == '#') return BUTTON_BACK;
    	if(k == '*') return BUTTON_SELECT;

    	return k;
    	return BUTTON_NONE;
    }
private:
    Keypad_I2C2* kpd;
};



//enum  { STATE_RUNNING, STATE_PAUSED, STATE_STOPPED };


/*
//EN
#define TEXT_AUTO_EM			"AUTO!"
#define TEXT_OFF_EM				"OFF!"
#define TEXT_ON_EM 				"ON!"
#define TEXT_START_EM 			"STOP!"
#define TEXT_STOP_EM 			"START!"
#define TEXT_START_ALL_EM 		"START ALL!"
#define TEXT_STOP_ALL_EM 		"STOP ALL!"
#define TEXT_RESTART_ALL_EM 	"RESTART ALL!"
#define TEXT_RESETPARAM_EM 		"RESET SETTING!"

#define TEXT_STARTED			"STARTED"
#define TEXT_STOPPED			"STOPPED"
#define TEXT_RESTARTED			"RESTARTED"
#define TEXT_OFF				"OFF "
#define TEXT_ON 				"ON  "
#define TEXT_MAN 				"MAN  "
#define TEXT_AUTO 				"AUTO "
#define TEXT_STOP 				"STOP"
#define TEXT_RUN 				"RUN "
#define TEXT_OFF2				"0 "
#define TEXT_ON2 				"1 "
#define TEXT_MAN2 				"M "
#define TEXT_AUTO2 				"A "
#define TEXT_STOP2 				"S "
#define TEXT_RUN2 				"R "


#define TEXT_A_ROOT				"A SETTING->"
#define TEXT_A_START_ALL_EM 	"A START!"
#define TEXT_A_STOP_ALL_EM 		"A STOP!"
#define TEXT_A_RESTART_ALL_EM 	"A RESTART!"
#define TEXT_A_INIT				"A INIT:"
#define TEXT_A_MODE				"A MODE:"
#define TEXT_A_ONHOUR			"A ON:       [H]"
#define TEXT_A_ONMIN			"A ON:       [M]"
#define TEXT_A_ONSEC			"A ON:       [S]"
#define TEXT_A_OFFHOUR			"A OFF:      [H]"
#define TEXT_A_OFFMIN			"A OFF:      [M]"
#define TEXT_A_OFFSEC			"A OFF:      [S]"
#define TEXT_A_CYCLELIMIT		"A CYCLES LIMIT:"
*/

//CZ
#define TEXT_AUTO_EM			"AUTO!"
#define TEXT_OFF_EM				"VYP!"
#define TEXT_ON_EM 				"ZAP!"
#define TEXT_START_EM 			"STOP!"
#define TEXT_STOP_EM 			"START!"
#define TEXT_START_ALL_EM 		"START VSE!"
#define TEXT_STOP_ALL_EM 		"STOP VSE!"
#define TEXT_RESTART_ALL_EM 	"RESTART VSE!"
#define TEXT_RESETPARAM_EM 		"RESET NASTAVEN!"

#define TEXT_STARTED			"STARTED"
#define TEXT_STOPPED			"STOPPED"
#define TEXT_RESTARTED			"RESTARTED"
#define TEXT_OFF				"VYP "
#define TEXT_ON 				"ZAP "
#define TEXT_MAN 				"MAN  "
#define TEXT_AUTO 				"AUTO "
#define TEXT_STOP 				"STOP"
#define TEXT_RUN 				"BEZI"
#define TEXT_OFF2				"0 "
#define TEXT_ON2 				"1 "
#define TEXT_MAN2 				"M "
#define TEXT_AUTO2 				"A "
#define TEXT_STOP2 				"S "
#define TEXT_RUN2 				"B "

#define TEXT_A_ROOT				"A NASTAVENI->"
#define TEXT_A_START_ALL_EM 	"A START!"
#define TEXT_A_STOP_ALL_EM 		"A STOP!"
#define TEXT_A_RESTART_ALL_EM 	"A RESTART!"
#define TEXT_A_INIT				"A VYCHOZI:"
#define TEXT_A_MODE				"A REZIM:"
#define TEXT_A_ONHOUR			"A ZAP:    [HOD]"
#define TEXT_A_ONMIN			"A ZAP:    [MIN]"
#define TEXT_A_ONSEC			"A ZAP:    [SEC]"
#define TEXT_A_OFFHOUR			"A VYP:    [HOD]"
#define TEXT_A_OFFMIN			"A VYP:    [MIN]"
#define TEXT_A_OFFSEC			"A VYP:	   [SEC]"
#define TEXT_A_CYCLELIMIT		"A POCET CYKLU: "

//EEPROM
#define ADDR_A_MODE 		20
#define ADDR_A_INIT 		24
#define ADDR_A_ONHOUR 		28
#define ADDR_A_ONMIN 		32
#define ADDR_A_ONSEC 		36
#define ADDR_A_OFFHOUR 		40
#define ADDR_A_OFFMIN 		44
#define ADDR_A_OFFSEC 		48
#define ADDR_A_CYCLESLIMIT	52


// Create a list of states and values for a select input
MENU_SELECT_ITEM  sel_auto= { 0, {TEXT_AUTO_EM} };
MENU_SELECT_ITEM  sel_off = { 1, {TEXT_OFF_EM} };
MENU_SELECT_ITEM  sel_on  = { 2, {TEXT_ON_EM} };
//MENU_SELECT_ITEM  sel_rc_disable={ 3, {"DISABLE RADIO"} };
//MENU_SELECT_ITEM  sel_rcon= { 3, {"RC ON"} };
MENU_SELECT_ITEM  sel_stop= { 0, {TEXT_START_EM} };
MENU_SELECT_ITEM  sel_start={ 1, {TEXT_STOP_EM} };

MENU_SELECT_LIST  const stateMode_list[] = { &sel_auto, &sel_off, &sel_on};
MENU_SELECT_LIST  const stateOffOn_list[] = { &sel_off, &sel_on };
//MENU_SELECT_LIST  const stateStopStart_list[] = { &sel_stop, &sel_start};

MENU_ITEM X_start_item   = { {TEXT_START_ALL_EM},  ITEM_ACTION, 0,        MENU_TARGET(&X_uiStart) };
MENU_ITEM X_stop_item   =  { {TEXT_STOP_ALL_EM },  ITEM_ACTION, 0,        MENU_TARGET(&X_uiStop)  };
MENU_ITEM X_restart_item  ={ {TEXT_RESTART_ALL_EM},  ITEM_ACTION, 0,        MENU_TARGET(&X_uiRestart) };

MENU_ITEM reset_item  ={ {TEXT_RESETPARAM_EM},  ITEM_ACTION, 0,        MENU_TARGET(&uiResetParam) };

//MENU_SELECT A_state_select = { &A_state,           MENU_SELECT_SIZE(stateStopStart_list),   MENU_TARGET(&stateStopStart_list) };
//MENU_VALUE A_state_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&A_state_select), 0};
//MENU_ITEM A_state_item    =  { {"A STATE:"}, ITEM_VALUE,  0,        MENU_TARGET(&A_state_value) };

MENU_SELECT A_init_select = { &A_init,           MENU_SELECT_SIZE(stateOffOn_list),   MENU_TARGET(&stateOffOn_list) };
MENU_VALUE A_init_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&A_init_select), ADDR_A_INIT};
MENU_ITEM A_init_item    =  { {TEXT_A_INIT}, ITEM_VALUE,  0,        MENU_TARGET(&A_init_value)};

MENU_ITEM A_start_item   = { {TEXT_A_START_ALL_EM},  ITEM_ACTION, 0,        MENU_TARGET(&A_uiStart) };
MENU_ITEM A_stop_item    = { {TEXT_A_STOP_ALL_EM },  ITEM_ACTION, 0,        MENU_TARGET(&A_uiStop) };
MENU_ITEM A_restart_item = { {TEXT_A_RESTART_ALL_EM},  ITEM_ACTION, 0,        MENU_TARGET(&A_uiRestart) };

MENU_SELECT A_mode_select = { &A_mode,           MENU_SELECT_SIZE(stateMode_list),   MENU_TARGET(&stateMode_list) };
MENU_VALUE A_mode_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&A_mode_select), ADDR_A_MODE};
MENU_ITEM A_mode_item    =  { {TEXT_A_MODE}, ITEM_VALUE,  0,        MENU_TARGET(&A_mode_value) };
//                               TYPE             MAX    MIN    TARGET
MENU_VALUE A_onHour_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&A_onHour), ADDR_A_ONHOUR};
MENU_VALUE A_onMin_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&A_onMin), ADDR_A_ONMIN};
MENU_VALUE A_onSec_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&A_onSec), ADDR_A_ONSEC};
MENU_VALUE A_offHour_value ={ TYPE_UINT,       0,    0,     MENU_TARGET(&A_offHour), ADDR_A_OFFHOUR};
MENU_VALUE A_offMin_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&A_offMin), ADDR_A_OFFMIN};
MENU_VALUE A_offSec_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&A_offSec), ADDR_A_OFFSEC};
MENU_VALUE A_cyclesLimit_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&A_cyclesLimit), ADDR_A_CYCLESLIMIT};

//                                "123456789ABCDEF"
MENU_ITEM A_onHour_item   = { {TEXT_A_ONHOUR},   ITEM_VALUE,  0,        MENU_TARGET(&A_onHour_value) };
MENU_ITEM A_onMin_item   =  { {TEXT_A_ONMIN},   ITEM_VALUE,  0,        MENU_TARGET(&A_onMin_value)  };
MENU_ITEM A_onSec_item   =  { {TEXT_A_ONSEC},   ITEM_VALUE,  0,        MENU_TARGET(&A_onSec_value)  };
MENU_ITEM A_offHour_item  = { {TEXT_A_OFFHOUR},   ITEM_VALUE,  0,        MENU_TARGET(&A_offHour_value)};
MENU_ITEM A_offMin_item   = { {TEXT_A_OFFHOUR},   ITEM_VALUE,  0,        MENU_TARGET(&A_offMin_value) };
MENU_ITEM A_offSec_item   = { {TEXT_A_OFFHOUR},   ITEM_VALUE,  0,        MENU_TARGET(&A_offSec_value) };
MENU_ITEM A_cyclesLimit_item   = { {TEXT_A_CYCLELIMIT},   ITEM_VALUE,  0,        MENU_TARGET(&A_cyclesLimit_value) };

MENU_LIST const A_submenu_list[] = { &A_start_item, &A_stop_item, &A_restart_item, &A_init_item, &A_cyclesLimit_item, &A_onSec_item,&A_onMin_item, &A_onHour_item, &A_offSec_item,&A_offMin_item, &A_offHour_item, &A_mode_item};
MENU_ITEM A_menu_submenu = { {TEXT_A_ROOT},  ITEM_MENU,  MENU_SIZE(A_submenu_list),  MENU_TARGET(&A_submenu_list) };


MENU_LIST const root_list[]   = { &X_start_item, &X_stop_item, &X_restart_item, &A_menu_submenu, &reset_item };
MENU_ITEM menu_root     = { {"ROOT"},        ITEM_MENU,   MENU_SIZE(root_list),    MENU_TARGET(&root_list) };

OMMenuMgr2 Menu(&menu_root, MENU_DIGITAL, &kpd);

void uiDraw(char* p_text, int p_row, int p_col, int len) {
	lcd.backlight();
	lcd.setCursor(p_col, p_row);
	for( int i = 0; i < len; i++ ) {
		if( p_text[i] < '!' || p_text[i] > '~' )
			lcd.write(' ');
		else
			lcd.write(p_text[i]);
	}
}

void uiLcdPrintSpaces8() {
	lcd.print(F("        "));
}

void uiInstrument(char* name, bool out, uint8_t mode, uint8_t state, unsigned long cycles, unsigned long cyclesLimit, bool detail=true) {
	//lcd.clear();
	if(detail)
		lcd.setCursor(0, 0);
	lcd.print(name);

	if(detail) {
		if(A_outPin)
			lcd.print(F(TEXT_ON));
		else
			lcd.print(F(TEXT_OFF));
		if(mode)
			lcd.print(F(TEXT_MAN));
		else
			lcd.print(F(TEXT_AUTO));
		if(state)
			lcd.print(F(TEXT_RUN));
		else
			lcd.print(F(TEXT_STOP));
	}
	else {
		if(A_outPin)
			lcd.print(F(TEXT_ON2));
		else
			lcd.print(F(TEXT_OFF2));
		if(mode)
			lcd.print(F(TEXT_MAN2));
		else
			lcd.print(F(TEXT_AUTO2));
		/*if(state)
			lcd.print(F("R"));
		else
			lcd.print(F("S"));*/
	}

	if(state) {
		if(secCnt & 1)
			lcd.print('-');
		else
			lcd.print('|');
		/*
		uint8_t cnt = secCnt & 3;
		if(cnt == 0) lcd.print('-');
		if(cnt == 1) lcd.print('/');
		if(cnt == 2) lcd.print('|');
		if(cnt == 3) lcd.print("\\");
		*/
	}
	else
		if(secCnt & 1)
			lcd.print('-');
		else
			lcd.print(' ');

	if(detail) {
		lcd.setCursor(0, 1);
		lcd.print(cycles);
		lcd.print('/');
		lcd.print(cyclesLimit);
		uiLcdPrintSpaces8();
		uiLcdPrintSpaces8();
	}
}

void uiMain() {
	Menu.enable(false);

	if(uiPage==0) {
		//lcd.setCursor(0, 0);
		//lcd.print("-");
		//uiLcdPrintSpaces8();
		//uiLcdPrintSpaces8();
		lcd.setCursor(0, 0);
		uiInstrument("A:", A_outPin, A_mode, A_state, A_halfCycles>>1, A_cyclesLimit, false);
		lcd.print(F("  "));
		uiInstrument("B:", A_outPin, A_mode, A_state, A_halfCycles>>1, A_cyclesLimit, false);

		lcd.setCursor(0, 1);
		uiInstrument("C:", A_outPin, A_mode, A_state, A_halfCycles>>1, A_cyclesLimit, false);
		lcd.print(F("  "));
		uiInstrument("D:", A_outPin, A_mode, A_state, A_halfCycles>>1, A_cyclesLimit, false);
	}
	else if(uiPage==1)
		uiInstrument("A ", A_outPin, A_mode, A_state, A_halfCycles>>1, A_cyclesLimit);
	else if(uiPage==2)
		uiInstrument("B ", A_outPin, A_mode, A_state, A_halfCycles>>1, A_cyclesLimit);
	else if(uiPage==3)
		uiInstrument("C ", A_outPin, A_mode, A_state, A_halfCycles>>1, A_cyclesLimit);
	else if(uiPage==4)
		uiInstrument("D ", A_outPin, A_mode, A_state, A_halfCycles>>1, A_cyclesLimit);
	else if(uiPage==5) {
		lcd.setCursor(0, 0);
		lcd.print(F("EMAIL   "));
		lcd.print(F("BCSEDLON@"));
		lcd.setCursor(0, 1);
		uiLcdPrintSpaces8();
		lcd.setCursor(7, 1);
		lcd.print(F("GMAIL.COM"));
	}



	//lcd.backlight();
	/*
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print('|');
	lcd.print('-');
	*/
}

/*
void listener(char ch) {
	Serial.println(ch);
}
*/


void loadEEPROM() {
    using namespace OMEEPROM;

    read(ADDR_A_MODE, A_mode);
    read(ADDR_A_INIT, A_init);
    read(ADDR_A_CYCLESLIMIT, A_cyclesLimit);
    read(ADDR_A_ONHOUR, A_onHour);
    read(ADDR_A_ONMIN, A_onMin);
    read(ADDR_A_ONSEC, A_onSec);
    read(ADDR_A_OFFHOUR, A_offHour);
    read(ADDR_A_OFFMIN, A_offMin);
    read(ADDR_A_OFFSEC, A_offSec);
}

void saveDefaultEEPROM() {
    using namespace OMEEPROM;

    A_mode = 0;
    A_init = false;
    A_cyclesLimit = 0;
    A_onHour = A_onMin = A_onSec = A_offHour = A_offMin = A_offSec =0;
    write(ADDR_A_MODE, A_mode);
    write(ADDR_A_INIT, A_init);
    write(ADDR_A_CYCLESLIMIT, A_cyclesLimit);
    write(ADDR_A_ONHOUR, A_onHour);
    write(ADDR_A_ONMIN, A_onMin);
    write(ADDR_A_ONSEC, A_onSec);
    write(ADDR_A_OFFHOUR, A_offHour);
    write(ADDR_A_OFFMIN, A_offMin);
    write(ADDR_A_OFFSEC, A_offSec);

}

//The setup function is called once at startup of the sketch
void setup()
{
	// Add your initialization code here
	Serial.begin(9600);
		while(!Serial);

	if( OMEEPROM::saved() )
		loadEEPROM();
	else
		saveDefaultEEPROM();

	Wire.begin( );
	kpd.begin( makeKeymap(keys) );
	//kpd.setDebounceTime(10);
	//kpd.setHoldTime(120);
	//kpd.addEventListener(listener);

	lcd.begin(LCD_COLS, LCD_ROWS);

	Menu.setDrawHandler(uiDraw);
	Menu.setExitHandler(uiMain);
	Menu.enable(true);

	uiMain();

	pinMode(PIN_LED, OUTPUT);

	// Timer0 is already used for millis() - we'll just interrupt somewhere
	// in the middle and call the "Compare A" function below
	//OCR0A = 0xAF;
	//TIMSK0 |= _BV(OCIE0A);
}





/*
// Interrupt is called once a millisecond,
SIGNAL(TIMER0_COMPA_vect)
{
	//run every sec
		if(X_state) {
			//running
			if(A_state) {
				if(A_out)
					A_set = A_onSec + A_onMin*60 + A_onHour*60*60;
				else
					A_set = A_offSec + A_offMin*60 + A_offHour*60*60;
				if(A_set > A_sec) {
					A_out = ~A_out;
					A_sec = 0;
				}
				else
					A_sec++;
			}

		}

}
*/


void uiResetParam() {
	saveDefaultEEPROM();
	loadEEPROM();
	X_restart();
	X_stop();
}


void uiMessage(const char* msg) {
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print(msg);
	delay(500);
}
void uiMessageStarted(){
	uiMessage(TEXT_STARTED);
}
void uiMessageStopped(){
	uiMessage(TEXT_STOPPED);
}
void uiMessageRestarted(){
	uiMessage(TEXT_RESTARTED);
}

void X_uiStart() {
	X_start();
	uiMessageStarted();
}
void X_uiStop() {
	X_stop();
	uiMessageStopped();
}
void X_uiRestart() {
	X_restart();
	uiMessageRestarted();
}
void A_uiStart() {
	A_start();
	uiMessageStarted();
}
void A_uiStop() {
	A_stop();
	uiMessageStopped();
}
void A_uiRestart() {
	A_restart();
	uiMessageRestarted();
}

void X_start() {
	X_state = 1;
	A_start();
}
void X_stop() {
	X_state = 0;
	A_stop();
}
void X_restart() {
	X_state = 1;
	A_restart();
}

void A_start() {
	A_state = 1;
	if(!A_halfCycles)
		A_out = A_init;
}
void A_stop() {
	A_state = 0;
}
void A_restart() {
	A_state = 1;
	A_out = A_init;
	A_sec = 0;
	A_halfCycles = 0;
}

bool getInstrumentControl(bool a, uint8_t mode) {
	if(mode == 0) return a;
	if(mode == 1) return false;
	if(mode == 2) return true;
	return false;
}



// The loop function is called in an endless loop
void loop()
{
	//Add your repeated code here
	//char key = kpd.getKey();
	char ch = kpd.getRawKey();
	if(ch == '*') //KPD_ENTER)
		Menu.enable(true);

	Menu.checkInput();


	if (secInterval.expired()) {
		secInterval.set(1000);
		secCnt++;

		if(X_state) {
			//running


			if(((A_halfCycles >> 1) >= A_cyclesLimit) && A_cyclesLimit) {
				A_state = 0;
				A_out = false;
			}
			if(A_state) { // && !A_mode
				if(A_out)
					A_set = A_onSec + A_onMin*60 + A_onHour*60*60;
				else
					A_set = A_offSec + A_offMin*60 + A_offHour*60*60;
				if(A_sec >= A_set) {
					A_out = !A_out;
					A_sec = 0;
					A_halfCycles++;
				}
				A_sec++;
			}

		}

		A_outPin = getInstrumentControl(A_out, A_mode);

		digitalWrite(PIN_LED, A_outPin);
	}

	if(!Menu.enable()) {
		uiMain();

		ch = kpd.getKey();
		if(ch =='A') //KPD_UP)
			uiPage--;
		if(ch == 'B') //KPD_DOWN)
			uiPage++;
		if(ch == '#') //KPD_DOWN)
			uiPage = 0;
		uiPage = max(0, uiPage);
		uiPage = min(5, uiPage);
	}
}
