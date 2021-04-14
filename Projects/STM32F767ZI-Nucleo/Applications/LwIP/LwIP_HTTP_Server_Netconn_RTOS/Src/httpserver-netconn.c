/**
******************************************************************************
* @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Src/httpser-netconn.c 
* @author  MCD Application Team
* @brief   Basic http server implementation using LwIP netconn API  
******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
* All rights reserved.</center></h2>
*
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted, provided that the following conditions are met:
*
* 1. Redistribution of source code must retain the above copyright notice, 
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* 3. Neither the name of STMicroelectronics nor the names of other 
*    contributors to this software may be used to endorse or promote products 
*    derived from this software without specific written permission.
* 4. This software, including modifications and/or derivative works of this 
*    software, must execute solely and exclusively on microcontroller or
*    microprocessor devices manufactured by or for STMicroelectronics.
* 5. Redistribution and use of this software other than as permitted under 
*    this license is void and will automatically terminate your rights under 
*    this license. 
*
* THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
* PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
* RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
* SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/apps/fs.h"
#include "string.h"
#include "httpserver-netconn.h"
#include "cmsis_os.h"
//#include "jsmn.h"
//#include "ArduinoJson.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "json-maker.h"
#include "tiny-json.h"
#include "stm32f7xx_nucleo_144.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define WEBSERVER_THREAD_PRIO    ( osPriorityAboveNormal )

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u32_t nPageHits = 0;

/* Format of dynamic web page: the page header */
static const unsigned char PAGE_START[] = {0
//  0x3c,0x21,0x44,0x4f,0x43,0x54,0x59,0x50,0x45,0x20,0x68,0x74,0x6d,0x6c,0x20,0x50,
//  0x55,0x42,0x4c,0x49,0x43,0x20,0x22,0x2d,0x2f,0x2f,0x57,0x33,0x43,0x2f,0x2f,0x44,
//  0x54,0x44,0x20,0x48,0x54,0x4d,0x4c,0x20,0x34,0x2e,0x30,0x31,0x2f,0x2f,0x45,0x4e,
//  0x22,0x20,0x22,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x77,0x77,0x77,0x2e,0x77,0x33,
//  0x2e,0x6f,0x72,0x67,0x2f,0x54,0x52,0x2f,0x68,0x74,0x6d,0x6c,0x34,0x2f,0x73,0x74,
//  0x72,0x69,0x63,0x74,0x2e,0x64,0x74,0x64,0x22,0x3e,0x0d,0x0a,0x3c,0x68,0x74,0x6d,
//  0x6c,0x3e,0x0d,0x0a,0x3c,0x68,0x65,0x61,0x64,0x3e,0x0d,0x0a,0x20,0x20,0x3c,0x74,
//  0x69,0x74,0x6c,0x65,0x3e,0x53,0x54,0x4d,0x33,0x32,0x46,0x37,0x78,0x78,0x54,0x41,
//  0x53,0x4b,0x53,0x3c,0x2f,0x74,0x69,0x74,0x6c,0x65,0x3e,0x0d,0x0a,0x20,0x20,0x3c,
//  0x6d,0x65,0x74,0x61,0x20,0x68,0x74,0x74,0x70,0x2d,0x65,0x71,0x75,0x69,0x76,0x3d,
//  0x22,0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x54,0x79,0x70,0x65,0x22,0x0d,0x0a,
//  0x20,0x63,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x3d,0x22,0x74,0x65,0x78,0x74,0x2f,0x68,
//  0x74,0x6d,0x6c,0x3b,0x20,0x63,0x68,0x61,0x72,0x73,0x65,0x74,0x3d,0x77,0x69,0x6e,
//  0x64,0x6f,0x77,0x73,0x2d,0x31,0x32,0x35,0x32,0x22,0x3e,0x0d,0x0a,0x20,0x20,0x3c,
//  0x6d,0x65,0x74,0x61,0x20,0x68,0x74,0x74,0x70,0x2d,0x65,0x71,0x75,0x69,0x76,0x3d,
//  0x22,0x72,0x65,0x66,0x72,0x65,0x73,0x68,0x22,0x20,0x63,0x6f,0x6e,0x74,0x65,0x6e,
//  0x74,0x3d,0x22,0x31,0x22,0x3e,0x0d,0x0a,0x20,0x20,0x3c,0x6d,0x65,0x74,0x61,0x20,
//  0x63,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x3d,0x22,0x4d,0x53,0x48,0x54,0x4d,0x4c,0x20,
//  0x36,0x2e,0x30,0x30,0x2e,0x32,0x38,0x30,0x30,0x2e,0x31,0x35,0x36,0x31,0x22,0x20,
//  0x6e,0x61,0x6d,0x65,0x3d,0x22,0x47,0x45,0x4e,0x45,0x52,0x41,0x54,0x4f,0x52,0x22,
//  0x3e,0x0d,0x0a,0x20,0x20,0x3c,0x73,0x74,0x79,0x6c,0x65,0x20,0x3d,0x22,0x66,0x6f,
//  0x6e,0x74,0x2d,0x77,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x6e,0x6f,0x72,0x6d,0x61,
//  0x6c,0x3b,0x20,0x66,0x6f,0x6e,0x74,0x2d,0x66,0x61,0x6d,0x69,0x6c,0x79,0x3a,0x20,
//  0x56,0x65,0x72,0x64,0x61,0x6e,0x61,0x3b,0x22,0x3e,0x3c,0x2f,0x73,0x74,0x79,0x6c,
//  0x65,0x3e,0x0d,0x0a,0x3c,0x2f,0x68,0x65,0x61,0x64,0x3e,0x0d,0x0a,0x3c,0x62,0x6f,
//  0x64,0x79,0x3e,0x0d,0x0a,0x3c,0x68,0x34,0x3e,0x3c,0x73,0x6d,0x61,0x6c,0x6c,0x20,
//  0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,0x74,0x2d,0x66,0x61,0x6d,0x69,
//  0x6c,0x79,0x3a,0x20,0x56,0x65,0x72,0x64,0x61,0x6e,0x61,0x3b,0x22,0x3e,0x3c,0x73,
//  0x6d,0x61,0x6c,0x6c,0x3e,0x3c,0x62,0x69,0x67,0x3e,0x3c,0x62,0x69,0x67,0x3e,0x3c,
//  0x62,0x69,0x67,0x0d,0x0a,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,
//  0x74,0x2d,0x77,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x62,0x6f,0x6c,0x64,0x3b,0x22,
//  0x3e,0x3c,0x62,0x69,0x67,0x3e,0x3c,0x73,0x74,0x72,0x6f,0x6e,0x67,0x3e,0x3c,0x65,
//  0x6d,0x3e,0x3c,0x73,0x70,0x61,0x6e,0x0d,0x0a,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,
//  0x22,0x66,0x6f,0x6e,0x74,0x2d,0x73,0x74,0x79,0x6c,0x65,0x3a,0x20,0x69,0x74,0x61,
//  0x6c,0x69,0x63,0x3b,0x22,0x3e,0x53,0x54,0x4d,0x33,0x32,0x46,0x37,0x78,0x78,0x20,
//  0x4c,0x69,0x73,0x74,0x20,0x6f,0x66,0x20,0x74,0x61,0x73,0x6b,0x73,0x20,0x61,0x6e,
//  0x64,0x0d,0x0a,0x74,0x68,0x65,0x69,0x72,0x20,0x73,0x74,0x61,0x74,0x75,0x73,0x3c,
//  0x2f,0x73,0x70,0x61,0x6e,0x3e,0x3c,0x2f,0x65,0x6d,0x3e,0x3c,0x2f,0x73,0x74,0x72,
//  0x6f,0x6e,0x67,0x3e,0x3c,0x2f,0x62,0x69,0x67,0x3e,0x3c,0x2f,0x62,0x69,0x67,0x3e,
//  0x3c,0x2f,0x62,0x69,0x67,0x3e,0x3c,0x2f,0x62,0x69,0x67,0x3e,0x3c,0x2f,0x73,0x6d,
//  0x61,0x6c,0x6c,0x3e,0x3c,0x2f,0x73,0x6d,0x61,0x6c,0x6c,0x3e,0x3c,0x2f,0x68,0x34,
//  0x3e,0x0d,0x0a,0x3c,0x68,0x72,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x77,0x69,
//  0x64,0x74,0x68,0x3a,0x20,0x31,0x30,0x30,0x25,0x3b,0x20,0x68,0x65,0x69,0x67,0x68,
//  0x74,0x3a,0x20,0x32,0x70,0x78,0x3b,0x22,0x3e,0x3c,0x73,0x70,0x61,0x6e,0x0d,0x0a,
//  0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,0x74,0x2d,0x77,0x65,0x69,
//  0x67,0x68,0x74,0x3a,0x20,0x62,0x6f,0x6c,0x64,0x3b,0x22,0x3e,0x0d,0x0a,0x3c,0x2f,
//  0x73,0x70,0x61,0x6e,0x3e,0x3c,0x73,0x70,0x61,0x6e,0x20,0x73,0x74,0x79,0x6c,0x65,
//  0x3d,0x22,0x66,0x6f,0x6e,0x74,0x2d,0x77,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x62,
//  0x6f,0x6c,0x64,0x3b,0x22,0x3e,0x0d,0x0a,0x3c,0x74,0x61,0x62,0x6c,0x65,0x20,0x73,
//  0x74,0x79,0x6c,0x65,0x3d,0x22,0x77,0x69,0x64,0x74,0x68,0x3a,0x20,0x39,0x36,0x31,
//  0x70,0x78,0x3b,0x20,0x68,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x33,0x30,0x70,0x78,
//  0x3b,0x22,0x20,0x62,0x6f,0x72,0x64,0x65,0x72,0x3d,0x22,0x31,0x22,0x0d,0x0a,0x20,
//  0x63,0x65,0x6c,0x6c,0x70,0x61,0x64,0x64,0x69,0x6e,0x67,0x3d,0x22,0x32,0x22,0x20,
//  0x63,0x65,0x6c,0x6c,0x73,0x70,0x61,0x63,0x69,0x6e,0x67,0x3d,0x22,0x32,0x22,0x3e,
//  0x0d,0x0a,0x20,0x20,0x3c,0x74,0x62,0x6f,0x64,0x79,0x3e,0x0d,0x0a,0x20,0x20,0x20,
//  0x20,0x3c,0x74,0x72,0x3e,0x0d,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x3c,0x74,0x64,
//  0x0d,0x0a,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,0x74,0x2d,0x66,
//  0x61,0x6d,0x69,0x6c,0x79,0x3a,0x20,0x56,0x65,0x72,0x64,0x61,0x6e,0x61,0x3b,0x20,
//  0x66,0x6f,0x6e,0x74,0x2d,0x77,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x62,0x6f,0x6c,
//  0x64,0x3b,0x20,0x66,0x6f,0x6e,0x74,0x2d,0x73,0x74,0x79,0x6c,0x65,0x3a,0x20,0x69,
//  0x74,0x61,0x6c,0x69,0x63,0x3b,0x20,0x62,0x61,0x63,0x6b,0x67,0x72,0x6f,0x75,0x6e,
//  0x64,0x2d,0x63,0x6f,0x6c,0x6f,0x72,0x3a,0x20,0x72,0x67,0x62,0x28,0x35,0x31,0x2c,
//  0x20,0x35,0x31,0x2c,0x20,0x32,0x35,0x35,0x29,0x3b,0x20,0x74,0x65,0x78,0x74,0x2d,
//  0x61,0x6c,0x69,0x67,0x6e,0x3a,0x20,0x63,0x65,0x6e,0x74,0x65,0x72,0x3b,0x22,0x3e,
//  0x3c,0x73,0x6d,0x61,0x6c,0x6c,0x3e,0x3c,0x61,0x0d,0x0a,0x20,0x68,0x72,0x65,0x66,
//  0x3d,0x22,0x2f,0x53,0x54,0x4d,0x33,0x32,0x46,0x37,0x78,0x78,0x2e,0x68,0x74,0x6d,
//  0x6c,0x22,0x3e,0x3c,0x73,0x70,0x61,0x6e,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,
//  0x63,0x6f,0x6c,0x6f,0x72,0x3a,0x20,0x77,0x68,0x69,0x74,0x65,0x3b,0x22,0x3e,0x48,
//  0x6f,0x6d,0x65,0x0d,0x0a,0x70,0x61,0x67,0x65,0x3c,0x2f,0x73,0x70,0x61,0x6e,0x3e,
//  0x3c,0x2f,0x61,0x3e,0x3c,0x2f,0x73,0x6d,0x61,0x6c,0x6c,0x3e,0x3c,0x2f,0x74,0x64,
//  0x3e,0x0d,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x3c,0x74,0x64,0x0d,0x0a,0x20,0x73,
//  0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,0x74,0x2d,0x66,0x61,0x6d,0x69,0x6c,
//  0x79,0x3a,0x20,0x56,0x65,0x72,0x64,0x61,0x6e,0x61,0x3b,0x20,0x66,0x6f,0x6e,0x74,
//  0x2d,0x77,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x62,0x6f,0x6c,0x64,0x3b,0x20,0x66,
//  0x6f,0x6e,0x74,0x2d,0x73,0x74,0x79,0x6c,0x65,0x3a,0x20,0x69,0x74,0x61,0x6c,0x69,
//  0x63,0x3b,0x20,0x62,0x61,0x63,0x6b,0x67,0x72,0x6f,0x75,0x6e,0x64,0x2d,0x63,0x6f,
//  0x6c,0x6f,0x72,0x3a,0x20,0x72,0x67,0x62,0x28,0x35,0x31,0x2c,0x20,0x35,0x31,0x2c,
//  0x20,0x32,0x35,0x35,0x29,0x3b,0x20,0x74,0x65,0x78,0x74,0x2d,0x61,0x6c,0x69,0x67,
//  0x6e,0x3a,0x20,0x63,0x65,0x6e,0x74,0x65,0x72,0x3b,0x22,0x3e,0x3c,0x61,0x0d,0x0a,
//  0x20,0x68,0x72,0x65,0x66,0x3d,0x22,0x53,0x54,0x4d,0x33,0x32,0x46,0x37,0x78,0x78,
//  0x41,0x44,0x43,0x2e,0x68,0x74,0x6d,0x6c,0x22,0x3e,0x3c,0x73,0x70,0x61,0x6e,0x20,
//  0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,0x74,0x2d,0x77,0x65,0x69,0x67,
//  0x68,0x74,0x3a,0x20,0x62,0x6f,0x6c,0x64,0x3b,0x22,0x3e,0x3c,0x2f,0x73,0x70,0x61,
//  0x6e,0x3e,0x3c,0x2f,0x61,0x3e,0x3c,0x73,0x6d,0x61,0x6c,0x6c,0x3e,0x3c,0x61,0x0d,
//  0x0a,0x20,0x68,0x72,0x65,0x66,0x3d,0x22,0x2f,0x53,0x54,0x4d,0x33,0x32,0x46,0x37,
//  0x78,0x78,0x54,0x41,0x53,0x4b,0x53,0x2e,0x68,0x74,0x6d,0x6c,0x22,0x3e,0x3c,0x73,
//  0x70,0x61,0x6e,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x63,0x6f,0x6c,0x6f,0x72,
//  0x3a,0x20,0x77,0x68,0x69,0x74,0x65,0x3b,0x22,0x3e,0x4c,0x69,0x73,0x74,0x0d,0x0a,
//  0x6f,0x66,0x20,0x74,0x61,0x73,0x6b,0x73,0x3c,0x2f,0x73,0x70,0x61,0x6e,0x3e,0x3c,
//  0x2f,0x61,0x3e,0x3c,0x2f,0x73,0x6d,0x61,0x6c,0x6c,0x3e,0x3c,0x2f,0x74,0x64,0x3e,
//  0x0d,0x0a,0x20,0x20,0x20,0x20,0x3c,0x2f,0x74,0x72,0x3e,0x0d,0x0a,0x20,0x20,0x3c,
//  0x2f,0x74,0x62,0x6f,0x64,0x79,0x3e,0x0d,0x0a,0x3c,0x2f,0x74,0x61,0x62,0x6c,0x65,
//  0x3e,0x0d,0x0a,0x3c,0x62,0x72,0x3e,0x0d,0x0a,0x3c,0x2f,0x73,0x70,0x61,0x6e,0x3e,
//  0x3c,0x73,0x70,0x61,0x6e,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,0x6e,
//  0x74,0x2d,0x77,0x65,0x69,0x67,0x68,0x74,0x3a,0x20,0x62,0x6f,0x6c,0x64,0x3b,0x22,
//  0x3e,0x3c,0x2f,0x73,0x70,0x61,0x6e,0x3e,0x3c,0x73,0x6d,0x61,0x6c,0x6c,0x3e,0x3c,
//  0x73,0x70,0x61,0x6e,0x0d,0x0a,0x20,0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x66,0x6f,
//  0x6e,0x74,0x2d,0x66,0x61,0x6d,0x69,0x6c,0x79,0x3a,0x20,0x56,0x65,0x72,0x64,0x61,
//  0x6e,0x61,0x3b,0x22,0x3e,0x4e,0x75,0x6d,0x62,0x65,0x72,0x20,0x6f,0x66,0x20,0x70,
//  0x61,0x67,0x65,0x20,0x68,0x69,0x74,0x73,0x3a,0x0d,0x0a,0x00
};

