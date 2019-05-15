/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2019
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ql_gpio.h 
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   IIC API defines.
 *
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/


#ifndef __Custom_iic__H__
#define __Custom_iic__H__

#include "ql_gpio.h"

#define IIC_SDA_H Ql_GPIO_SetLevel(PINNAME_GPIO5,PINLEVEL_HIGH);   //SDA 1
#define IIC_SDA_L Ql_GPIO_SetLevel(PINNAME_GPIO5,PINLEVEL_LOW);	   //SDA 0


#define IIC_SCL_H Ql_GPIO_SetLevel(PINNAME_GPIO4,PINLEVEL_HIGH);    //SCL 1
#define IIC_SCL_L Ql_GPIO_SetLevel(PINNAME_GPIO4,PINLEVEL_LOW);    //SCL 0

#define IIC_SCL  PINNAME_GPIO4
#define IIC_SDA  PINNAME_GPIO5

void Custom_GPIO_Init (void);
void Custom_GPIO_Init_IIC(void);

#endif  // __Custom_H__