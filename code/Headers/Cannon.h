/***
Cannon Library Header File
***/
#ifndef Cannon_H
#define Cannon_H

#define COUNT_UPPER_LIM 3968
#define COUNT_LOWER_LIM 128

//typedef gaurd
#ifndef UI_STATE_T
#define UI_STATE_T
typedef enum{
	On = 1,
	Off = 0
}UI_State_t;
#endif

void Cannon_HWInit(void);

void Cannon_LED(UI_State_t state);
void Cannon_Vibrate(UI_State_t state);

uint16_t Cannon_GetPotValue(void);
uint8_t Cannon_ReadButton(void);

bool Cannon_UserPosition(uint16_t ADCount);
bool Cannon_MoveToBank(uint8_t Bank);
bool Cannon_ManualPosition(uint8_t position);

uint8_t QueryCannonPosition(void);
uint8_t QueryBankPositions(uint8_t Bank);

#endif /*Cannon_H*/
