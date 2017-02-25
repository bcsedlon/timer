#include "Arduino.h"

#include "libraries/Keypad_I2C/Keypad_I2C.h"
#include "libraries/Keypad/Keypad.h"
#include "libraries/NewliquidCrystal/LiquidCrystal_I2C.h"
#include "libraries/OMEEPROM/OMEEPROM.h"
#include "libraries/OMMenuMgr/OMMenuMgr.h"
#include "libraries/Interval/interval.h"

#define PIN_LED 13
#define PIN_A 9
#define PIN_B 10
#define PIN_C 11
#define PIN_D 12

#define PIN_BUZZER 4
#define PIN_CO2 A0


Interval secInterval;
unsigned long secCnt;
bool secOverflow;

uint8_t X_state;

uint8_t A_mode, A_state, A_init;
uint A_onHour, A_onMin, A_onSec, A_offHour, A_offMin, A_offSec, A_offsetSec;
unsigned long A_cyclesLimit, A_halfCycles;
unsigned long A_set, A_sec;
bool  A_out, A_outPin;

uint8_t B_mode, B_state, B_init;
uint B_onHour, B_onMin, B_onSec, B_offHour, B_offMin, B_offSec, B_offsetSec;
unsigned long B_cyclesLimit, B_halfCycles;
unsigned long B_set, B_sec;
bool  B_out, B_outPin;

uint8_t C_mode, C_state, C_init;
uint C_onHour, C_onMin, C_onSec, C_offHour, C_offMin, C_offSec, C_offsetSec;
unsigned long C_cyclesLimit, C_halfCycles;
unsigned long C_set, C_sec;
bool  C_out, C_outPin;

uint8_t D_mode, D_state, D_init;
uint D_onHour, D_onMin, D_onSec, D_offHour, D_offMin, D_offSec, D_offsetSec;
unsigned long D_cyclesLimit, D_halfCycles;
unsigned long D_set, D_sec;
bool  D_out, D_outPin;

int uiPage;

/*
double co2;
char text[16];
char* text1 = "0123456789ABCDEF";
*/

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

class Alarm {
	bool activated;
	bool deactivated;
	Interval activeInterval;
	Interval deactiveInterval;

public:
	int alarmActiveDelay = 5000;
	int alarmDeactiveDelay = 5000;
	bool active;
	bool unAck;

	bool activate(bool state) {
		if(state) {
			if(!active) {
				if(!activated) {
					activated = true;
					activeInterval.set(alarmActiveDelay);
				}
				if(activeInterval.expired()) {
					activated = false;
					active = true;
					unAck = true;
					return true;
				}
			}
		}
		else
			activated = false;
		return false;
	};
	bool deactivate(bool state) {
		if(state){
			if(active) {
				if(!deactivated) {
					deactivated = true;
					deactiveInterval.set(alarmDeactiveDelay);
				}
				if(deactiveInterval.expired()) {
					deactivated = false;
					active = false;
					return true;
				}
			}
		}
		else
			deactivated = false;
		return false;
	}
	void ack() {
		unAck = false;
	}
};

Alarm co2LowAlarm;

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
    	if(bitMap[3] == 4) {
    		if(bitMap[2] == 8) A_mode=1;
    		if(bitMap[1] == 8) A_mode=2;
    		if(bitMap[0] == 8) A_mode=0;

    		if(bitMap[2] == 4) B_mode=1;
    		if(bitMap[1] == 4) B_mode=2;
    		if(bitMap[0] == 4) B_mode=0;

    		if(bitMap[2] == 2) C_mode=1;
    		if(bitMap[1] == 2) C_mode=2;
    		if(bitMap[0] == 2) C_mode=0;

    		if(bitMap[2] == 1) D_mode=1;
    		if(bitMap[1] == 1) D_mode=2;
    		if(bitMap[0] == 1) D_mode=0;
    		//return NO_KEY;
    	}
    	/*
    	if(bitMap[3] == 2) {
    		if(bitMap[2] == 8) A_start();
    	    if(bitMap[1] == 8) A_stop();
    	    if(bitMap[0] == 8) A_restart();

    	    if(bitMap[2] == 4) B_start();
    	    if(bitMap[1] == 4) B_stop();
    	    if(bitMap[0] == 4) B_restart();

    	    if(bitMap[2] == 2) C_start();
    	    if(bitMap[1] == 2) C_stop();
    	    if(bitMap[0] == 2) C_restart();

    	    if(bitMap[2] == 1) D_start();
    	    if(bitMap[1] == 1) D_stop();
    	    if(bitMap[0] == 1) D_restart();
    	    //return NO_KEY;
    	}
    	 */

    	//Serial.println(kTime);
    	// for menu system, makes delay between first and next
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
#define TEXT_RESET_ALL_EM 	"RESTART ALL!"
#define TEXT_RESETPARAM_EM 		"RESET SETTING!"
#define TEXT_RESETTED			"RESETTED"

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
#define TEXT_A_START_EM 	"A START!"
#define TEXT_A_STOP_EM 		"A STOP!"
#define TEXT_A_RESET_EM 	"A RESTART!"
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
#define TEXT_SETWIFIPASS		"WIFI PASSWORD:"


//CZ
#define TEXT_AUTO_EM			"AUTO!"
#define TEXT_OFF_EM				"VYP!"
#define TEXT_ON_EM 				"ZAP!"
#define TEXT_START_EM 			"STOP!"
#define TEXT_STOP_EM 			"START!"
#define TEXT_START_ALL_EM 		"START!"
#define TEXT_STOP_ALL_EM 		"STOP!"
#define TEXT_RESET_ALL_EM 		"RESET!"
#define TEXT_SETPARAM1_EM 		"NASTAVENI 1!"
#define TEXT_SETPARAM2_EM 		"NASTAVENI 2!"
#define TEXT_SETPARAM3_EM 		"NASTAVENI 3!"
#define TEXT_RESETTED			"NASTAVENO"
#define TEXT_OFF3				"VYP"
#define TEXT_ON3 				"ZAP"

#define TEXT_PASSIVE			"PASIVNI"
#define TEXT_ACTIVE				"AKTIVNI "
#define TEXT_STARTED			"SPUSTENO"
#define TEXT_STOPPED			"ZASTAVENO"
#define TEXT_RESTARTED			"RESETOVANO"
#define TEXT_OFF				"0 "
#define TEXT_ON 				"1 "
#define TEXT_MAN 				"MAN "
#define TEXT_AUTO 				"AUT "
#define TEXT_PAS				"VYP "
#define TEXT_ACT				"ZAP "
#define TEXT_STOP 				"STOP"
#define TEXT_RUN 				"CHOD"
#define TEXT_OFF2				"0"
#define TEXT_ON2 				"1"
#define TEXT_MAN2 				"M"
#define TEXT_AUTO2 				"A"
#define TEXT_PAS2				"P"
#define TEXT_ACT2				"A"
#define TEXT_STOP2 				"STOP"
#define TEXT_RUN2 				"CHOD"

// A
#define TEXT_A_ROOT				"A NASTAVENI->"
#define TEXT_A_START_EM 		"A START!"
#define TEXT_A_STOP_EM 			"A STOP!"
#define TEXT_A_RESET_EM 		"A RESET!"
#define TEXT_A_INIT				"A VYCHOZI:"
#define TEXT_A_MODE				"A RELE:"
#define TEXT_A_STATE			"A STAV:"
#define TEXT_A_ONHOUR			"A ZAP:    [HOD]"
#define TEXT_A_ONMIN			"A ZAP:    [MIN]"
#define TEXT_A_ONSEC			"A ZAP:    [SEC]"
#define TEXT_A_OFFHOUR			"A VYP:    [HOD]"
#define TEXT_A_OFFMIN			"A VYP:    [MIN]"
#define TEXT_A_OFFSEC			"A VYP:	   [SEC]"
#define TEXT_A_CYCLELIMIT		"A POCET CYKLU: "
#define TEXT_A_OFFSETSEC		"A OFFSET: [SEC]"

// B
#define TEXT_B_ROOT				"B NASTAVENI->"
#define TEXT_B_START_EM 	"B START!"
#define TEXT_B_STOP_EM 		"B STOP!"
#define TEXT_B_RESET_EM 	"B RESET!"
#define TEXT_B_INIT				"B VYCHOZI:"
#define TEXT_B_MODE				"B RELE:"
#define TEXT_B_STATE			"B STAV:"
#define TEXT_B_ONHOUR			"B ZAP:    [HOD]"
#define TEXT_B_ONMIN			"B ZAP:    [MIN]"
#define TEXT_B_ONSEC			"B ZAP:    [SEC]"
#define TEXT_B_OFFHOUR			"B VYP:    [HOD]"
#define TEXT_B_OFFMIN			"B VYP:    [MIN]"
#define TEXT_B_OFFSEC			"B VYP:	   [SEC]"
#define TEXT_B_CYCLELIMIT		"B POCET CYKLU: "
#define TEXT_B_OFFSETSEC		"B OFFSET: [SEC]"

