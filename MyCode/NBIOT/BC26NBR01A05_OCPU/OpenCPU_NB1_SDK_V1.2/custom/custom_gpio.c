/*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   GPIO.c
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   This app demonstrates is test GPIO 
 *
 *
 *
 ****************************************************************************/

#include "ql_trace.h"
#include "ql_system.h"
#include "ql_gpio.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_uart.h"


#include "custom_Test.h"


// Define the UART port 
/*
static Enum_SerialPort m_myUartPort  = UART_PORT0;

static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{
     
}
*/

void Custom_GPIO_Program(void)
{

    // Specify a GPIO pin
    Enum_PinName  gpioPin = PINNAME_NETLIGHT;

    // Define the initial level for GPIO pin
    Enum_PinLevel gpioLvl = PINLEVEL_HIGH;

    // Initialize the GPIO pin (output high level, pull up)
    Ql_GPIO_Init(gpioPin, PINDIRECTION_OUT, gpioLvl, PINPULLSEL_PULLUP);
    APP_DEBUG("<-- Initialize GPIO pin (PINNAME_STATUS): output, high level, pull up -->\r\n");

    // Get the direction of GPIO
    APP_DEBUG("<-- Get the GPIO direction: %d -->\r\n", Ql_GPIO_GetDirection(gpioPin));

    // Get the level value of GPIO
    APP_DEBUG("<-- Get the GPIO level value: %d -->\r\n\r\n", Ql_GPIO_GetLevel(gpioPin));

    // Set the GPIO level to low after 500ms.
    APP_DEBUG("<-- Set the GPIO level to low after 500ms -->\r\n");
    Ql_Sleep(500);
    Ql_GPIO_SetLevel(gpioPin, PINLEVEL_LOW);
    APP_DEBUG("<-- Get the GPIO level value: %d -->\r\n\r\n", Ql_GPIO_GetLevel(gpioPin));

    // Set the GPIO level to high after 500ms.
    APP_DEBUG("<-- Set the GPIO level to high after 500ms -->\r\n");
    Ql_Sleep(500);
    Ql_GPIO_SetLevel(gpioPin, PINLEVEL_HIGH);
    APP_DEBUG("<-- Get the GPIO level value: %d -->\r\n", Ql_GPIO_GetLevel(gpioPin));
}

