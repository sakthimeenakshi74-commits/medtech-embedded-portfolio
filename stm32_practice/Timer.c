#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdio.h>

volatile bool one_sec_elapsed=false;
void GPIO_Init(){
   RCC->AHB1ENR|=(1<<2);
   GPIOC->MODER&= ~(3<<(13*2));
   GPIOC->MODER|= (1<<(13*2));
   GPIOC->OTYPER&= ~(1<<13);
   GPIOC->BSRR |= (1<<13);
}

void TIM2_Init(){
   RCC->APB1ENR|= (1<<0);
   TIM2->PSC=8399; //84 MHz/8400=10KHz
   TIM2->ARR=19;  //20 ticks/10KHz=2ms
   TIM2->CR1|= (1<<7);
   TIM2->DIER|= (1<<0);
   NVIC_SetPriority(TIM2_IRQn,2);
   NVIC_EnableIRQ(TIM2_IRQn);
   TIM2->CR1|= (1<<0);
}

void TIM2_IRQHandler(void){
   static uint32_t SampleCount;
   if(TIM2->SR & (1<<0)){
      TIM2->SR&= ~(1<<0);
      SampleCount++;
      if(SampleCount==500){
         GPIOC->ODR^= (1<<13);
         one_sec_elapsed=true;
      }
   }
}

int main(void){
   SystemInit();
   GPIO_Init();
   TIM2_Init();
   while(1){
      if(one_sec_elapsed==true){
         printf("ECG samples collected: 500");
         one_sec_elapsed=false;
      }
   }
}