//Server Test code start
struct led_status {
  bool led1;
  bool led2;
  bool led3;
};

struct weather {
    int temp;
    int hum;
};

struct time {
    int hour;
    int minute;
};

struct measure {
    struct weather weather;
    struct time time;
};

struct data {
    char const* city;
    char const* street;
    struct measure measure;
    int samples[ 4 ];
};
/** Convert a weather structure in a JSON string.
  * @param dest Destination memory block.
  * @param src Source structure.
  * @return The length of the null-terminated string in dest. */
 /* "name":{"led1":true,"led2":true,"led3":true}, */
int led_status_to_json( char* dest, struct led_status const* data_src ) {
    char* temp_p = dest;  // temp_p always points to the null character
    temp_p = json_objOpen( temp_p, NULL );              // --> {\0
    temp_p = json_bool( temp_p, "led1", data_src->led1 ); // --> {"led1":true,\0
    temp_p = json_bool( temp_p, "led2", data_src->led2 ); // --> {"led1":true,"led2":true,\0   // --> "name":{"temp":22,"hum":45,\0
    temp_p = json_bool( temp_p, "led3", data_src->led3 ); // --> {"led1":true,"led2":true,"led3":true,\0
    temp_p = json_objClose( temp_p );                   // --> {"led1":true,"led2":true,"led3":true},\0
    temp_p = json_end( temp_p ); // --> "name":{"led1":true,"led2":true,"led3":true}\0
    return (temp_p - dest);
}
/* Add a time object property in a JSON string.
  "name":{"temp":-5,"hum":48}, */