// C
#define TEXT_C_ROOT				"C NASTAVENI->"
#define TEXT_C_START_EM 	"C START!"
#define TEXT_C_STOP_EM 		"C STOP!"
#define TEXT_C_RESET_EM 	"C RESET!"
#define TEXT_C_INIT				"C VYCHOZI:"
#define TEXT_C_MODE				"C RELE:"
#define TEXT_C_STATE			"C STAV:"
#define TEXT_C_ONHOUR			"C ZAP:    [HOD]"
#define TEXT_C_ONMIN			"C ZAP:    [MIN]"
#define TEXT_C_ONSEC			"C ZAP:    [SEC]"
#define TEXT_C_OFFHOUR			"C VYP:    [HOD]"
#define TEXT_C_OFFMIN			"C VYP:    [MIN]"
#define TEXT_C_OFFSEC			"C VYP:	   [SEC]"
#define TEXT_C_CYCLELIMIT		"C POCET CYKLU: "
#define TEXT_C_OFFSETSEC		"C OFFSET: [SEC]"

// D
#define TEXT_D_ROOT				"D NASTAVENI->"
#define TEXT_D_START_EM 	"D START!"
#define TEXT_D_STOP_EM 		"D STOP!"
#define TEXT_D_RESET_EM 	"D RESET!"
#define TEXT_D_INIT				"D VYCHOZI:"
#define TEXT_D_MODE				"D RELE:"
#define TEXT_D_STATE			"D STAV:"
#define TEXT_D_ONHOUR			"D ZAP:    [HOD]"
#define TEXT_D_ONMIN			"D ZAP:    [MIN]"
#define TEXT_D_ONSEC			"D ZAP:    [SEC]"
#define TEXT_D_OFFHOUR			"D VYP:    [HOD]"
#define TEXT_D_OFFMIN			"D VYP:    [MIN]"
#define TEXT_D_OFFSEC			"D VYP:	   [SEC]"
#define TEXT_D_CYCLELIMIT		"D POCET CYKLU: "
#define TEXT_D_OFFSETSEC		"D OFFSET: [SEC]"


//EEPROM

// A
#define ADDR_A_MODE 		20
#define ADDR_A_INIT 		24
#define ADDR_A_ONHOUR 		28
#define ADDR_A_ONMIN 		32
#define ADDR_A_ONSEC 		36
#define ADDR_A_OFFHOUR 		40
#define ADDR_A_OFFMIN 		44
#define ADDR_A_OFFSEC 		48
#define ADDR_A_STATE 		52
#define ADDR_A_CYCLESLIMIT	56	// 8 bytes ULONG
#define ADDR_A_OFFSETSEC	64

// B
#define ADDR_B_MODE 		80
#define ADDR_B_INIT 		84
#define ADDR_B_ONHOUR 		88
#define ADDR_B_ONMIN 		92
#define ADDR_B_ONSEC 		96
#define ADDR_B_OFFHOUR 		100
#define ADDR_B_OFFMIN 		104
#define ADDR_B_OFFSEC 		108
#define ADDR_B_STATE 		112
#define ADDR_B_CYCLESLIMIT	116	// 8 bytes ULONG
#define ADDR_B_OFFSETSEC	124

// C
#define ADDR_C_MODE 		140
#define ADDR_C_INIT 		144
#define ADDR_C_ONHOUR 		148
#define ADDR_C_ONMIN 		152
#define ADDR_C_ONSEC 		156
#define ADDR_C_OFFHOUR 		160
#define ADDR_C_OFFMIN 		164
#define ADDR_C_OFFSEC 		168
#define ADDR_C_STATE 		172
#define ADDR_C_CYCLESLIMIT	176	// 8 bytes ULONG
#define ADDR_C_OFFSETSEC	184

// D
#define ADDR_D_MODE 		200
#define ADDR_D_INIT 		204
#define ADDR_D_ONHOUR 		208
#define ADDR_D_ONMIN 		212
#define ADDR_D_ONSEC 		216
#define ADDR_D_OFFHOUR 		220
#define ADDR_D_OFFMIN 		224
#define ADDR_D_OFFSEC 		228
#define ADDR_D_STATE 		232
#define ADDR_D_CYCLESLIMIT	236	// 8 bytes ULONG
#define ADDR_D_OFFSETSEC	244

// Create a list of states and values for a select input
MENU_SELECT_ITEM  sel_off2 = { 0, {TEXT_OFF3} };
MENU_SELECT_ITEM  sel_on2  = { 1, {TEXT_ON3} };

MENU_SELECT_ITEM  sel_auto= { 0, {TEXT_AUTO_EM} };
MENU_SELECT_ITEM  sel_off = { 1, {TEXT_OFF_EM} };
MENU_SELECT_ITEM  sel_on  = { 2, {TEXT_ON_EM} };
//MENU_SELECT_ITEM  sel_rc_disable={ 3, {"DISABLE RADIO"} };
//MENU_SELECT_ITEM  sel_rcon= { 3, {"RC ON"} };
MENU_SELECT_ITEM  sel_stop= { 0, {TEXT_START_EM} };
MENU_SELECT_ITEM  sel_start={ 1, {TEXT_STOP_EM} };

MENU_SELECT_ITEM  sel_passive={ 0, {TEXT_PASSIVE} };
MENU_SELECT_ITEM  sel_active= { 1, {TEXT_ACTIVE} };

MENU_SELECT_LIST  const stateOffOn2_list[] = { &sel_off2, &sel_on2 };
MENU_SELECT_LIST  const stateMode_list[] = { &sel_auto, &sel_off, &sel_on};
MENU_SELECT_LIST  const stateOffOn_list[] = { &sel_off2, &sel_on2 };
MENU_SELECT_LIST  const statePasAct_list[] = { &sel_passive, &sel_active };
//MENU_SELECT_LIST  const stateStopStart_list[] = { &sel_stop, &sel_start};

MENU_ITEM X_start_item   = { {TEXT_START_ALL_EM},  ITEM_ACTION, 0,        MENU_TARGET(&X_uiStart) };
MENU_ITEM X_stop_item   =  { {TEXT_STOP_ALL_EM },  ITEM_ACTION, 0,        MENU_TARGET(&X_uiStop)  };
MENU_ITEM X_restart_item  ={ {TEXT_RESET_ALL_EM},  ITEM_ACTION, 0,        MENU_TARGET(&X_uiRestart) };

MENU_ITEM setParam1_item  ={ {TEXT_SETPARAM1_EM},  ITEM_ACTION, 0,        MENU_TARGET(&uiSetParam1) };
MENU_ITEM setParam2_item  ={ {TEXT_SETPARAM2_EM},  ITEM_ACTION, 0,        MENU_TARGET(&uiSetParam2) };
MENU_ITEM setParam3_item  ={ {TEXT_SETPARAM3_EM},  ITEM_ACTION, 0,        MENU_TARGET(&uiSetParam3) };
//MENU_ITEM setWifiPass_item  ={ {TEXT_SETWIFIPASS},  ITEM_ACTION, 0,        MENU_TARGET(&uiSetWifiPass) };

//MENU_SELECT A_state_select = { &A_state,           MENU_SELECT_SIZE(stateStopStart_list),   MENU_TARGET(&stateStopStart_list) };
//MENU_VALUE A_state_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&A_state_select), 0};
//MENU_ITEM A_state_item    =  { {"A STATE:"}, ITEM_VALUE,  0,        MENU_TARGET(&A_state_value) };

////////////////////////////////////////
// A
////////////////////////////////////////
MENU_SELECT A_init_select = { &A_init,           MENU_SELECT_SIZE(stateOffOn2_list),   MENU_TARGET(&stateOffOn2_list) };
MENU_VALUE A_init_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&A_init_select), ADDR_A_INIT};
MENU_ITEM A_init_item    =  { {TEXT_A_INIT}, ITEM_VALUE,  0,        MENU_TARGET(&A_init_value)};

MENU_ITEM A_start_item   = { {TEXT_A_START_EM},  ITEM_ACTION, 0,        MENU_TARGET(&A_uiStart) };
MENU_ITEM A_stop_item    = { {TEXT_A_STOP_EM },  ITEM_ACTION, 0,        MENU_TARGET(&A_uiStop) };
MENU_ITEM A_restart_item = { {TEXT_A_RESET_EM},  ITEM_ACTION, 0,        MENU_TARGET(&A_uiRestart) };

