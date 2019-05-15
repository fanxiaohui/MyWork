#ifdef __CUSTOMER2_CODE__
#include "custom_feature_def.h"
#include "ql_stdlib.h"
#include "ql_common.h"
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_error.h"
#include "ql_uart.h"
#include "ql_timer.h"
#include "ril_network.h"
#include "ril_socket.h"
#include "ril.h"
#include "ril_util.h"


#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  UART_PORT0
#define DBG_BUF_LEN   256
static char DBG_BUFFER[DBG_BUF_LEN];
#define APP_DEBUG(FORMAT,...) {\
    Ql_memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    Ql_sprintf(DBG_BUFFER,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace(DBG_BUFFER);\
    } else {\
        Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), \
		Ql_strlen((const char *)(DBG_BUFFER)));\
    }\
}
#else
#define APP_DEBUG(FORMAT,...) 
#endif


/*****************************************************************
* UART Param
******************************************************************/
#define SERIAL_RX_BUFFER_LEN  256
static u8 m_RxBuf_Uart[SERIAL_RX_BUFFER_LEN];

/*****************************************************************
* timer param
******************************************************************/

#define AT_TIMER_PERIOD      2000
#define AT_TIMER_ID          TIMER_ID_USER_START

#define SEND_BUFFER_LEN     64
#define RECV_BUFFER_LEN     64



s32 AT_Count=0;
typedef enum{
    AT_CPSMS_OFF,
	AT_CEREG,
    AT_CGATT,
    AT_QLWSERV,
    AT_QLWCONF,
    AT_QLWADDOBJ,
    AT_QLWOPEN,
    AT_QLWUPDATE,
    AT_QLWCFG,
    AT_QLWDATASEND,
    AT_QLWCLOSE,
    AT_CPSMS
}Enum_AT_STep;
static u8 AT_Step=AT_CPSMS_OFF;
static u8 sim_is_ready=0;


#define SRVADDR_LEN  32


static u8 m_send_buf[SEND_BUFFER_LEN];
//static u8 m_recv_buf[RECV_BUFFER_LEN];
//static u8 at_CPSMS_on[50] = "AT+CPSMS=1,,,\"00100001\",\"00100001\"\0\0";
static u8 at_CPSMS_on[16] = "AT+CPSMS=1\0\0";
static u8 at_CPSMS_off[16] = "AT+CPSMS=0\0\0";


#define  serv_port_strs "5683" 
static   u8 temps_AT_CEREG[16] = "AT+CEREG?\0\0";
static   u8 temps_AT_CGATT[16] = "AT+CGATT?\0\0";
static   u8 temps_AT_QLWSERV[45] = "AT+QLWSERV=\"180.101.147.115\",5683\0\0";
static   u8 temps_AT_serv_q[20] = "AT+QLWSERV?\0\0";
static   u8 temps_AT_conf_q[20] = "AT+QLWCONF?\0\0";
static   u8 temps_AT_QLWADDOBJ[32] = "AT+QLWADDOBJ=19,0,1,\"0\"\0\0";
static   u8 temps_AT_QLWOPEN[20] = "AT+QLWOPEN=0\0\0";
static   u8 temps_AT_QLWUPDATE[20] = "AT+QLWUPDATE\0\0";
static   u8 temps_AT_QLWCFG[32] = "AT+QLWCFG=\"dataformat\",1,1\0\0";
static   u8 temps_AT_QLWDATASEND[45] = "AT+QLWDATASEND=19,0,0,1,05,0x0000\0\0";
static   u8 temps_AT_QLWCLOSE[20] = "AT+QLWCLOSE\0\0";

static u8 res = 0;
static u8 AT_OVERTIMES = 25;
static u8 AT_serv_q = 0;
static u8 AT_conf_q = 0;
static u8 send_count_ = 0;


//01 smoke alarm----low:ok
// 03 smoke good
// 04 low Voltage----high :ok
// 05 OK
// 06 Voltage good
static u8 alarm_or_bat = 0x0;
static bool STATUS_smoke = FALSE;
static bool STATUS_voltage = FALSE;

static bool SEND_01 = FALSE;
static bool SEND_03 = FALSE;
static bool SEND_04 = FALSE;
static bool SEND_05 = FALSE;
static bool SEND_06 = FALSE;
static u8 count_03 = 0;
static u8 count_06 = 0;

static u8 io_ready = 0;
//static u8 io_read_bat_count = 0x0;
//static u8 io_read_alm_count = 0x0;
static bool rev_flag = FALSE;
static char IMEI_BUF[32];

