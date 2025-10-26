/********** IRQ demo: PA0 -> EXTI0, toggle LED PC13 **********/
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"

#define LED_PORT   GPIOC
#define LED_PIN    GPIO_Pin_13
#define BTN_PORT   GPIOA
#define BTN_PIN    GPIO_Pin_0

static void GPIO_Config(void) {
    GPIO_InitTypeDef g;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOC |
                           RCC_APB2Periph_AFIO, ENABLE);

    // PC13: LED on-board (active-low)
    g.GPIO_Pin   = LED_PIN;
    g.GPIO_Mode  = GPIO_Mode_Out_PP;
    g.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(LED_PORT, &g);
    GPIO_SetBits(LED_PORT, LED_PIN);           // t?t (active-low)

    // PA0: nút nh?n (kéo lên n?i), nh?n -> 0V
    g.GPIO_Pin   = BTN_PIN;
    g.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_Init(BTN_PORT, &g);
}

static void EXTI0_Config(void) {
    EXTI_InitTypeDef e;
    NVIC_InitTypeDef n;

    // Ánh x? PA0 -> EXTI Line0
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    // C?u h?nh EXTI0: ng?t c?nh xu?ng (nh?n -> 0)
    e.EXTI_Line    = EXTI_Line0;
    e.EXTI_Mode    = EXTI_Mode_Interrupt;
    e.EXTI_Trigger = EXTI_Trigger_Falling;
    e.EXTI_LineCmd = ENABLE;
    EXTI_Init(&e);

    // B?t kênh NVIC c?a IRQ EXTI0
    n.NVIC_IRQChannel = EXTI0_IRQn;                 // <-- Ðây là "IRQ"
    n.NVIC_IRQChannelPreemptionPriority = 2;        // ýu tiên ng?t
    n.NVIC_IRQChannelSubPriority        = 0;
    n.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&n);
}

// ISR = Interrupt Service Routine cho IRQ EXTI0
void EXTI0_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line0) == SET) {
        EXTI_ClearITPendingBit(EXTI_Line0);         // xóa c? trý?c

        // Toggle LED PC13 (active-low)
        if (GPIO_ReadOutputDataBit(LED_PORT, LED_PIN))
            GPIO_ResetBits(LED_PORT, LED_PIN);      // b?t
        else
            GPIO_SetBits(LED_PORT, LED_PIN);        // t?t
    }
}

int main(void) {
    SystemInit();
    GPIO_Config();
    EXTI0_Config();

    while (1) {
        __WFI();  // ch? ng?t ð? ti?t ki?m nãng lý?ng (không b?t bu?c)
    }
}