MENU_SELECT A_mode_select = { &A_mode,           MENU_SELECT_SIZE(stateMode_list),   MENU_TARGET(&stateMode_list) };
MENU_VALUE A_mode_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&A_mode_select), ADDR_A_MODE};
MENU_ITEM A_mode_item    =  { {TEXT_A_MODE}, ITEM_VALUE,  0,        MENU_TARGET(&A_mode_value) };

MENU_SELECT A_state_select = { &A_state,           MENU_SELECT_SIZE(stateOffOn_list),   MENU_TARGET(&stateOffOn_list) };
MENU_VALUE A_state_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&A_state_select), ADDR_A_STATE};
MENU_ITEM A_state_item    =  { {TEXT_A_STATE}, ITEM_VALUE,  0,        MENU_TARGET(&A_state_value) };
//                               TYPE             MAX    MIN    TARGET
MENU_VALUE A_onHour_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&A_onHour), ADDR_A_ONHOUR};
MENU_VALUE A_onMin_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&A_onMin), ADDR_A_ONMIN};
MENU_VALUE A_onSec_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&A_onSec), ADDR_A_ONSEC};
MENU_VALUE A_offHour_value ={ TYPE_UINT,       0,    0,     MENU_TARGET(&A_offHour), ADDR_A_OFFHOUR};
MENU_VALUE A_offMin_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&A_offMin), ADDR_A_OFFMIN};
MENU_VALUE A_offSec_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&A_offSec), ADDR_A_OFFSEC};
MENU_VALUE A_cyclesLimit_value = { TYPE_ULONG,       0,    0,     MENU_TARGET(&A_cyclesLimit), ADDR_A_CYCLESLIMIT};
MENU_VALUE A_offsetSec_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&A_offsetSec), ADDR_A_OFFSETSEC};

//                                "123456789ABCDEF"
MENU_ITEM A_onHour_item   = { {TEXT_A_ONHOUR},   ITEM_VALUE,  0,        MENU_TARGET(&A_onHour_value) };
MENU_ITEM A_onMin_item   =  { {TEXT_A_ONMIN},   ITEM_VALUE,  0,        MENU_TARGET(&A_onMin_value)  };
MENU_ITEM A_onSec_item   =  { {TEXT_A_ONSEC},   ITEM_VALUE,  0,        MENU_TARGET(&A_onSec_value)  };
MENU_ITEM A_offHour_item  = { {TEXT_A_OFFHOUR},   ITEM_VALUE,  0,        MENU_TARGET(&A_offHour_value)};
MENU_ITEM A_offMin_item   = { {TEXT_A_OFFMIN},   ITEM_VALUE,  0,        MENU_TARGET(&A_offMin_value) };
MENU_ITEM A_offSec_item   = { {TEXT_A_OFFSEC},   ITEM_VALUE,  0,        MENU_TARGET(&A_offSec_value) };
MENU_ITEM A_cyclesLimit_item   = { {TEXT_A_CYCLELIMIT},   ITEM_VALUE,  0,        MENU_TARGET(&A_cyclesLimit_value) };
MENU_ITEM A_offsetSec_item   = { {TEXT_A_OFFSETSEC},   ITEM_VALUE,  0,        MENU_TARGET(&A_offsetSec_value) };

//MENU_LIST const A_submenu_list[] = { &A_state_item, &A_init_item, &A_cyclesLimit_item, &A_restart_item, &A_onSec_item,&A_onMin_item, &A_onHour_item, &A_offSec_item,&A_offMin_item, &A_offHour_item, &A_offsetSec_item, &A_mode_item}; //&A_start_item, &A_stop_item,
MENU_LIST const A_submenu_list[] = { &A_init_item, &A_cyclesLimit_item, &A_onSec_item,&A_onMin_item, &A_onHour_item, &A_offSec_item,&A_offMin_item, &A_offHour_item, &A_offsetSec_item, &A_mode_item}; //&A_start_item, &A_stop_item,
MENU_ITEM A_menu_submenu = { {TEXT_A_ROOT},  ITEM_MENU,  MENU_SIZE(A_submenu_list),  MENU_TARGET(&A_submenu_list) };

////////////////////////////////////////
// B
////////////////////////////////////////
MENU_SELECT B_init_select = { &B_init,           MENU_SELECT_SIZE(stateOffOn2_list),   MENU_TARGET(&stateOffOn2_list) };
MENU_VALUE B_init_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&B_init_select), ADDR_B_INIT};
MENU_ITEM B_init_item    =  { {TEXT_B_INIT}, ITEM_VALUE,  0,        MENU_TARGET(&B_init_value)};

MENU_ITEM B_start_item   = { {TEXT_B_START_EM},  ITEM_ACTION, 0,        MENU_TARGET(&B_uiStart) };
MENU_ITEM B_stop_item    = { {TEXT_B_STOP_EM },  ITEM_ACTION, 0,        MENU_TARGET(&B_uiStop) };
MENU_ITEM B_restart_item = { {TEXT_B_RESET_EM},  ITEM_ACTION, 0,        MENU_TARGET(&B_uiRestart) };

MENU_SELECT B_mode_select = { &B_mode,           MENU_SELECT_SIZE(stateMode_list),   MENU_TARGET(&stateMode_list) };
MENU_VALUE B_mode_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&B_mode_select), ADDR_B_MODE};
MENU_ITEM B_mode_item    =  { {TEXT_B_MODE}, ITEM_VALUE,  0,        MENU_TARGET(&B_mode_value) };

MENU_SELECT B_state_select = { &B_state,           MENU_SELECT_SIZE(stateOffOn_list),   MENU_TARGET(&stateOffOn_list) };
MENU_VALUE B_state_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&B_state_select), ADDR_B_STATE};
MENU_ITEM B_state_item    =  { {TEXT_B_STATE}, ITEM_VALUE,  0,        MENU_TARGET(&B_state_value) };
//                               TYPE             MAX    MIN    TARGET
MENU_VALUE B_onHour_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&B_onHour), ADDR_B_ONHOUR};
MENU_VALUE B_onMin_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&B_onMin), ADDR_B_ONMIN};
MENU_VALUE B_onSec_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&B_onSec), ADDR_B_ONSEC};
MENU_VALUE B_offHour_value ={ TYPE_UINT,       0,    0,     MENU_TARGET(&B_offHour), ADDR_B_OFFHOUR};
MENU_VALUE B_offMin_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&B_offMin), ADDR_B_OFFMIN};
MENU_VALUE B_offSec_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&B_offSec), ADDR_B_OFFSEC};
MENU_VALUE B_cyclesLimit_value = { TYPE_ULONG,       0,    0,     MENU_TARGET(&B_cyclesLimit), ADDR_B_CYCLESLIMIT};
MENU_VALUE B_offsetSec_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&B_offsetSec), ADDR_B_OFFSETSEC};

//                                "123456789ABCDEF"
MENU_ITEM B_onHour_item   = { {TEXT_B_ONHOUR},   ITEM_VALUE,  0,        MENU_TARGET(&B_onHour_value) };
MENU_ITEM B_onMin_item   =  { {TEXT_B_ONMIN},   ITEM_VALUE,  0,        MENU_TARGET(&B_onMin_value)  };
MENU_ITEM B_onSec_item   =  { {TEXT_B_ONSEC},   ITEM_VALUE,  0,        MENU_TARGET(&B_onSec_value)  };
MENU_ITEM B_offHour_item  = { {TEXT_B_OFFHOUR},   ITEM_VALUE,  0,        MENU_TARGET(&B_offHour_value)};
MENU_ITEM B_offMin_item   = { {TEXT_B_OFFMIN},   ITEM_VALUE,  0,        MENU_TARGET(&B_offMin_value) };
MENU_ITEM B_offSec_item   = { {TEXT_B_OFFSEC},   ITEM_VALUE,  0,        MENU_TARGET(&B_offSec_value) };
MENU_ITEM B_cyclesLimit_item   = { {TEXT_B_CYCLELIMIT},   ITEM_VALUE,  0,        MENU_TARGET(&B_cyclesLimit_value) };
MENU_ITEM B_offsetSec_item   = { {TEXT_B_OFFSETSEC},   ITEM_VALUE,  0,        MENU_TARGET(&B_offsetSec_value) };

