/***
Servo Module Header File
***/
#ifndef Servo_H
#define Servo_H


bool Servo_HWInit(void);

bool Servo_MoveCannon(uint8_t ServoCount);
bool Servo_MoveTimer(uint8_t ServoCount);
bool Servo_MoveO2Gauge(uint8_t ServoCount);
bool Servo_MovePowerGauge(uint8_t ServoCount);
bool Servo_MoveTOT(uint8_t ServoCount);

uint8_t QueryServoPosition(uint8_t Which);

#endif /*Servo_H*/
