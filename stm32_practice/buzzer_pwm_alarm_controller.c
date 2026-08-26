#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdio.h>

/* --- Hardware Pin Mapping Configurations --- */
#define BUZZER_PIN 0
#define BUZZER_PORT GPIOA
#define ALARM_BUTTON_PIN 0
#define ALARM_BUTTON_PORT GPIOB

/* --- Pre-Calculated Compare Match Values for Duty Cycles --- */
/* Formulated using: CCRx = (ARR + 1) * Target_Percentage */
#define CCR_SILENT 0        /*   0% Duty Cycle (0 / 500)   -> Speaker completely quiet */
#define CCR_WARNING 100     /*  20% Duty Cycle (100 / 500) -> Minimal alert volume */
#define CCR_CRITICAL 300    /*  60% Duty Cycle (300 / 500) -> Loud priority beep */
#define CCR_EMERGENCY 495   /*  99% Duty Cycle (495 / 500) -> Maximum hazard notification */

/* --- ISO 13485 / IEC 62304 Compliant Alarm State Engine Type Definition --- */
typedef enum {
   ALARMSILENT = 0,
   ALARMWARNING = 1,
   ALARM_CRITICAL = 2,
   ALARM_EMERGENCY = 3
} AlarmState_t;

/* 
 * CRITICAL VOLATILE MEMORY DECLARATIONS:
 * 'currentState' and 'StateChanged' are updated asynchronously inside the ISR. 
 * 'volatile' prevents compiler optimizations from caching these values in CPU 
 * registers, ensuring the main foreground loop instantly reads RAM state shifts.
 */
volatile AlarmState_t currentState = ALARMSILENT;
volatile bool StateChanged = false;

void PWM_BUZZER_INIT(void) {
   /* Enable GPIOA (Bit 0) and TIM2 (Bit 0) peripheral clock gates */
   RCC->AHB1ENR |= (1 << 0);
   RCC->APB1ENR |= (1 << 0);

   /* Configure PA0: Clear mode bits, then set to Alternate Function Mode (0x2) */
   GPIOA->MODER &= ~(3 << (BUZER_PIN * 2));
   GPIOA->MODER |= (2 << (BUZZER_PIN * 2));
   
   /* Set PA0 to High Speed (0x3) to keep high-frequency PWM edges crisp */
   GPIOA->OSPEEDR |= (3 << (BUZZER_PIN * 2));
   
   /* Route PA0 pin to Alternate Function 1 (AF1) to connect it to TIM2_CH1 */
   GPIOA->AFR[0] &= ~(0XF << (BUZZER_PIN * 4));
   GPIOA->AFR[0] |= (0X1 << (BUZZER_PIN * 4));

   /* 
    * TIM2 TIMEBASE FREQUENCY CALCULATION (16MHz Core Clock Source):
    * Tick Clock Frequency = 16MHz / (PSC + 1) = 16,000,000 / (83 + 1) = 190.47 kHz
    * Output PWM Frequency = Tick Clock / (ARR + 1) = 190,476 / (499 + 1) = 380.95 Hz
    * (Note: Comment says KHz, math targets ~381Hz based on a 16MHz internal base)
    */
   TIM2->PSC = 83;
   TIM2->ARR = 499;
   
   /* Set ARPE bit (Bit 7): Enable Auto-Reload Preload buffering for glitch-free updates */
   TIM2->CR1 |= (1 << 7);

   /* 
    * CCMR1 Configuration: Clear output compare bits, set PWM Mode 1 (0x6 << 4), 
    * and set OC1PE (Bit 3) to enable compare register preload buffering.
    */
   TIM2->CCMR1 &= ~(7 << 4);
   TIM2->CCMR1 |= (6 << 4);
   TIM2->CCMR1 |= (1 << 3);

   /* Start at 0% power, connect Output Channel 1, and enable the main timer counter */
   TIM2->CCR1 = 0;
   TIM2->CCER |= (1 << 0);
   TIM2->CR1 |= (1 << 0);
}