//MENU_LIST const B_submenu_list[] = { &B_state_item, &B_init_item, &B_cyclesLimit_item, &B_restart_item, &B_onSec_item,&B_onMin_item, &B_onHour_item, &B_offSec_item,&B_offMin_item, &B_offHour_item, &B_offsetSec_item, &B_mode_item}; //&B_start_item, &B_stop_item,
MENU_LIST const B_submenu_list[] = { &B_init_item, &B_cyclesLimit_item,  &B_onSec_item,&B_onMin_item, &B_onHour_item, &B_offSec_item,&B_offMin_item, &B_offHour_item, &B_offsetSec_item, &B_mode_item}; //&B_start_item, &B_stop_item,
MENU_ITEM B_menu_submenu = { {TEXT_B_ROOT},  ITEM_MENU,  MENU_SIZE(B_submenu_list),  MENU_TARGET(&B_submenu_list) };

////////////////////////////////////////
// C
////////////////////////////////////////
MENU_SELECT C_init_select = { &C_init,           MENU_SELECT_SIZE(stateOffOn2_list),   MENU_TARGET(&stateOffOn2_list) };
MENU_VALUE C_init_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&C_init_select), ADDR_C_INIT};
MENU_ITEM C_init_item    =  { {TEXT_C_INIT}, ITEM_VALUE,  0,        MENU_TARGET(&C_init_value)};

MENU_ITEM C_start_item   = { {TEXT_C_START_EM},  ITEM_ACTION, 0,        MENU_TARGET(&C_uiStart) };
MENU_ITEM C_stop_item    = { {TEXT_C_STOP_EM },  ITEM_ACTION, 0,        MENU_TARGET(&C_uiStop) };
MENU_ITEM C_restart_item = { {TEXT_C_RESET_EM},  ITEM_ACTION, 0,        MENU_TARGET(&C_uiRestart) };

MENU_SELECT C_mode_select = { &C_mode,           MENU_SELECT_SIZE(stateMode_list),   MENU_TARGET(&stateMode_list) };
MENU_VALUE C_mode_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&C_mode_select), ADDR_C_MODE};
MENU_ITEM C_mode_item    =  { {TEXT_C_MODE}, ITEM_VALUE,  0,        MENU_TARGET(&C_mode_value) };

MENU_SELECT C_state_select = { &C_state,           MENU_SELECT_SIZE(stateOffOn_list),   MENU_TARGET(&stateOffOn_list) };
MENU_VALUE C_state_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&C_state_select), ADDR_C_STATE};
MENU_ITEM C_state_item    =  { {TEXT_C_STATE}, ITEM_VALUE,  0,        MENU_TARGET(&C_state_value) };
//                               TYPE             MAX    MIN    TARGET
MENU_VALUE C_onHour_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&C_onHour), ADDR_C_ONHOUR};
MENU_VALUE C_onMin_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&C_onMin), ADDR_C_ONMIN};
MENU_VALUE C_onSec_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&C_onSec), ADDR_C_ONSEC};
MENU_VALUE C_offHour_value ={ TYPE_UINT,       0,    0,     MENU_TARGET(&C_offHour), ADDR_C_OFFHOUR};
MENU_VALUE C_offMin_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&C_offMin), ADDR_C_OFFMIN};
MENU_VALUE C_offSec_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&C_offSec), ADDR_C_OFFSEC};
MENU_VALUE C_cyclesLimit_value = { TYPE_ULONG,       0,    0,     MENU_TARGET(&C_cyclesLimit), ADDR_C_CYCLESLIMIT};
MENU_VALUE C_offsetSec_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&C_offsetSec), ADDR_C_OFFSETSEC};

//                                "123456789ABCDEF"
MENU_ITEM C_onHour_item   = { {TEXT_C_ONHOUR},   ITEM_VALUE,  0,        MENU_TARGET(&C_onHour_value) };
MENU_ITEM C_onMin_item   =  { {TEXT_C_ONMIN},   ITEM_VALUE,  0,        MENU_TARGET(&C_onMin_value)  };
MENU_ITEM C_onSec_item   =  { {TEXT_C_ONSEC},   ITEM_VALUE,  0,        MENU_TARGET(&C_onSec_value)  };
MENU_ITEM C_offHour_item  = { {TEXT_C_OFFHOUR},   ITEM_VALUE,  0,        MENU_TARGET(&C_offHour_value)};
MENU_ITEM C_offMin_item   = { {TEXT_C_OFFMIN},   ITEM_VALUE,  0,        MENU_TARGET(&C_offMin_value) };
MENU_ITEM C_offSec_item   = { {TEXT_C_OFFSEC},   ITEM_VALUE,  0,        MENU_TARGET(&C_offSec_value) };
MENU_ITEM C_cyclesLimit_item   = { {TEXT_C_CYCLELIMIT},   ITEM_VALUE,  0,        MENU_TARGET(&C_cyclesLimit_value) };
MENU_ITEM C_offsetSec_item   = { {TEXT_C_OFFSETSEC},   ITEM_VALUE,  0,        MENU_TARGET(&C_offsetSec_value) };

//MENU_LIST const C_submenu_list[] = { &C_state_item, &C_init_item, &C_cyclesLimit_item, &C_restart_item, &C_onSec_item,&C_onMin_item, &C_onHour_item, &C_offSec_item,&C_offMin_item, &C_offHour_item, &C_offsetSec_item, &C_mode_item}; //&C_start_item, &C_stop_item,
MENU_LIST const C_submenu_list[] = { &C_init_item, &C_cyclesLimit_item, &C_onSec_item,&C_onMin_item, &C_onHour_item, &C_offSec_item,&C_offMin_item, &C_offHour_item, &C_offsetSec_item, &C_mode_item}; //&C_start_item, &C_stop_item,
MENU_ITEM C_menu_submenu = { {TEXT_C_ROOT},  ITEM_MENU,  MENU_SIZE(C_submenu_list),  MENU_TARGET(&C_submenu_list) };

////////////////////////////////////////
// D
////////////////////////////////////////
MENU_SELECT D_init_select = { &D_init,           MENU_SELECT_SIZE(stateOffOn2_list),   MENU_TARGET(&stateOffOn2_list) };
MENU_VALUE D_init_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&D_init_select), ADDR_D_INIT};
MENU_ITEM D_init_item    =  { {TEXT_D_INIT}, ITEM_VALUE,  0,        MENU_TARGET(&D_init_value)};

MENU_ITEM D_start_item   = { {TEXT_D_START_EM},  ITEM_ACTION, 0,        MENU_TARGET(&D_uiStart) };
MENU_ITEM D_stop_item    = { {TEXT_D_STOP_EM },  ITEM_ACTION, 0,        MENU_TARGET(&D_uiStop) };
MENU_ITEM D_restart_item = { {TEXT_D_RESET_EM},  ITEM_ACTION, 0,        MENU_TARGET(&D_uiRestart) };

MENU_SELECT D_mode_select = { &D_mode,           MENU_SELECT_SIZE(stateMode_list),   MENU_TARGET(&stateMode_list) };
MENU_VALUE D_mode_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&D_mode_select), ADDR_D_MODE};
MENU_ITEM D_mode_item    =  { {TEXT_D_MODE}, ITEM_VALUE,  0,        MENU_TARGET(&D_mode_value) };

MENU_SELECT D_state_select = { &D_state,           MENU_SELECT_SIZE(stateOffOn_list),   MENU_TARGET(&stateOffOn_list) };
MENU_VALUE D_state_value =   { TYPE_SELECT,     0,     0,     MENU_TARGET(&D_state_select), ADDR_D_STATE};
MENU_ITEM D_state_item    =  { {TEXT_D_STATE}, ITEM_VALUE,  0,        MENU_TARGET(&D_state_value) };
//                               TYPE             MAX    MIN    TARGET
MENU_VALUE D_onHour_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&D_onHour), ADDR_D_ONHOUR};
MENU_VALUE D_onMin_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&D_onMin), ADDR_D_ONMIN};
MENU_VALUE D_onSec_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&D_onSec), ADDR_D_ONSEC};
MENU_VALUE D_offHour_value ={ TYPE_UINT,       0,    0,     MENU_TARGET(&D_offHour), ADDR_D_OFFHOUR};
MENU_VALUE D_offMin_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&D_offMin), ADDR_D_OFFMIN};
MENU_VALUE D_offSec_value = { TYPE_UINT,       0,    0,     MENU_TARGET(&D_offSec), ADDR_D_OFFSEC};
MENU_VALUE D_cyclesLimit_value = { TYPE_ULONG,       0,    0,     MENU_TARGET(&D_cyclesLimit), ADDR_D_CYCLESLIMIT};
MENU_VALUE D_offsetSec_value =  { TYPE_UINT,       0,    0,     MENU_TARGET(&D_offsetSec), ADDR_D_OFFSETSEC};