static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, 
bool level, void* customizedPara);
static void Callback_Timer(u32 timerId, void* param);
//static void Callback_Timer_heartbeat(u32 timerId, void* param);
static s32 ReadSerialPort(Enum_SerialPort port, /*[out]*/u8* pBuffer, /*[in]*/u32 bufLen);

static s32 ATResponse_Handler(char* line, u32 len, void* userData);

void proc_main_task(s32 taskId)
{
    ST_MSG msg;
    s32 ret;

    // s_iMutexId = Ql_OS_CreateMutex();
    // Ql_OS_TakeMutex(s_iMutexId,0xFFFFFFFF);
    // Ql_OS_GiveMutex(s_iMutexId);
    
    Ql_GPIO_Uninit(PINNAME_NETLIGHT);
    Ql_GPIO_Init(PINNAME_NETLIGHT, PINDIRECTION_OUT, PINLEVEL_LOW,PINPULLSEL_PULLUP);


    Ql_GPIO_Uninit(PINNAME_GPIO1);
    Ql_GPIO_Init(PINNAME_GPIO1, PINDIRECTION_IN, PINLEVEL_LOW, PINPULLSEL_PULLDOWN);
    Ql_GPIO_Uninit(PINNAME_GPIO5);
    Ql_GPIO_Init(PINNAME_GPIO5, PINDIRECTION_IN, PINLEVEL_LOW, PINPULLSEL_PULLDOWN);

    // Register & open UART port
    ret = Ql_UART_Register(UART_PORT0, CallBack_UART_Hdlr, NULL);
    ret = Ql_UART_Open(UART_PORT0, 115200, FC_NONE);
    //ret = Ql_UART_Register(UART_PORT2, CallBack_UART_Hdlr, NULL);
    //ret = Ql_UART_Open(UART_PORT2, 115200, FC_NONE);
    APP_DEBUG("<--OpenCPU: boot.-->\r\n");

    Ql_Timer_Register(AT_TIMER_ID, Callback_Timer, NULL);
    Ql_Timer_Start(AT_TIMER_ID, AT_TIMER_PERIOD, TRUE);

    Ql_GPIO_SetLevel(PINNAME_NETLIGHT, PINLEVEL_HIGH);

    while(TRUE)
    {
        Ql_OS_GetMessage(&msg);
        switch(msg.message)
        {
    #ifdef __OCPU_RIL_SUPPORT__
            case MSG_ID_RIL_READY:
                APP_DEBUG("<-- RIL is ready -->\r\n");
                Ql_RIL_Initialize();
                sim_is_ready = 1;
                
            break;
    #endif
            case MSG_ID_URC_INDICATION:
            {
                switch (msg.param1)
                {
                    case URC_SIM_CARD_STATE_IND:
                        APP_DEBUG("<-- SIM Card Status:%d -->\r\n", msg.param2
);
                        
                    break;
                    default:
                    //APP_DEBUG("<-- Other URC: type=%d\r\n", msg.param1);
                    break;
                }
            }
            break;
            default:
            break;
        }
//01 smoke alarm
// 03 smoke good
// 04 low Voltage
// 05 OK
// 06 Voltage good
        {
            //APP_DEBUG("<-- \r\n" );
            // io_read_count = 0;
            u8 io_value = Ql_GPIO_GetLevel(PINNAME_GPIO1);
            if(io_value == 1){//low:ok
                STATUS_smoke = TRUE;				
                SEND_01 = TRUE;	
				SEND_03 = FALSE;
				count_03 = 0;				
            }else{
				if(STATUS_smoke){
					count_03++;
					if(count_03 > 8){
						SEND_03 = TRUE;
					}
				}
				
            }
            io_value = Ql_GPIO_GetLevel(PINNAME_GPIO5);
            if(io_value == 0){//high:ok
                STATUS_voltage = TRUE;
                SEND_04 = TRUE;
				SEND_06 = FALSE;
				count_06 = 0;
            }else{
				if(STATUS_voltage){
					count_06++;
					if(count_06 > 8){
						SEND_06 = TRUE;
					}
				}
				
            }
            io_ready = 0x03;
          
        }
        
        
    }
}

