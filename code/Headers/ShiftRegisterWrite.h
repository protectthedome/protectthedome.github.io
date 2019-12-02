// Header for ShiftRegisterWrite module
// Rev 1

#ifndef ShiftRegisterWrite_H
#define ShiftRegisterWrite_H
void SR_Init(void);
uint8_t SR_GetCurrentRegister(void);
void SR_Write(uint8_t NewValue);
#endif
