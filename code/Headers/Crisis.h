/***
Crisis Header File
***/
#ifndef Crisis_H
#define Crisis_H

#ifndef UI_STATE_T
#define UI_STATE_T
typedef enum{
	On = 1,
	Off = 0
}UI_State_t;
#endif

void Crisis_Meteor(UI_State_t state);
void Crisis_AirLeak(UI_State_t state);
void Crisis_Power(UI_State_t state);
void Crisis_Buzzer(UI_State_t state);

#endif /*Crisis_H*/