//                                "123456789ABCDEF"
MENU_ITEM D_onHour_item   = { {TEXT_D_ONHOUR},   ITEM_VALUE,  0,        MENU_TARGET(&D_onHour_value) };
MENU_ITEM D_onMin_item   =  { {TEXT_D_ONMIN},   ITEM_VALUE,  0,        MENU_TARGET(&D_onMin_value)  };
MENU_ITEM D_onSec_item   =  { {TEXT_D_ONSEC},   ITEM_VALUE,  0,        MENU_TARGET(&D_onSec_value)  };
MENU_ITEM D_offHour_item  = { {TEXT_D_OFFHOUR},   ITEM_VALUE,  0,        MENU_TARGET(&D_offHour_value)};
MENU_ITEM D_offMin_item   = { {TEXT_D_OFFMIN},   ITEM_VALUE,  0,        MENU_TARGET(&D_offMin_value) };
MENU_ITEM D_offSec_item   = { {TEXT_D_OFFSEC},   ITEM_VALUE,  0,        MENU_TARGET(&D_offSec_value) };
MENU_ITEM D_cyclesLimit_item   = { {TEXT_D_CYCLELIMIT},   ITEM_VALUE,  0,        MENU_TARGET(&D_cyclesLimit_value) };
MENU_ITEM D_offsetSec_item   = { {TEXT_D_OFFSETSEC},   ITEM_VALUE,  0,        MENU_TARGET(&D_offsetSec_value) };

//MENU_LIST const D_submenu_list[] = { &D_state_item, &D_init_item, &D_cyclesLimit_item, &D_restart_item, &D_onSec_item,&D_onMin_item, &D_onHour_item, &D_offSec_item,&D_offMin_item, &D_offHour_item, &D_offsetSec_item, &D_mode_item}; //&D_start_item, &D_stop_item,
MENU_LIST const D_submenu_list[] = { &D_init_item, &D_cyclesLimit_item, &D_onSec_item,&D_onMin_item, &D_onHour_item, &D_offSec_item,&D_offMin_item, &D_offHour_item, &D_offsetSec_item, &D_mode_item}; //&D_start_item, &D_stop_item,
MENU_ITEM D_menu_submenu = { {TEXT_D_ROOT},  ITEM_MENU,  MENU_SIZE(D_submenu_list),  MENU_TARGET(&D_submenu_list) };






MENU_LIST const root_list[]   = { &X_start_item, &X_stop_item, &X_restart_item, &A_menu_submenu, &B_menu_submenu, &C_menu_submenu, &D_menu_submenu, &setParam1_item, &setParam2_item, &setParam3_item };
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

void uiInstrumentSettingsOff(char* name, bool init, uint offSec, uint offMin, uint offHour, uint onSec, uint onMin, uint onHour, bool out, uint sec, bool detail=true) {
	//lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print(name);
	lcd.print(' ');
	//lcd.print(name);
	//lcd.print(offsetSec);
	lcd.print(TEXT_OFF3);
	lcd.print(' ');
	//lcd.setCursor(0, 0);
	//lcd.print("0:");
	lcd.print(offHour);
	lcd.print("H");
	lcd.print(offMin);
	lcd.print("M");
	lcd.print(offSec);
	lcd.print("S");
	uiLcdPrintSpaces8();

	lcd.setCursor(0, 1);
	if(!out) {
		lcd.print(sec);
		lcd.print("S");
	}
	/*
	lcd.setCursor(0, 1);
	lcd.print("  ");
	lcd.print(TEXT_ON3);
	lcd.print(' ');
	lcd.print(onHour);
	lcd.print("H");
	lcd.print(onMin);
	lcd.print("M");
	lcd.print(onSec);
	lcd.print("S");
	*/
	uiLcdPrintSpaces8();
	uiLcdPrintSpaces8();
}

void uiInstrumentSettingsOn(char* name, bool init, uint offSec, uint offMin, uint offHour, uint onSec, uint onMin, uint onHour, bool out, uint sec, bool detail=true) {
	//lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print(name);
	lcd.print(' ');
	//lcd.print(name);
	//lcd.print(offsetSec);

	lcd.print(TEXT_ON3);
	lcd.print(' ');
	//lcd.setCursor(0, 0);
	//lcd.print("0:");
	lcd.print(onHour);
	lcd.print("H");
	lcd.print(onMin);
	lcd.print("M");
	lcd.print(onSec);
	lcd.print("S");
	uiLcdPrintSpaces8();

	lcd.setCursor(0, 1);
	if(out) {
		lcd.print(sec);
		lcd.print("S");
	}
	/*
	lcd.setCursor(0, 1);
	lcd.print("  ");
	lcd.print(TEXT_ON3);
	lcd.print(' ');
	lcd.print(onHour);
	lcd.print("H");
	lcd.print(onMin);
	lcd.print("M");
	lcd.print(onSec);
	lcd.print("S");
	*/
	uiLcdPrintSpaces8();
	uiLcdPrintSpaces8();
}

void uiInstrument(char* name, bool out, uint8_t mode, uint8_t state, unsigned long cycles, unsigned long cyclesLimit, bool detail=true) {

	if(detail)
		lcd.setCursor(0, 0);
	lcd.print(name);


	if(!detail) {
		if(secCnt & 1)
			lcd.print(':');
		else
			lcd.print(' ');
	}

	if(detail) {
		lcd.print(' ');
		if(out)
			lcd.print(F(TEXT_ACT));
		else
			lcd.print(F(TEXT_PAS));
		if(mode)
			lcd.print(F(TEXT_MAN));
		else
			lcd.print(F(TEXT_AUTO));
		/*
		if(state)
			lcd.print(F(TEXT_ACT));
		else
			lcd.print(F(TEXT_PAS));
		*/
		if(X_state)
			lcd.print(F(TEXT_RUN));
		else
			lcd.print(F(TEXT_STOP));
		uiLcdPrintSpaces8();
	}
	else {
		if(out)
			lcd.print(F(TEXT_ON2));
		else
			lcd.print(F(TEXT_OFF2));
		if(mode)
			lcd.print(F(TEXT_MAN2));
		else
			lcd.print(F(TEXT_AUTO2));
	}
	if(detail) {
		lcd.setCursor(0, 1);
		lcd.print(cycles);

		if(secCnt & 1)
			lcd.print(':');
		else
			lcd.print(' ');

		lcd.print(cyclesLimit);
		uiLcdPrintSpaces8();
		uiLcdPrintSpaces8();
	}
}

void uiInstrumentX(char* name, bool out, uint8_t mode, uint8_t state, unsigned long cycles, unsigned long cyclesLimit, bool detail=true) {
	//lcd.clear();
	if(detail)
		lcd.setCursor(0, 0);
	lcd.print(name);
	if(!detail)
		if(secCnt & 1)
			lcd.print(':');
		else
			lcd.print(' ');


	if(detail) {
		//if(A_outPin)
		if(out)
			lcd.print(F(TEXT_ON));
		else
			lcd.print(F(TEXT_OFF));
		if(mode)
			lcd.print(F(TEXT_MAN));
		else
			lcd.print(F(TEXT_AUTO));
		if(state)
			lcd.print(F(TEXT_ACT));
		else
			lcd.print(F(TEXT_PAS));
		if(X_state)
			lcd.print(F(TEXT_RUN));
		else
			lcd.print(F(TEXT_STOP));
	}
	else {
		//if(A_outPin)
		if(out)
			lcd.print(F(TEXT_ON2));
		else
			lcd.print(F(TEXT_OFF2));
		if(!mode && state && X_state)
			lcd.print(F(TEXT_RUN2));
		else
			lcd.print(F(TEXT_STOP2));

		/*
		if(mode)
			lcd.print(F(TEXT_MAN2));
		else
			lcd.print(F(TEXT_AUTO2));
		if(state)
			lcd.print(F(TEXT_ACT2));
		else
			lcd.print(F(TEXT_PAS2));
		if(X_state)
			lcd.print(F(TEXT_RUN2));
		else
			lcd.print(F(TEXT_STOP2));
		*/
	}
	/*
	if(state && X_state) {
		if(secCnt & 1)
			lcd.print('-');
		else
			lcd.print('|');

		//uint8_t cnt = secCnt & 3;
		//if(cnt == 0) lcd.print('-');
		//if(cnt == 1) lcd.print('/');
		//if(cnt == 2) lcd.print('|');
		//if(cnt == 3) lcd.print("\\");

	}
	else
		if(secCnt & 1)
			lcd.print('-');
		else
			lcd.print(' ');
	*/
	if(detail) {
		lcd.setCursor(0, 1);
		lcd.print(cycles);

		if(secCnt & 1)
			lcd.print(':');
		else
			lcd.print(' ');

		lcd.print(cyclesLimit);
		uiLcdPrintSpaces8();
		uiLcdPrintSpaces8();
	}
}