static void Callback_Timer(u32 timerId, void* param)
{
    s32 ret;

    if (AT_TIMER_ID == timerId)
    {        
        if(sim_is_ready!=1){
            //APP_DEBUG("<--sim not ready-->\r\n");
            return;
        }
        if(io_ready != 0x03){
            //APP_DEBUG("<--io not ready-->\r\n");
            return;
        }

        if(AT_Step == AT_QLWDATASEND){
            if(send_count_%5 != 0){
                send_count_++;
                return;
            }
        }
        AT_Count++;
        if(AT_Step == AT_CEREG){
			if(AT_Count > AT_OVERTIMES * 5){///////////////////////////overtimes 5 reset
				
				//return;            
				Ql_Reset(0);/////////////////////////////////////debug
			}
		}else{
			if(AT_Count > AT_OVERTIMES){///////////////////////////overtimes 5 reset
				
				//return;            
				Ql_Reset(0);/////////////////////////////////////debug
			}
		}
        Ql_memset(m_send_buf,0,SEND_BUFFER_LEN);
        s32 m_send_buf_len = 0;

        switch (AT_Step)
        {
        case AT_CPSMS_OFF:
        //Ql_RIL_SendATCmd((char*)at_CPSMS_off, Ql_strlen((char*)at_CPSMS_off), ATResponse_Handler, NULL, 0);
        Ql_memcpy(m_send_buf,at_CPSMS_off,Ql_strlen((char*)at_CPSMS_off));
        break;
		case AT_CEREG:
        Ql_memcpy(m_send_buf,temps_AT_CEREG,Ql_strlen((char*)temps_AT_CEREG));
        break;
        case AT_CGATT:
            //AT+CGATT?        
        Ql_memcpy(m_send_buf,temps_AT_CGATT,Ql_strlen((char*)temps_AT_CGATT)); 
        break;
        case AT_QLWSERV:
        //AT+QLWSERV="180.101.147.115",5683
        if(AT_serv_q == 0){
            Ql_memcpy(m_send_buf,temps_AT_QLWSERV,Ql_strlen((char*)temps_AT_QLWSERV));
        }else{
            Ql_memcpy(m_send_buf,temps_AT_serv_q,Ql_strlen((char*)temps_AT_serv_q));
        }
        break;
        case AT_QLWCONF:
//AT+QLWCONF="866971030535996"
        if(AT_conf_q == 0){
                if(Ql_strlen(IMEI_BUF) < 10){
                    ret = RIL_GetIMEI(IMEI_BUF);
                    if(ret != RIL_AT_SUCCESS){
                        APP_DEBUG("<--no imei.-->\r\n");                        
                        return;
                    }
                     APP_DEBUG("<--imei=%s.-->\r\n",IMEI_BUF);
                }
                m_send_buf[m_send_buf_len++]='A';
                m_send_buf[m_send_buf_len++]='T';
                m_send_buf[m_send_buf_len++]='+';
                m_send_buf[m_send_buf_len++]='Q';
                m_send_buf[m_send_buf_len++]='L';
                m_send_buf[m_send_buf_len++]='W';
                m_send_buf[m_send_buf_len++]='C';
                m_send_buf[m_send_buf_len++]='O';
                m_send_buf[m_send_buf_len++]='N';
                m_send_buf[m_send_buf_len++]='F';
                m_send_buf[m_send_buf_len++]='=';
                m_send_buf[m_send_buf_len++]='\"';

                ret = Ql_strlen(IMEI_BUF);
                for(int i=0;i<ret;i++){
                    m_send_buf[m_send_buf_len++]=IMEI_BUF[i];
                }
                m_send_buf[m_send_buf_len++]='\"';
                m_send_buf[m_send_buf_len++]='\0';
                m_send_buf[m_send_buf_len++]='\0';
        }else{
            Ql_memcpy(m_send_buf,temps_AT_conf_q,Ql_strlen((char*)temps_AT_conf_q));
        }
        break;
        case AT_QLWADDOBJ:
            //AT+QLWADDOBJ=19,0,1,"0"

        Ql_memcpy(m_send_buf,temps_AT_QLWADDOBJ,Ql_strlen((char*)temps_AT_QLWADDOBJ)); 
        break;
        case AT_QLWOPEN:
            //AT+QLWOPEN=0

        Ql_memcpy(m_send_buf,temps_AT_QLWOPEN,Ql_strlen((char*)temps_AT_QLWOPEN));
        break;
        case AT_QLWUPDATE:
            //AT+QLWUPDATE

        Ql_memcpy(m_send_buf,temps_AT_QLWUPDATE,Ql_strlen((char*)temps_AT_QLWUPDATE));
        break;
        case AT_QLWCFG:
            //AT+QLWCFG="dataformat",1,1

        Ql_memcpy(m_send_buf,temps_AT_QLWCFG,Ql_strlen((char*)temps_AT_QLWCFG));
        break;
        case AT_QLWDATASEND:
//AT+QLWDATASEND=19,0,0,1,05,0x0000
        send_count_++;
        Ql_memcpy(m_send_buf,temps_AT_QLWDATASEND,Ql_strlen((char*)temps_AT_QLWDATASEND));
//01 smoke alarm
// 03 smoke good
// 04 low Voltage
// 05 OK
// 06 Voltage good

            if(SEND_01&&SEND_04){
                if(alarm_or_bat == 0){
                    alarm_or_bat = 1;
                    res = '1';
                }else{
                    alarm_or_bat = 0;
                    res = '4';
                }
            }else if(SEND_01&& (!SEND_04)){
                if(alarm_or_bat == 0){
                    alarm_or_bat = 1;
                    res = '1';
                    
                }else{
                    alarm_or_bat = 0;
                    if(SEND_06){
                        res = '6';
                    }else{
                        res = '1';
                    }
                }
            }else if((!SEND_01)&& SEND_04){
                if(alarm_or_bat == 0){
                    alarm_or_bat = 1;
                    if(SEND_03){
                        res = '3';
                    }else{
                        res = '4';
                    }
                }else{
                    alarm_or_bat = 0;
					res = '4';
                }
            }else {                
                if(alarm_or_bat == 0){
                    alarm_or_bat = 1;
                    if(SEND_03){
                        res = '3';
                    }else{
						if(SEND_06){
							res = '6';
						}else{
							res = '5';
						}
                    }
                }else{
                    alarm_or_bat = 0;
					if(SEND_06){
                        res = '6';
                    }else{
                        if(SEND_03){
							res = '3';
						}else{
							res = '5';
						}
                    }
                }
            }
      
        m_send_buf[25] = res;
        break;
        case AT_QLWCLOSE:
//AT+QLWCLOSE

        Ql_memcpy(m_send_buf,temps_AT_QLWCLOSE,Ql_strlen((char*)temps_AT_QLWCLOSE));
        break;
        case AT_CPSMS:
            Ql_Timer_Stop(AT_TIMER_ID);
            Ql_RIL_SendATCmd((char*)at_CPSMS_on, Ql_strlen((char*)at_CPSMS_on), ATResponse_Handler, NULL, 0);

            Ql_GPIO_SetLevel(PINNAME_NETLIGHT, PINLEVEL_LOW);
        break;
        default:
        break;
        }

        APP_DEBUG("%s\n",m_send_buf);
       // m_send_buf[Ql_strlen((char*)m_send_buf)] = '\0';
        rev_flag = TRUE;
        Ql_RIL_SendATCmd((char*)m_send_buf, Ql_strlen((char*)m_send_buf), ATResponse_Handler, NULL, 0);
        
    }

}
static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{
    switch (msg)
    {
    case EVENT_UART_READY_TO_READ:        
        if(port == UART_PORT0){
           s32 totalBytes = ReadSerialPort(port, m_RxBuf_Uart, sizeof(m_RxBuf_Uart));

            if (totalBytes <= 0)
            {
                APP_DEBUG("<-- No data in UART buffer! -->\r\n");
                return;
            }
            {// Read data from UART
                s32 ret;
                char* pCh = NULL;
                
                // Echo
                //Ql_UART_Write(m_myUartPort, m_RxBuf_Uart, totalBytes);

                pCh = Ql_strstr((char*)m_RxBuf_Uart, "\r\n");
                if (pCh)
                {
                    *(pCh + 0) = '\0';
                    *(pCh + 1) = '\0';
                }

                // No permission for single <cr><lf>
                if (Ql_strlen((char*)m_RxBuf_Uart) == 0)
                {
                    return;
                }
                ret = Ql_RIL_SendATCmd((char*)m_RxBuf_Uart, totalBytes, ATResponse_Handler, NULL, 0);
                //ret = Ql_RIL_SendATCmd((char*)m_RxBuf_Uart, totalBytes, ATResponse_Handler, NULL, 5000);
            }
       }
       break;        
    case EVENT_UART_READY_TO_WRITE:
        break;
    default:
        break;
    }
}

