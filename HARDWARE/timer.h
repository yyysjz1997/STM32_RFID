#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
#include "delay.h"
#include "output.h"

void TIM2_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM5_Capture_Init(u16 arr,u16 psc);
void servo_try_main(void);
void servo_main(void);
void TIM6_Int_Init(u16 arr,u16 psc);


#endif


