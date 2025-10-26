#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

void UART1_Config(void);
void UART1_SendChar(char c);
void UART1_SendString(char *s);
void SystemClock_HSE_72MHz(void);

void Task1(void *pvParameters);
void Task2(void *pvParameters);

SemaphoreHandle_t xUARTMutex;

int main(void)
{
    SystemClock_HSE_72MHz();
    UART1_Config();

    xUARTMutex = xSemaphoreCreateMutex();

    xTaskCreate(Task1, "T1", 128, NULL, 1, NULL);
    xTaskCreate(Task2, "T2", 128, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1);
}

void UART1_Config(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef uart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    uart.USART_BaudRate = 115200;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_Mode = USART_Mode_Tx;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART1, &uart);
    USART_Cmd(USART1, ENABLE);
}

void UART1_SendChar(char c)
{
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = c;
}

void UART1_SendString(char *s)
{
    while (*s)
    {
        UART1_SendChar(*s++);
    }
}

void Task1(void *pvParameters)
{
    int i;
    while (1)
    {
        if (xSemaphoreTake(xUARTMutex, portMAX_DELAY) == pdTRUE)
        {
            for (i = 0; i < 10; i++)
                UART1_SendChar('A');
            UART1_SendString("\r\n");
            xSemaphoreGive(xUARTMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void Task2(void *pvParameters)
{
    int i;
    while (1)
    {
        if (xSemaphoreTake(xUARTMutex, portMAX_DELAY) == pdTRUE)
        {
            for (i = 0; i < 10; i++)
                UART1_SendChar('B');
            UART1_SendString("\r\n");
            xSemaphoreGive(xUARTMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void SystemClock_HSE_72MHz(void)
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    if (RCC_WaitForHSEStartUp() == SUCCESS)
    {
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        RCC_PCLK2Config(RCC_HCLK_Div1);
        RCC_PCLK1Config(RCC_HCLK_Div2);
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
        RCC_PLLCmd(ENABLE);
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        while (RCC_GetSYSCLKSource() != 0x08);
    }
}