static s32 ATResponse_Handler(char* line, u32 len, void* userData)
{
    APP_DEBUG("%s\r\n", (u8*)line);    
    if (Ql_RIL_FindLine(line, len, "OK"))
    {
        if((sim_is_ready==1)&& rev_flag){

            switch (AT_Step)
            {
            case AT_CPSMS_OFF:
            AT_Step = AT_CEREG;                  
            AT_Count =0;
            break;
            case AT_CGATT: 
			case AT_CEREG:
            break;
            case AT_QLWSERV:
            //if(AT_serv_q == 0){
               AT_Step = AT_QLWCONF;
                AT_Count =0; 
            //}
            break;
            case AT_QLWCONF:
            AT_Step = AT_QLWADDOBJ;
            AT_Count =0;
            break;
            case AT_QLWADDOBJ:
            AT_Step = AT_QLWOPEN;
            AT_Count =0;
            break;
            case AT_QLWOPEN:
            AT_Step = AT_QLWUPDATE;
            AT_Count =0;
            break;
            case AT_QLWUPDATE:
            AT_Step = AT_QLWCFG;
            AT_Count =0;
            break;
            case AT_QLWCFG:
            AT_Step = AT_QLWDATASEND;
            AT_Count =0;
            break;
            case AT_QLWDATASEND:
            AT_Count =0;

            if(res == '1'){
                
				SEND_01 = FALSE;				

            }else if(res == '3'){                
                SEND_03 = FALSE; 
				STATUS_smoke = FALSE;
            }else if(res == '4'){
                
				SEND_04 = FALSE;
				
            }else if(res == '5'){
				if((!STATUS_smoke) && (!STATUS_voltage)){
					AT_Step = AT_QLWCLOSE;
				}
            }else if(res == '6'){
                SEND_06 = FALSE;
				STATUS_voltage = FALSE;
            }
                        
            break;
            case AT_QLWCLOSE:
            AT_Step = AT_CPSMS;
            break;
            }
        }
        return  RIL_ATRSP_SUCCESS;
    }
    else if (Ql_RIL_FindLine(line, len, "ERROR"))
    {
        if(AT_Step == AT_QLWSERV){
            AT_serv_q = 1;
        }else if(AT_Step == AT_QLWCONF){
            AT_conf_q = 1;
        }
        rev_flag = FALSE;
        return  RIL_ATRSP_FAILED;
    }
    else if (Ql_RIL_FindString(line, len, "+CME ERROR"))
    {
        rev_flag = FALSE;
        return  RIL_ATRSP_FAILED;
    }
    else if (Ql_RIL_FindString(line, len, "+CMS ERROR:"))
    {
        rev_flag = FALSE;
        return  RIL_ATRSP_FAILED;
    }else if (Ql_RIL_FindLine(line, len, "CGATT: 1")){
        
        if(AT_Step == AT_CGATT){ 
            AT_Step = AT_QLWSERV;                  
            AT_Count =0;
            rev_flag = FALSE;
            
        }
    }else if (Ql_RIL_FindLine(line, len, "CEREG: 1,1")){
        
        if(AT_Step == AT_CEREG){ 
            AT_Step = AT_CGATT;                  
            AT_Count =0;
            rev_flag = FALSE;
            
        }
    }else if(Ql_RIL_FindLine(line, len, serv_port_strs)){
        if(AT_Step == AT_QLWSERV){
            AT_Step = AT_QLWCONF;
            AT_Count =0;
            rev_flag = FALSE;
        }
    }else if(Ql_RIL_FindLine(line, len, "+QLWCONF")){
        if(AT_Step == AT_QLWCONF){
            AT_Step = AT_QLWADDOBJ;
            AT_Count =0;
            rev_flag = FALSE;
        }
    }

    return RIL_ATRSP_CONTINUE; //continue wait
}

static s32 ReadSerialPort(Enum_SerialPort port, u8* pBuffer, u32 bufLen)
{
    s32 rdLen = 0;
    s32 rdTotalLen = 0;
    if (NULL == pBuffer || 0 == bufLen)
    {
        return -1;
    }
    Ql_memset(pBuffer, 0x0, bufLen);
    while (1)
    {
        rdLen = Ql_UART_Read(port, pBuffer + rdTotalLen, bufLen - rdTotalLen);
        if (rdLen <= 0)  // All data is read out, or Serial Port Error!
        {
            break;
        }
        rdTotalLen += rdLen;
        // Continue to read...
    }
    if (rdLen < 0) // Serial Port Error!
    {
        APP_DEBUG("<--Fail to read from port[%d]-->\r\n", port);
        return -99;
    }
    return rdTotalLen;
}

#endif