char* json_weather( char* dest, char const* name, struct weather const* weather ) {
    dest = json_objOpen( dest, name );              // --> "name":{\0
    dest = json_int( dest, "temp", weather->temp ); // --> "name":{"temp":22,\0
    dest = json_int( dest, "hum", weather->hum );   // --> "name":{"temp":22,"hum":45,\0
    dest = json_objClose( dest );                   // --> "name":{"temp":22,"hum":45},\0
    return dest;
}

/* Add a time object property in a JSON string.
  "name":{"hour":18,"minute":32}, */
char* json_time( char* dest, char const* name, struct time const* time ) {
    dest = json_objOpen( dest, name );
    dest = json_int( dest, "hour",   time->hour   );
    dest = json_int( dest, "minute", time->minute );
    dest = json_objClose( dest );
    return dest;
}

/* Add a measure object property in a JSON string.
 "name":{"weather":{"temp":-5,"hum":48},"time":{"hour":18,"minute":32}}, */
char* json_measure( char* dest, char const* name, struct measure const* measure ) {
    dest = json_objOpen( dest, name );
    dest = json_weather( dest, "weather", &measure->weather );
    dest = json_time( dest, "time", &measure->time );
    dest = json_objClose( dest );
    return dest;
}

/* Add a data object property in a JSON string. */
char* json_data( char* dest, char const* name, struct data const* data ) {
    dest = json_objOpen( dest, NULL );
    dest = json_str( dest, "city",   data->city );
    dest = json_str( dest, "street", data->street );
    dest = json_measure( dest, "measure", &data->measure );
    dest = json_arrOpen( dest, "samples" );
    for( int i = 0; i < 4; ++i )
        dest = json_int( dest, NULL, data->samples[i] );
    dest = json_arrClose( dest );
    dest = json_objClose( dest );
    return dest;
}

