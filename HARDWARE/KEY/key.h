#ifndef _key_h_
#define _key_h_
#include "sys.h"

void KEY_Init(void);
void keyscan(void);
void keydriver(void);
void keyaction(unsigned char keycode);
void EEROM_SpeedPIDRead(void);
#endif

