


#include "custom_feature_def.h"
#include "ril.h"
#include "ril_util.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_trace.h"
#include "ql_uart.h"
#include "ql_system.h"
#include "ql_timer.h"


#include "ql_gpio.h"

#include "custom_Test.h"

#define URC_RCV_TASK_ID  main_task_id

//AT 指令的实现分为两个部分，一个是AT指令的发送；另一个是对AT指令返回参数的处理和判断。

// Implementation for AT
//

//s32 RIL_AT_TEST(/*[in]*/char* pAT, /*[in]*/u32 pATLen)
/*	{
	  if (!pAT || pATLen < 15)

		{
		return QL_RET_ERR_INVALID_PARAMETER;
		}

		Ql_memset(pAT, 0x0, pATLen);

	  return Ql_RIL_SendATCmd("AT", Ql_strlen("AT"), ATResponse_AT_Handler, pAT,0);
	}


*/

void proc_reserved4(void)
{

	 ST_MSG msg1;
	 s32 ret=0;

	APP_DEBUG("<-- this is AT commd test!-->\r\n");
	for(;;)
	{
	//	Ql_OS_GetMessage(&msg1);
	Ql_OS_GetMessage(&msg1);
        
    switch(msg1.message)
      {
		case MSG_ID_RIL_READY:
        APP_DEBUG("<-- RIL is ready -->\r\n");
         Ql_RIL_Initialize();      
        break;

        case MSG_ID_URC_INDICATION_TEST:
		switch(msg1.param1)
			{

				case 1:
				 APP_DEBUG("<-- test 0 -->\r\n");
				 Ql_Sleep(500);
			 	Ql_Sleep(500);
			   	Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_CFUN_STATE_IND, 1);
			 	 break;
				 case URC_AT_TEST_STATE_IND :
				 APP_DEBUG("<-- test 1 -->\r\n");
				 Ql_Sleep(500);
				 Ql_Sleep(500);
			 	Ql_OS_SendMessage(URC_RCV_TASK_ID, MSG_ID_URC_INDICATION, URC_CFUN_STATE_IND, 2);
			 	 break;
			}
		  break;
        default:
        APP_DEBUG("<-- Other URC: type=%d\r\n", 6);
          break;
		}
	}
}
/*
s32 RIL_GetIMEI(char* imei)
{
    char strAT[] = "AT+CGSN=1\n";
    if (NULL == imei)
    {
        return RIL_AT_INVALID_PARAM;
    }
    return Ql_RIL_SendATCmd(strAT, Ql_strlen(strAT), ATRsp_IMEI_Handler,(void*)imei, 0);
}

static s32 ATRsp_IMEI_Handler(char* line, u32 len, void* param)
{
    char* pHead = NULL;
    pHead = Ql_RIL_FindLine(line, len, "OK"); // find <CR><LF>OK<CR><LF>, <CR>OK<CR>£¬<LF>OK<LF>
    if (pHead)
    {  
        return RIL_ATRSP_SUCCESS;
    }

    pHead = Ql_RIL_FindLine(line, len, "ERROR");// find <CR><LF>ERROR<CR><LF>, <CR>ERROR<CR>£¬<LF>ERROR<LF>
    if (pHead)
    {  
        return RIL_ATRSP_FAILED;
    } 

    pHead = Ql_RIL_FindString(line, len, "+CME ERROR:");//fail
    if (pHead)
    {
        return RIL_ATRSP_FAILED;
    }
	pHead = Ql_RIL_FindString(line, len, "+CGSN:");//fail
    if (pHead)
    {
        char* p1 = NULL;
        char* p2 = NULL;
		
        p1 = Ql_strstr(pHead, ":");
        p2 = Ql_strstr(p1 + 1, "\r\n");
        if (p1 && p2)
        {
            Ql_memcpy((char*)param, p1 + 2, p2 - p1 - 2);
        }
    }
    return RIL_ATRSP_CONTINUE; //continue wait
}*/