/** Convert a data structure to a root JSON object.
  * @param dest Destination memory block.
  * @param data Source data structure.
  * @return  The JSON string length. */
int data_to_json( char* dest, struct data const* data ) {
    char* p = json_data( dest, NULL, data );
    p = json_end( p );
    return p - dest;
}

//Server test code end
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
* @brief serve tcp connection  
* @param conn: pointer on connection structure 
* @retval None
*/
static void http_server_serve(struct netconn *conn) 
{
  struct netbuf *inbuf;
  err_t recv_err;
  char* buf;
  static u16_t buflen;
  struct fs_file file;
  struct led_status ledStatus_s;
  char json_output[512];
  json_t tokens[30];
  /* Read the data from the port, blocking if nothing yet there. 
  We assume the request (the part we care about) is in one netbuf */
  recv_err = netconn_recv(conn, &inbuf);
  
  if (recv_err == ERR_OK)
  {
    if (netconn_err(conn) == ERR_OK) 
    {
      //buf is the payload
      netbuf_data(inbuf, (void**)&buf, &buflen);
      
      /* Is this an HTTP GET command? (only check the first 5 chars, since
      there are other formats for GET, and we're keeping it very simple )*/
      if ((buflen >=5) && (strncmp(buf, "GET /", 5) == 0))
      {
        /* Check if request to get ST.gif */ 
        if (strncmp((char const *)buf,"GET /STM32F7xx_files/ST.gif",27)==0)
        {
          fs_open(&file, "/STM32F7xx_files/ST.gif"); 
          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
          fs_close(&file);
        }   
        /* Check if request to get stm32.jpeg */
        else if (strncmp((char const *)buf,"GET /STM32F7xx_files/stm32.jpg",30)==0)
        {
          fs_open(&file, "/STM32F7xx_files/stm32.jpg"); 
          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
          fs_close(&file);
        }
        else if (strncmp((char const *)buf,"GET /STM32F7xx_files/logo.jpg", 29) == 0)                                           
        {
          /* Check if request to get ST logo.jpg */
          fs_open(&file, "/STM32F7xx_files/logo.jpg"); 
          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
          fs_close(&file);
        }
        else if(strncmp(buf, "GET /STM32F7xxTASKS.html", 24) == 0)
        {
          /* Load dynamic page */
          DynWebPage(conn);
        }
        else if((strncmp(buf, "GET /applications.json", 22) == 0)) 
        {
          /* Load STM32F7xx page */
          fs_open(&file, "/applications.json"); 
          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
          fs_close(&file);
        }
        else if (strncmp((char const *)buf,"GET /jQuery/jquery.js", 21) == 0)                                           
        {
          /* Check if request to get ST logo.jpg */
          fs_open(&file, "/jQuery/jquery.js"); 
          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
          fs_close(&file);
        }
        else if (strncmp((char const *)buf,"GET /bootstrap/bootstrap.css", 28) == 0)                                           
        {
          /* Check if request to get ST logo.jpg */
          fs_open(&file, "/bootstrap/bootstrap.css"); 
          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
          fs_close(&file);
        }
        else if((strncmp(buf, "GET /STM32F7xx.html", 19) == 0)||(strncmp(buf, "GET / ", 6) == 0)) 
        {
          /* Load STM32F7xx page */
          fs_open(&file, "/STM32F7xx.html"); 
          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
          fs_close(&file);
        }
        //Start of the test string for embedded webserver
        else if((strncmp(buf, "GET /style.css", 14) == 0)||(strncmp(buf, "GET / ", 6) == 0)) 
        {
          /* Load css styles page */
          fs_open(&file, "/style.css"); 
          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
          fs_close(&file);
        }
        else if((strncmp(buf, "GET /functions.js", 17) == 0)||(strncmp(buf, "GET / ", 6) == 0)) 
        {
          /* Load functions js page */
          fs_open(&file, "/functions.js"); 
          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
          fs_close(&file);
        }
        
        
        else 
        {
          /* Load Error page */
          fs_open(&file, "/404.html"); 
          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
          fs_close(&file);
        }
      }
      //This is for the POST request
      /* Is this an HTTP POST command? (only check the first 6 chars, since
      there are other formats for POST, and we're keeping it very simple )*/
      else if ((buflen >=6) && (strncmp(buf, "POST /", 6) == 0))
      {
       // char *SuccessfulPOSTResponse = "Nothing";
        //netconn_write(conn, (const unsigned char*)buf, (size_t)strlen(buf), NETCONN_NOCOPY);
        char* payload = strchr((const char*)buf,'{');
        json_t const* json = json_create(payload,tokens,(sizeof(tokens)/sizeof (*tokens)));
      //  netconn_write(conn, (const unsigned char*)payload, (size_t)strlen(payload), NETCONN_NOCOPY);
        if(json)
        {
          volatile int test = 2;
        }
       // json_t const* led1On= json_getProperty(json,"led1On");
        //json_t const* led2On= json_getProperty(json,"led2On");
       // json_t const* led3On= json_getProperty(json,"led3On");
        
        char const* led1Value = json_getPropertyValue(json,"led1On");
        char const* led2Value = json_getPropertyValue(json,"led2On");
        char const* led3Value = json_getPropertyValue(json,"led3On");
        /* Check if request to POST  */ 
        
        if((strncmp(buf, "POST /STM32F7xx.html", 20) == 0)||(strncmp(buf, "POST / ", 7) == 0))
        {
         // char* payload = strchr(( char*)buf,'{');
          
                    //if((strncmp(payload, "ledOn\":true", 11) == 0))
         // if((strstr(payload, "led1On=true"))!=NULL)
          if(strcmp(led1Value, "true") == 0)
        {
          /* Turn On LED 1 on client request*/
           BSP_LED_On(LED1);
           ledStatus_s.led1 = true;
          
        }else{
        
          BSP_LED_Off(LED1);
          ledStatus_s.led1 = false;
        }
       // if((strstr(payload, "led2On=true"))!=NULL)
        if(strcmp(led2Value, "true") == 0)
        {
          /* Turn On LED 2 on client request*/
           BSP_LED_On(LED2);
           ledStatus_s.led2 = true;
          
        }else
        {
          BSP_LED_Off(LED2);
          ledStatus_s.led2 = false;
        }
        //if((strstr(payload, "led3On=true"))!=NULL)
        if(strcmp(led3Value, "true") == 0)
        {
          /* Turn On LED 2 on client request*/
           BSP_LED_On(LED3);
           ledStatus_s.led3 = true;
          
          
        }else
        {
          BSP_LED_Off(LED3);
          ledStatus_s.led3 = false;
        }
        // netconn_write(conn, (const unsigned char*)payload, (size_t)strlen(payload), NETCONN_NOCOPY);
           int json_data_length = led_status_to_json(json_output, &ledStatus_s);
    //  if(json_data_length<= buflen)
           netconn_write(conn, (const unsigned char*)json_output, (size_t)strlen(json_output), NETCONN_NOCOPY);
      }
      
      
          
        //  netconn_write(conn, (const unsigned char*), (size_t)strlen(payload), NETCONN_NOCOPY);
//          if((strncmp(buf, "POST /STM32F7xx.html", 31) == 0))
//        {
//          /* Turn On LED 2 on client request*/
//           BSP_LED_On(LED2);
//           SuccessfulPOSTResponse = "LEDOn";
//           netconn_write(conn, (const unsigned char*)SuccessfulPOSTResponse, (size_t)strlen(SuccessfulPOSTResponse), NETCONN_NOCOPY);
//          
//        }else
//        {
//          BSP_LED_Off(LED2);
//          SuccessfulPOSTResponse = "LEDNotfound";
//          netconn_write(conn, (const unsigned char*)SuccessfulPOSTResponse, (size_t)strlen(SuccessfulPOSTResponse), NETCONN_NOCOPY);
//        }
//          char *data = "{ 'StatusCode': 200, 'Message': 'Success'}";
//          int resultCode;
//          jsmn_parser parser;
//          jsmntok_t token[128]; /* We expect no more than 128 JSON tokens */
//          
//          jsmn_init(&parser);
//          resultCode = jsmn_parse(&parser, data, strlen(data), token, 128);
          
          //char *buffer;
          //Json maker test code starts
        /*
          static struct data const data = {
        .city    = "liverpool",
        .street  = "mathew",
        .measure = {
            .weather = {
                .hum  = 65,
                .temp = 25
            },
            .time = {
                .hour   = 14,
                .minute = 31
            }
        },
        .samples = {
             25,
             65,
            -37,
            512
        }
    };
    char buff[512];
    int len = data_to_json( buff, &data );
          
          //Json maker test code ends
          
         // char *SuccessfulPOSTResponse = "{'StatusCode': 200, 'Message': 'Success'}";
         // sprintf(buffer,"\{\StatusCode\:\%d\,\Message\:%s}",200,'Suc');
         // sprintf(buffer,"{\'StatusCode\'\)";
         // netconn_write(conn, (const unsigned char*)SuccessfulPOSTResponse, (size_t)strlen(SuccessfulPOSTResponse), NETCONN_NOCOPY);
         // netconn_write(conn, (const unsigned char*)buffer, (size_t)strlen(buffer), NETCONN_NOCOPY);
          netconn_write(conn, (const unsigned char*)buff, (size_t)strlen(buff), NETCONN_NOCOPY);*/
         
      } 
        //         if((strncmp(buf, "POST /STM32F7xx.html", 20) == 0)||(strncmp(buf, "POST / ", 7) == 0)) 
        //        {
        //          /* Load STM32F7xx page */ 
        //          fs_open(&file, "/jQuery/jquery.js"); 
        //          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
        //          fs_close(&file);
        //        }
        
      }
      else
      {
        //To do
      }
  }
      //End of the test string for embedded webserver
  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);
  
  /* Delete the buffer (netconn_recv gives us ownership,
  so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);
}


/**
* @brief  http server thread 
* @param arg: pointer on argument(not used here) 
* @retval None
*/
static void http_server_netconn_thread(void *arg)
{ 
  struct netconn *conn, *newconn;
  err_t err, accept_err;
  
  /* Create a new TCP connection handle */
  conn = netconn_new(NETCONN_TCP);
  
  if (conn!= NULL)
  {
    /* Bind to port 80 (HTTP) with default IP address */
    err = netconn_bind(conn, NULL, 80);
    
    if (err == ERR_OK)
    {
      /* Put the connection into LISTEN state */
      netconn_listen(conn);
      
      while(1) 
      {
        /* accept any icoming connection */
        accept_err = netconn_accept(conn, &newconn);
        if(accept_err == ERR_OK)
        {
          /* serve connection */
          http_server_serve(newconn);
          
          /* delete connection */
          netconn_delete(newconn);
        }
      }
    }
  }
}

