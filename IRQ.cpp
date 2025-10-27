#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

//================================================
// Ð?nh nghia chân
//================================================
#define LED_BLINK_PORT   GPIOA
#define LED_BLINK_PIN    GPIO_Pin_5
#define LED_EVENT_PORT   GPIOA
#define LED_EVENT_PIN    GPIO_Pin_6
#define BTN_PORT         GPIOA
#define BTN_PIN          GPIO_Pin_0

//================================================
// Khai báo semaphore
//================================================
SemaphoreHandle_t xButtonSemaphore;

//================================================
// C?u hình GPIO
//================================================
static void GPIO_Config(void) {
    GPIO_InitTypeDef g;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    // LED PA5: nháy liên t?c
    g.GPIO_Pin   = LED_BLINK_PIN;
    g.GPIO_Mode  = GPIO_Mode_Out_PP;
    g.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(LED_BLINK_PORT, &g);
    GPIO_ResetBits(LED_BLINK_PORT, LED_BLINK_PIN);

    // LED PA6: sáng khi nh?n nút
    g.GPIO_Pin   = LED_EVENT_PIN;
    g.GPIO_Mode  = GPIO_Mode_Out_PP;
    g.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(LED_EVENT_PORT, &g);
    GPIO_ResetBits(LED_EVENT_PORT, LED_EVENT_PIN);

    // Nút PA0 (pull-up n?i)
    g.GPIO_Pin   = BTN_PIN;
    g.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_Init(BTN_PORT, &g);
}

//================================================
// C?u hình ng?t ngoài EXTI0 (PA0)
//================================================
static void EXTI0_Config(void) {
    EXTI_InitTypeDef e;
    NVIC_InitTypeDef n;

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    e.EXTI_Line    = EXTI_Line0;
    e.EXTI_Mode    = EXTI_Mode_Interrupt;
    e.EXTI_Trigger = EXTI_Trigger_Falling;  // nh?n nút ? c?nh xu?ng
    e.EXTI_LineCmd = ENABLE;
    EXTI_Init(&e);

    n.NVIC_IRQChannel = EXTI0_IRQn;
    n.NVIC_IRQChannelPreemptionPriority = 5; // Uu tiên th?p hon FreeRTOS
    n.NVIC_IRQChannelSubPriority        = 0;
    n.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&n);
}

//================================================
// Trình ph?c v? ng?t ngoài EXTI0 (PA0)
//================================================
void EXTI0_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (EXTI_GetITStatus(EXTI_Line0) == SET) {
        EXTI_ClearITPendingBit(EXTI_Line0);

        // G?i semaphore cho task x? lý
        xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

//================================================
// Task 1: Blink LED PA5
//================================================
void vTaskBlinkLED(void *pvParameters) {
    while (1) {
        GPIO_SetBits(LED_BLINK_PORT, LED_BLINK_PIN);
        vTaskDelay(pdMS_TO_TICKS(500));
GPIO_ResetBits(LED_BLINK_PORT, LED_BLINK_PIN);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

//================================================
// Task 2: B?t LED PA6 khi có ng?t nút nh?n
//================================================
void vTaskButtonEvent(void *pvParameters) {
    while (1) {
        // Ch? semaphore t? ng?t
        if (xSemaphoreTake(xButtonSemaphore, portMAX_DELAY) == pdTRUE) {
            GPIO_SetBits(LED_EVENT_PORT, LED_EVENT_PIN);   // B?t LED
            vTaskDelay(pdMS_TO_TICKS(3000));               // Gi? 3 giây
            GPIO_ResetBits(LED_EVENT_PORT, LED_EVENT_PIN); // T?t LED
        }
    }
}

int main(void) {
    SystemInit();
    GPIO_Config();
    EXTI0_Config();

    xButtonSemaphore = xSemaphoreCreateBinary();

    xTaskCreate(vTaskBlinkLED, "Blink", 128, NULL, 1, NULL);
    xTaskCreate(vTaskButtonEvent, "Button", 128, NULL, 2, NULL);

    vTaskStartScheduler();

    while (1);
}
