/*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   custom_app.c
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   custom app
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


//This Task For Init AT Commed 
void proc_reserved4(void)
{
    for(;;)
    {
 	  	 s32 ret;
   		 ret = Ql_RIL_SendATCmd("AT+CEREG?\n", Ql_strlen("AT+CEREG?\n"), ATResponse_Handler, NULL, 0);
   		 APP_DEBUG("<--  This is ATCommed Test!-->\r\n");
   		 Ql_Sleep(500);
    }

}


//This Task For Init AT Commed 
void proc_reserved5(void)
{
    for(;;)
    {
 //   s32 ret;
 //   ret = Ql_RIL_SendATCmd("AT\n", Ql_strlen("AT\n"), ATResponse_Handler, NULL, 0);
    APP_DEBUG("<-- This is Task5 Test!-->\r\n");
    Ql_Sleep(500);
    }

}

void proc_reserved6(void)
{
    for(;;)
    {
 //   s32 ret;
 //   ret = Ql_RIL_SendATCmd("AT\n", Ql_strlen("AT\n"), ATResponse_Handler, NULL, 0);
    APP_DEBUG("<-- This is Task6 Test!-->\r\n");
    Ql_Sleep(500);
    }

}
void proc_reserved7(void)
{
    for(;;)
    {
 //   s32 ret;
 //   ret = Ql_RIL_SendATCmd("AT\n", Ql_strlen("AT\n"), ATResponse_Handler, NULL, 0);
    APP_DEBUG("<-- This is Task7 Test!-->\r\n");
    Ql_Sleep(500);
    }

}
void proc_reserved8(void)
{
    for(;;)
    {
 //   s32 ret;
 //   ret = Ql_RIL_SendATCmd("AT\n", Ql_strlen("AT\n"), ATResponse_Handler, NULL, 0);
    APP_DEBUG("<-- This is Task8 Test!-->\r\n");
    Ql_Sleep(500);
    }

}
void proc_reserved9(void)
{
    for(;;)
    {
 //   s32 ret;
 //   ret = Ql_RIL_SendATCmd("AT\n", Ql_strlen("AT\n"), ATResponse_Handler, NULL, 0);
    APP_DEBUG("<-- This is Task9 Test!-->\r\n");
    Ql_Sleep(500);
    }

}
void proc_reserved10(void)
{
    for(;;)
    {
 //   s32 ret;
 //   ret = Ql_RIL_SendATCmd("AT\n", Ql_strlen("AT\n"), ATResponse_Handler, NULL, 0);
    APP_DEBUG("<-- This is Task10 Test!-->\r\n");
    Ql_Sleep(500);
    }

}