/**
* @brief  Initialize the HTTP server (start its thread) 
* @param  none
* @retval None
*/
void http_server_netconn_init()
{
  sys_thread_new("HTTP", http_server_netconn_thread, NULL, DEFAULT_THREAD_STACKSIZE, WEBSERVER_THREAD_PRIO);
}

/**
* @brief  Create and send a dynamic Web Page. This page contains the list of 
*         running tasks and the number of page hits. 
* @param  conn pointer on connection structure 
* @retval None
*/
void DynWebPage(struct netconn *conn)
{
  portCHAR PAGE_BODY[512];
  portCHAR pagehits[10] = {0};
  
  memset(PAGE_BODY, 0,512);
  
  /* Update the hit count */
  nPageHits++;
  sprintf(pagehits, "%d", (int)nPageHits);
  strcat(PAGE_BODY, pagehits);
  strcat((char *)PAGE_BODY, "<pre><br>Name          State  Priority  Stack   Num" );
  strcat((char *)PAGE_BODY, "<br>---------------------------------------------<br>");
  
  /* The list of tasks and their status */
  osThreadList((unsigned char *)(PAGE_BODY + strlen(PAGE_BODY)));
  strcat((char *)PAGE_BODY, "<br><br>---------------------------------------------");
  strcat((char *)PAGE_BODY, "<br>B : Blocked, R : Ready, D : Deleted, S : Suspended<br>");
  
  /* Send the dynamically generated page */
  netconn_write(conn, PAGE_START, strlen((char*)PAGE_START), NETCONN_COPY);
  netconn_write(conn, PAGE_BODY, strlen(PAGE_BODY), NETCONN_COPY);
}