//TODO
uint8_t uiState;
enum {UISTATE_MAIN, UISTATE_EDITTEXT };

char uiChar;
bool uiMenuBlocked;
//

void uiMain() {

	Menu.enable(false);

	if(co2LowAlarm.unAck) {
		(secCnt & 1) ? lcd.backlight() : lcd.noBacklight();
	}
	else {
		lcd.backlight();
	}

	if(uiState == UISTATE_MAIN) {

		if(uiChar =='A') //KPD_UP)
			uiPage--;
		if(uiChar == 'B') //KPD_DOWN)
			uiPage++;
		if(uiChar == '#') //KPD_DOWN)
			uiPage = 0;

		uiPage = max(0, uiPage);
		uiPage = min(13, uiPage);

		if(uiPage==0) {
			/*
			//lcd.setCursor(0, 0);
			//lcd.print("-");
			//uiLcdPrintSpaces8();
			//uiLcdPrintSpaces8();
			lcd.setCursor(0, 0);
			uiInstrument("A", A_outPin, A_mode, A_state, A_halfCycles>>1, A_cyclesLimit, false);
			lcd.print("  ");

			//if(secCnt & 1)
			//	lcd.print(':');
			//else
			//	lcd.print(' ');

			uiInstrument("B", B_outPin, B_mode, B_state, B_halfCycles>>1, B_cyclesLimit, false);

			lcd.setCursor(0, 1);
			uiInstrument("C", C_outPin, C_mode, C_state, C_halfCycles>>1, C_cyclesLimit, false);
			lcd.print("  ");

			//if(secCnt & 1)
			//	lcd.print(' ');
			//else
			//	lcd.print(':');

			uiInstrument("D", D_outPin, D_mode, D_state, D_halfCycles>>1, D_cyclesLimit, false);
			*/

			lcd.setCursor(0, 0);
			if(X_state)
				lcd.print(F(TEXT_RUN));
			else
				lcd.print(F(TEXT_STOP));
			lcd.print("   ");


			//lcd.setCursor(7, 0);
			uiInstrument("A", A_outPin, A_mode, A_state, A_halfCycles>>1, A_cyclesLimit, false);
			lcd.print(' ');
			uiInstrument("B", B_outPin, B_mode, B_state, B_halfCycles>>1, B_cyclesLimit, false);
			//lcd.print(' ');
			lcd.setCursor(0, 1);
			lcd.print(F("       "));
			//lcd.setCursor(7, 1);
			uiInstrument("C", C_outPin, C_mode, C_state, C_halfCycles>>1, C_cyclesLimit, false);
			lcd.print(' ');
			uiInstrument("D", D_outPin, D_mode, D_state, D_halfCycles>>1, D_cyclesLimit, false);
			//lcd.print(' ');
		}

		else if(uiPage==1)
			uiInstrument("A:", A_outPin, A_mode, A_state, A_halfCycles>>1, A_cyclesLimit);
		else if(uiPage==2)
			uiInstrument("B:", B_outPin, B_mode, B_state, B_halfCycles>>1, B_cyclesLimit);
		else if(uiPage==3)
			uiInstrument("C:", C_outPin, C_mode, C_state, C_halfCycles>>1, C_cyclesLimit);
		else if(uiPage==4)
			uiInstrument("D:", D_outPin, D_mode, D_state, D_halfCycles>>1, D_cyclesLimit);

		else if(uiPage==5)
			uiInstrumentSettingsOff("A:", A_init, A_offSec, A_offMin, A_offHour, A_onSec, A_onMin, A_onHour, A_out, A_sec);
		else if(uiPage==6)
			uiInstrumentSettingsOn("A:", A_init, A_offSec, A_offMin, A_offHour, A_onSec, A_onMin, A_onHour, A_out, A_sec);

		else if(uiPage==7)
			uiInstrumentSettingsOff("B:", B_init, B_offSec, B_offMin, B_offHour, B_onSec, B_onMin, B_onHour, B_out, B_sec);
		else if(uiPage==8)
			uiInstrumentSettingsOn("B:", B_init, B_offSec, B_offMin, B_offHour, B_onSec, B_onMin, B_onHour, B_out, B_sec);

		else if(uiPage==9)
			uiInstrumentSettingsOff("C:", C_init, C_offSec, C_offMin, C_offHour, C_onSec, C_onMin, C_onHour, C_out, C_sec);
		else if(uiPage==10)
			uiInstrumentSettingsOn("C:", C_init, C_offSec, C_offMin, C_offHour, C_onSec, C_onMin, C_onHour, C_out, C_sec);

		else if(uiPage==11)
			uiInstrumentSettingsOff("D:", D_init, D_offSec, D_offMin, D_offHour, D_onSec, D_onMin, D_onHour, D_out, D_sec);
		else if(uiPage==12)
			uiInstrumentSettingsOn("D:", D_init, D_offSec, D_offMin, D_offHour, D_onSec, D_onMin, D_onHour, D_out, D_sec);

		else if(uiPage==13) {
			lcd.setCursor(0, 0);
			lcd.print(F("EMAIL  "));
			lcd.print(F("BCSEDLON@"));
			lcd.setCursor(0, 1);
			//uiLcdPrintSpaces8();
			lcd.setCursor(0, 1);
			lcd.print(F("       GMAIL.COM"));
		}
		/*
		else if(uiPage==6) {
			lcd.setCursor(0, 0);
			lcd.print(F("CO2: "));
			lcd.print(co2);
			uiLcdPrintSpaces8();
			lcd.setCursor(0, 1);
			if(co2LowAlarm.active) {
				lcd.print(F("NIZKA KONCENT"));
				if(secCnt & 1)
					lcd.print(F("!!!"));
				else
					lcd.print(F("   "));
			}
			else {
				uiLcdPrintSpaces8();
				uiLcdPrintSpaces8();
			}
		}*/
	}


	//lcd.backlight();
	/*
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print('|');
	lcd.print('-');
	*/

	/*
	if(uiState == UISTATE_EDITTEXT) {

		lcd.setCursor(0, 0);
		//"0123456789ABCDEF"
		lcd.print(F("TEXT:"));

		if(uiChar == 'C')
			uiPage++;
		if(uiChar == 'D')
			uiPage--;

		uiPage = max(0, uiPage);
		uiPage = min(15, uiPage);

		//text[0] = 64;
		uint8_t i;

		//strncpy(text2, text, 16);
		i = text[uiPage];
		if(uiChar == 'A') i++;
		if(uiChar == 'B') i--;
		i = max(32, i);
		i = min(126, i);
		text[uiPage] = (char)i;

		lcd.setCursor(0, 1);
		lcd.print(text);

		uiLcdPrintSpaces8();
		uiLcdPrintSpaces8();

		if(secCnt & 1) {
			lcd.setCursor(uiPage, 1);
			lcd.print('_');
		}

		//lcd.setCursor(uiPage, 1);
		//lcd.print(char(i));

		//Serial.println(uiChar);
		//Serial.println();
		//Serial.println(uiPage);
		//Serial.println(i);
		//Serial.println();

		if(uiChar == '*') {
			strncpy(text1, text, 16);
			//uiState = UISTATE_MAIN;
			//uiMenuBlocked = false;
			//Menu.enable(true);
			//uiState=0;
		}

		if(uiChar == '#') {
			uiMenuBlocked = false;
			//Menu.enable(true);
			uiState = UISTATE_MAIN;
		}



	}*/
	uiChar = 0;
}

/*
void listener(char ch) {
	Serial.println(ch);
}
*/