void EXTI_INIT(void) {
   /* Enable GPIOB (Bit 1) and SYSCFG (Bit 14) system configuration clocks */
   RCC->AHB1ENR |= (1 << 1);
   RCC->APB2ENR |= (1 << 14);

   /* Configure PB0 to Input Mode (0x0) with an Internal Pull-Up Resistor (0x1) */
   GPIOB->MODER &= ~(3 << (ALARM_BUTTON_PIN * 2));
   GPIOB->PUPDR &= ~(3 << (ALARM_BUTTON_PIN * 2));
   GPIOB->PUPDR |= (1 << (ALARM_BUTTON_PIN * 2));

   /* Route Port B Pin 0 to the external interrupt line 0 (EXTI0) handler matrix */
   SYSCFG->EXTICR[0] &= ~(0XF << ALARM_BUTTON_PIN * 4);
   SYSCFG->EXTICR[0] |= (0X1 << ALARM_BUTTON_PIN * 4);

   /* Unmask line 0, enable Falling Edge Detection, disable Rising Edge Detection */
   EXTI->IMR |= (1 << ALARM_BUTTON_PIN);
   EXTI->FTSR |= (1 << ALARM_BUTTON_PIN);
   EXTI->RTSR &= ~(1 << ALARM_BUTTON_PIN);
   
   /* Set Priority to 1 (High priority medical alert) and enable the vector line in NVIC */
   NVIC_SetPriority(EXTI0_IRQn, 1);
   NVIC_EnableIRQ(EXTI0_IRQn);
}

void EXTI0_IRQHandler(void) {
   /* Verify if the interrupt source is matching the pending bit for Pin 0 */
   if (EXTI->PR & (1 << ALARM_BUTTON_PIN)) {
      /* Clear the pending bit by writing a 1 to it (Standard ARM architecture rule) */
      EXTI->PR = (1 << ALARM_BUTTON_PIN);
      
      /* Circular state increment modulo 4: SILENT -> WARNING -> CRITICAL -> EMERGENCY -> SILENT */
      currentState = (currentState + 1) % 4;
   }
   
   /* Immediate hardware register override based on the newly updated alarm loop state */
   switch (currentState) {
      case ALARMSILENT:
         TIM2->CCR1 = CCR_SILENT;
         break;
      case ALARMWARNING:
         TIM2->CCR1 = CCR_WARNING; /* Sets active pulse width to 20% */
         break;
      case ALARM_CRITICAL:
         TIM2->CCR1 = CCR_CRITICAL; /* Sets active pulse width to 60% */
         break;
      case ALARM_EMERGENCY:
         TIM2->CCR1 = CCR_EMERGENCY; /* Sets active pulse width to 99% */
         count = 0;                  /* WARNING: 'count' is an un-declared local/global variable */
         break;
   }
   
   /* Signal background main loop to dispatch unblocking UART telemetry printouts */
   StateChanged = true;
}

int main(void) {
   SystemInit();        /* Initialize core flash interface and standard vector offsets */
   PWM_BUZZER_INIT();   /* Initialize the PA0 hardware PWM audio driver */
   EXTI_INIT();         /* Activate the PB0 edge-triggered safety push-button hook */
   
   while (1) {
      /* Polling block checks flag set by async ISR context */
      if (StateChanged == true) {
         StateChanged = false; /* Instantly clear flag to trap next state transition event */
         
         /* Telemetry output matching the current active safety configuration metrics */
         switch (currentState) {
            case ALARMSILENT:
               printf("Status : SILENT\n");
               break;
            case ALARMWARNING:
               printf("Status : WARNING\n");
               break;
            case ALARM_CRITICAL:
               printf("Status : CRITICAL\n");
               break;
            case ALARM_EMERGENCY:
               printf("Status : EMERGENCY\n");
               break;
         }
      }
   }
}
