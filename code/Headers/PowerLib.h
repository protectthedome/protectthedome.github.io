/***
Power Library Header File
***/
#ifndef PowerLib_H
#define PowerLib_H

void PowerLib_HWInit(void);
uint8_t PowerLib_PinState(void);

bool PowerLib_MoveGauge(uint8_t position);
uint8_t PowerLib_QueryGauge(void);

#endif /*PowerLib_H*/