void loadEEPROM() {
    using namespace OMEEPROM;

    // A
    read(ADDR_A_MODE, A_mode);
    read(ADDR_A_STATE, A_state);
    read(ADDR_A_INIT, A_init);
    read(ADDR_A_CYCLESLIMIT, A_cyclesLimit);
    read(ADDR_A_ONHOUR, A_onHour);
    read(ADDR_A_ONMIN, A_onMin);
    read(ADDR_A_ONSEC, A_onSec);
    read(ADDR_A_OFFHOUR, A_offHour);
    read(ADDR_A_OFFMIN, A_offMin);
    read(ADDR_A_OFFSEC, A_offSec);
    read(ADDR_A_OFFSETSEC, A_offsetSec);

    // B
    read(ADDR_B_MODE, B_mode);
    read(ADDR_B_STATE, B_state);
    read(ADDR_B_INIT, B_init);
    read(ADDR_B_CYCLESLIMIT, B_cyclesLimit);
    read(ADDR_B_ONHOUR, B_onHour);
    read(ADDR_B_ONMIN, B_onMin);
    read(ADDR_B_ONSEC, B_onSec);
    read(ADDR_B_OFFHOUR, B_offHour);
    read(ADDR_B_OFFMIN, B_offMin);
    read(ADDR_B_OFFSEC, B_offSec);
    read(ADDR_B_OFFSETSEC, B_offsetSec);

    // C
    read(ADDR_C_MODE, C_mode);
    read(ADDR_C_STATE, C_state);
    read(ADDR_C_INIT, C_init);
    read(ADDR_C_CYCLESLIMIT, C_cyclesLimit);
    read(ADDR_C_ONHOUR, C_onHour);
    read(ADDR_C_ONMIN, C_onMin);
    read(ADDR_C_ONSEC, C_onSec);
    read(ADDR_C_OFFHOUR, C_offHour);
    read(ADDR_C_OFFMIN, C_offMin);
    read(ADDR_C_OFFSEC, C_offSec);
    read(ADDR_C_OFFSETSEC, C_offsetSec);

    // D
    read(ADDR_D_MODE, D_mode);
    read(ADDR_D_STATE, D_state);
    read(ADDR_D_INIT, D_init);
    read(ADDR_D_CYCLESLIMIT, D_cyclesLimit);
    read(ADDR_D_ONHOUR, D_onHour);
    read(ADDR_D_ONMIN, D_onMin);
    read(ADDR_D_ONSEC, D_onSec);
    read(ADDR_D_OFFHOUR, D_offHour);
    read(ADDR_D_OFFMIN, D_offMin);
    read(ADDR_D_OFFSEC, D_offSec);
    read(ADDR_D_OFFSETSEC, D_offsetSec);

}

void saveSetting3() {

	// A
	A_mode = 0;
	A_state = 1;
	A_init = false;
	A_cyclesLimit = 1500;
	A_onSec = 6;
	A_offSec = 18;
	A_offsetSec = 0;
	A_onHour = A_onMin =  A_offHour = A_offMin = 0;

	// B
	B_mode = 0;
	B_state = 1;
	B_init = true;
	B_cyclesLimit = 1500;
	B_onSec = 6;
	B_offSec = 18;
	B_offsetSec = 6;
	B_onHour = B_onMin = B_offHour = B_offMin = 0;

	// C
	C_mode = 0;
	C_state = 1;
	C_init = false;
	C_cyclesLimit = 1500;
	C_onSec = C_offSec = 12;
	C_onHour = C_onMin = C_offHour = C_offMin = 0;
	C_offsetSec= 6;

	// D
	D_mode = 0;
	D_state = 1;
	D_init = true;
	D_cyclesLimit = 1500;
	D_onSec = D_offSec = 12;
	D_onHour = D_onMin = D_offHour = D_offMin = 0;
	D_offsetSec = 6;

    saveDefaultEEPROM();
}

void saveSetting1() {
	// A
	A_mode = 0;
	A_state = 1;
	A_init = false;
	A_cyclesLimit = 1500;
	A_onSec = 60;
	A_offSec = 180;
	A_offsetSec = 0;
	A_onHour = A_onMin =  A_offHour = A_offMin = 0;

	// B
	B_mode = 0;
	B_state = 1;
	B_init = true;
	B_cyclesLimit = 1500;
	B_onSec = 60;
	B_offSec = 180;
	B_offsetSec = 60;
	B_onHour = B_onMin = B_offHour = B_offMin = 0;

	// C
	C_mode = 0;
	C_state = 1;
	C_init = false;
	C_cyclesLimit = 1500;
	C_onSec = C_offSec = 120;
	C_onHour = C_onMin = C_offHour = C_offMin = 0;
	C_offsetSec= 60;

	// D
	D_mode = 0;
	D_state = 1;
	D_init = true;
	D_cyclesLimit = 1500;
	D_onSec = D_offSec = 120;
	D_onHour = D_onMin = D_offHour = D_offMin = 0;
	D_offsetSec = 60;

	saveDefaultEEPROM();
}

void saveSetting2() {
	// A
	A_mode = 0;
	A_state = 1;
	A_init = false;
	A_cyclesLimit = 5;

	A_offsetSec = 0;

	A_onHour = 0;
	A_onMin = 15;
	A_onSec = 0;

	A_offHour = 0;
	A_offMin = 10;
	A_offSec = 0;

	// B
	B_mode = 0;
	B_state = 1;
	B_init = true;
	B_cyclesLimit = 5;

	B_offsetSec = 0;

	B_onHour = 0;
	B_onMin = 10;
	B_onSec = 0;
	B_offHour = 0;
	B_offMin = 15;
	B_offSec = 0;

	// C
	C_mode = 0;
	C_state = 1;
	C_init = false;
	C_cyclesLimit = 5;

	C_offsetSec = 0;

	C_onHour = 0;
	C_onMin = 15;
	C_onSec = 0;

	C_offHour = 0;
	C_offMin = 10;
	C_offSec = 0;

	// D
	D_mode = 0;
	D_state = 1;
	D_init = true;
	D_cyclesLimit = 5;

	D_offsetSec = 0;

	D_onHour = 0;
	D_onMin = 10;
	D_onSec = 0;
	D_offHour = 0;
	D_offMin = 15;
	D_offSec = 0;

}

