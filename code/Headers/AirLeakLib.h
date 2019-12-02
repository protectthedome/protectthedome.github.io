/***
AirLeak Library Header File
***/
#ifndef AirLeakLib_H
#define AirLeakLib_H

//typedef gaurd
#ifndef UI_STATE_T
#define UI_STATE_T
typedef enum{
	On = 1,
	Off = 0
}UI_State_t;
#endif

void AirLeak_Init(void);
bool AirLeak_FanControl(uint8_t Which, UI_State_t state);
bool AirLeak_MoveGauge(uint8_t position);

void AirLeak_ReadHallSensors(uint8_t* data);

uint8_t AirLeak_QueryFanState(uint8_t Which);
uint8_t AirLeak_QueryGauge(void);

#endif /*AirLeakLib_H*/
