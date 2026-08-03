#include "stm32f401.h"
volatile bool patientCalled=false;
void GPIO_EXTI_INIT(void){
   // CLOCK ENABLE
   RCC->AHB1ENR|=(1<<2); //GPIOC
   RCC->AHB1ENR|=(1<<1); //GPIOB
   RCC->APB2ENR|=(1<<14); //SYSCFG
   //PC13 AS PUSH-PULL O/P
   GPIOC->MODER&=~(3<<(2*13));
   GPIOC->MODER|=(1<<(2*13));
   GPIOC->OTYPER&=~(1<<13);
   GPIOC->BSRR =(1<<13);
   //PB5 AS PULL UP I/P
   GPIOB->MODER&=~(3<<(2*5));
   GPIOB->PUPDR&=~(3<<(2*5));
   GPIOB->PUPDR|=(1<<(2*5));
   //ROUTE PB5 TO EXTI5
   SYSCFG->EXTICR[1]&=~(0XF <<4);
   SYSCFG->EXTICR[1]|=(0X1 <<4);
   //MASK AND EDGE TRIGGER
   EXTI->IMR|=(1<<5);
   EXTI->FTSR|=(1<<5);
   EXTI->RTSR&=~(1<<5);
   //NVIC
   NVIC_SetPriority(EXTI9_5_IRQn,1);
   NVIC_EnableIRQ(EXTI9_5_IRQn);
}
//ISR
void EXTI9_5_IRQHandler(void){
   if(EXTI->PR & (1<<5)){
      EXTI->PR=(1<<5);
      GPIOC->ODR^=(1<<13);
      patientCalled=true;
   }
}
int main(void){
   SystemInit();
   GPIO_EXTI_INIT();
   while(1){
   if(patientCalled==true){
      printf("Patient call received");
      patientCalled = false;  
   }
   }
}