void saveDefaultEEPROM() {
    using namespace OMEEPROM;

    // A
    write(ADDR_A_MODE, A_mode);
    write(ADDR_A_STATE, A_state);
    write(ADDR_A_INIT, A_init);
    write(ADDR_A_CYCLESLIMIT, A_cyclesLimit);
    write(ADDR_A_ONHOUR, A_onHour);
    write(ADDR_A_ONMIN, A_onMin);
    write(ADDR_A_ONSEC, A_onSec);
    write(ADDR_A_OFFHOUR, A_offHour);
    write(ADDR_A_OFFMIN, A_offMin);
    write(ADDR_A_OFFSEC, A_offSec);
    write(ADDR_A_OFFSETSEC, A_offsetSec);

    // B
    write(ADDR_B_MODE, B_mode);
    write(ADDR_B_STATE, B_state);
    write(ADDR_B_INIT, B_init);
    write(ADDR_B_CYCLESLIMIT, B_cyclesLimit);
    write(ADDR_B_ONHOUR, B_onHour);
    write(ADDR_B_ONMIN, B_onMin);
    write(ADDR_B_ONSEC, B_onSec);
    write(ADDR_B_OFFHOUR, B_offHour);
    write(ADDR_B_OFFMIN, B_offMin);
    write(ADDR_B_OFFSEC, B_offSec);
    write(ADDR_B_OFFSETSEC, B_offsetSec);

    // C
    write(ADDR_C_MODE, C_mode);
    write(ADDR_C_STATE, C_state);
    write(ADDR_C_INIT, C_init);
    write(ADDR_C_CYCLESLIMIT, C_cyclesLimit);
    write(ADDR_C_ONHOUR, C_onHour);
    write(ADDR_C_ONMIN, C_onMin);
    write(ADDR_C_ONSEC, C_onSec);
    write(ADDR_C_OFFHOUR, C_offHour);
    write(ADDR_C_OFFMIN, C_offMin);
    write(ADDR_C_OFFSEC, C_offSec);
    write(ADDR_C_OFFSETSEC, C_offsetSec);

    // D
    write(ADDR_D_MODE, D_mode);
    write(ADDR_D_STATE, D_state);
    write(ADDR_D_INIT, D_init);
    write(ADDR_D_CYCLESLIMIT, D_cyclesLimit);
    write(ADDR_D_ONHOUR, D_onHour);
    write(ADDR_D_ONMIN, D_onMin);
    write(ADDR_D_ONSEC, D_onSec);
    write(ADDR_D_OFFHOUR, D_offHour);
    write(ADDR_D_OFFMIN, D_offMin);
    write(ADDR_D_OFFSEC, D_offSec);
    write(ADDR_D_OFFSETSEC, D_offsetSec);

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
		//saveDefaultEEPROM();
		saveSetting1();

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

	digitalWrite(PIN_A, true);
	digitalWrite(PIN_B, true);
	digitalWrite(PIN_C, true);
	digitalWrite(PIN_D, true);
	pinMode(PIN_A, OUTPUT);
	pinMode(PIN_B, OUTPUT);
	pinMode(PIN_C, OUTPUT);
	pinMode(PIN_D, OUTPUT);

	digitalWrite(PIN_BUZZER, HIGH);
	pinMode(PIN_BUZZER, OUTPUT);


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

/*
void uiSetWifiPass() {
	Menu.enable(false);
	uiMenuBlocked = true;
	uiState = UISTATE_EDITTEXT;
	uiPage=0;
	lcd.clear();

	strncpy(text, text1, 16);
}
*/
void uiSetParam1() {
	//saveDefaultEEPROM();
	saveSetting1();
	loadEEPROM();
	X_restart();
	X_stop();
	uiMessage(TEXT_RESETTED);
}

void uiSetParam2() {
	//saveDefaultEEPROM();
	saveSetting2();
	loadEEPROM();
	X_restart();
	X_stop();
	uiMessage(TEXT_RESETTED);
}

void uiSetParam3() {
	//saveDefaultEEPROM();
	saveSetting3();
	loadEEPROM();
	X_restart();
	X_stop();
	uiMessage(TEXT_RESETTED);
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

	//digitalWrite(PIN_BUZZER, LOW);

	uiMessageRestarted();

	//digitalWrite(PIN_BUZZER, HIGH);
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


void B_uiStart() {
	B_start();
	uiMessageStarted();
}
void B_uiStop() {
	B_stop();
	uiMessageStopped();
}
void B_uiRestart() {
	B_restart();
	uiMessageRestarted();
}

void C_uiStart() {
	C_start();
	uiMessageStarted();
}
void C_uiStop() {
	C_stop();
	uiMessageStopped();
}
void C_uiRestart() {
	C_restart();
	uiMessageRestarted();
}

void D_uiStart() {
	D_start();
	uiMessageStarted();
}
void D_uiStop() {
	D_stop();
	uiMessageStopped();
}
void D_uiRestart() {
	D_restart();
	uiMessageRestarted();
}

void X_start() {
	X_state = 1;
	//A_start();
	if(!A_halfCycles)
		A_out = A_init;
	if(!B_halfCycles)
		B_out = B_init;
	if(!C_halfCycles)
		C_out = C_init;
	if(!D_halfCycles)
		D_out = D_init;
}
void X_stop() {
	X_state = 0;
	//A_stop();
}
void X_restart() {
	secCnt = 0;
	secOverflow = false;

	X_state = 0;
	A_restart();
	B_restart();
	C_restart();
	D_restart();
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
	//A_state = 1;
	A_out = A_init;
	A_sec = 0;
	A_halfCycles = 0;
	A_outPin = getInstrumentControl(A_out, A_mode);
}

void B_start() {
	B_state = 1;
	if(!B_halfCycles)
		B_out = B_init;
}
void B_stop() {
	B_state = 0;
}
void B_restart() {
	//B_state = 1;
	B_out = B_init;
	B_sec = 0;
	B_halfCycles = 0;
	B_outPin = getInstrumentControl(B_out, B_mode);
}

void C_start() {
	C_state = 1;
	if(!C_halfCycles)
		C_out = C_init;
}
void C_stop() {
	C_state = 0;
}
void C_restart() {
	//C_state = 1;
	C_out = C_init;
	C_sec = 0;
	C_halfCycles = 0;
	C_outPin = getInstrumentControl(C_out, C_mode);
}

void D_start() {
	D_state = 1;
	if(!D_halfCycles)
		D_out = D_init;
}
void D_stop() {
	D_state = 0;
}
void D_restart() {
	//D_state = 1;
	D_out = D_init;
	D_sec = 0;
	D_halfCycles = 0;
	D_outPin = getInstrumentControl(D_out, D_mode);
}

bool getInstrumentControl(bool a, uint8_t mode) {
	if(mode == 0) return a;
	if(mode == 1) return false;
	if(mode == 2) return true;
	return false;
}

// The loop function is called in an endless loop



double analogRead(int pin, int samples){
  int result = 0;
  for(int i=0; i<samples; i++){
    result += analogRead(pin);
  }
  return (double)(result / samples);
}


void loop()
{
	//Add your repeated code here
	//char key = kpd.getKey();
	//char ch = kpd.getRawKey();
	char ch = kpd.getKey2();
	if(ch) {
		co2LowAlarm.ack();
	}

	if(ch == '*') {//KPD_ENTER)
		//if(!uiState)
		if(!uiMenuBlocked)
			Menu.enable(true);
		//uiState = 0;
		//uiPage = 0;
	}
	/*
	if(ch == '#')
		if(uiState) {
			Menu.enable(true);
			uiState = 0;
		}
	*/

	Menu.checkInput();

	/*
	co2 = analogRead(PIN_CO2, 100);
	double co2LowLimit = 190;
	double co2LimitHyst = 2;
	co2LowAlarm.activate(co2 < co2LowLimit);
	co2LowAlarm.deactivate(co2 > co2LowLimit + co2LimitHyst);

	//co2LowAlarm.activate(true);
	//co2LowAlarm.deactivate(co2 > co2LowLimit + co2LimitHyst);
	*/

	if (secInterval.expired()) {
		secInterval.set(1000);




		if(X_state) {
			//running

			// A
			if(((A_halfCycles >> 1) >= A_cyclesLimit) && A_cyclesLimit) {
				A_state = 0;
				A_out = false;
			}
			if(A_state && ((A_offsetSec <= secCnt ) || secOverflow)) { //??? secOverflow
			//if(A_state) { // && !A_mode
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

			// B
			if(((B_halfCycles >> 1) >= B_cyclesLimit) && B_cyclesLimit) {
				B_state = 0;
				B_out = false;
			}
			if(B_state && ((B_offsetSec <= secCnt ) || secOverflow)) {
			//if(B_state) { // && !B_mode
				if(B_out)
					B_set = B_onSec + B_onMin*60 + B_onHour*60*60;
				else
					B_set = B_offSec + B_offMin*60 + B_offHour*60*60;
				if(B_sec >= B_set) {
					B_out = !B_out;
					B_sec = 0;
					B_halfCycles++;
				}
				B_sec++;
			}

			// C
			if(((C_halfCycles >> 1) >= C_cyclesLimit) && C_cyclesLimit) {
				C_state = 0;
				C_out = false;
			}
			if(C_state && ((C_offsetSec <= secCnt ) || secOverflow)) {
			//if(C_state) { // && !C_mode
				if(C_out)
					C_set = C_onSec + C_onMin*60 + C_onHour*60*60;
				else
					C_set = C_offSec + C_offMin*60 + C_offHour*60*60;
				if(C_sec >= C_set) {
					C_out = !C_out;
					C_sec = 0;
					C_halfCycles++;
				}
				C_sec++;
			}

			// D
			if(((D_halfCycles >> 1) >= D_cyclesLimit) && D_cyclesLimit) {
				D_state = 0;
				D_out = false;
			}
			if(D_state && ((D_offsetSec <= secCnt ) || secOverflow)) {
			//if(D_state) { // && !D_mode
				if(D_out)
					D_set = D_onSec + D_onMin*60 + D_onHour*60*60;
				else
					D_set = D_offSec + D_offMin*60 + D_offHour*60*60;
				if(D_sec >= D_set) {
					D_out = !D_out;
					D_sec = 0;
					D_halfCycles++;
				}
				D_sec++;
			}



			secCnt++;
			if(!secCnt)
				secOverflow = true;
		}

		digitalWrite(PIN_LED, A_outPin);

		A_outPin = getInstrumentControl(A_out, A_mode);
		B_outPin = getInstrumentControl(B_out, B_mode);
		C_outPin = getInstrumentControl(C_out, C_mode);
		D_outPin = getInstrumentControl(D_out, D_mode);

		digitalWrite(PIN_A, !A_outPin);
		digitalWrite(PIN_B, !B_outPin);
		digitalWrite(PIN_C, !C_outPin);
		digitalWrite(PIN_D, !D_outPin);
	}

	if(!Menu.shown() || !Menu.enable()) {
	//if(!Menu.enable()) {
		uiMain();

		ch = kpd.getKey();
		//ch = kpd.getKey2();
		/*
		if(ch =='A') //KPD_UP)
			uiPage--;
		if(ch == 'B') //KPD_DOWN)
			uiPage++;
		if(ch == '#') {//KPD_DOWN)
			uiPage = 0;
			//uiMenuBlocked = false;
		}
		*/
		//uiPage = max(0, uiPage);
		//uiPage = min(5, uiPage);

		if(uiChar==0)
			uiChar = ch;

	}
}
