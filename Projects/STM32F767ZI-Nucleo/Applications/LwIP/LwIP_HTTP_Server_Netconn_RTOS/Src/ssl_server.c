/*
 *  Copyright (C) 2018, STMicroelectronics, all right reserved.
 *  Copyright (C) 2006-2018, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "main.h"
#include "cmsis_os.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif
#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_time       time
#define mbedtls_time_t     time_t
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#define mbedtls_exit            exit
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE
#endif

#include <string.h>
#include <stdint.h>
#include "server_auth.h"
/* External Includes to support mbedTLS---------------------------------------*/
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"

#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif


static mbedtls_net_context listen_fd, client_fd;
//static uint8_t buf[1024];
static char buf[1024];
static char In_buf[1024];
//static uint8_t request_buf[1024];
static const char *pers = "ssl_server";
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_x509_crt srvcert;
mbedtls_pk_context pkey;
/* Test header and status codes   ---------------------------------------*/

char const* auth_header = "WWW-Authenticate: Basic realm=\"Protected Area\"\r\n";
const char* un_auth_code = "HTTP/1.1 401 Unauthorized\r\n";
const char* success_code = "HTTP/1.0 200 OK\r\n";
static unsigned char* decoded_user_pass = NULL;
char* header_buff[50] ;
uint8_t LoggedIn = 0;

#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_context cache;
#endif
 /* C_Array File system to support on mbedTLS---------------------------------------*/
struct fs_file file;
#include "lwip/apps/fs.h"

void SSL_Server(void const *argument)
{
  int ret, len;
  UNUSED(argument);
// #if defined(MBEDTLS_PLATFORM_C)
//  mbedtls_platform_context platform_ctx;
//  
//  if( ( ret = mbedtls_platform_setup( &platform_ctx ) ) != 0 )
//  {
//    // Error handling
//  }
//#endif
  
  mbedtls_printf( "\n  . just for debugging..." ); //just for debugging
  
#ifdef MBEDTLS_MEMORY_BUFFER_ALLOC_C
  mbedtls_memory_buffer_alloc_init(memory_buf, sizeof(memory_buf));
#endif
  mbedtls_net_init( &listen_fd );
  mbedtls_net_init( &client_fd );
  mbedtls_ssl_init( &ssl );
  mbedtls_ssl_config_init( &conf );
#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_init( &cache );
#endif
  mbedtls_x509_crt_init( &srvcert );
  mbedtls_pk_init( &pkey );
  mbedtls_entropy_init( &entropy );
  mbedtls_ctr_drbg_init( &ctr_drbg );

  /*
   * 1. Load the certificates and private RSA key
   */
  mbedtls_printf( "\n  . Loading the server cert. and key..." );

  /*
   * This program uses embedded test certificates with mbedtls_x509_crt_parse().
   * web files are ROM based without file system.
   * Using mbedtls_x509_crt_parse_file() to read the server and CA certificates
   * Note: resuires the implmentation of the File I/O API using the FatFs, that is 
   * not implemented yet.
   */

  ret = mbedtls_x509_crt_parse( &srvcert, (const unsigned char *) mbedtls_test_srv_crt, mbedtls_test_srv_crt_len );
  if(ret != 0)
  {
    mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret );
    goto exit;
  }

  ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char *) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
  if( ret != 0 )
  {
    mbedtls_printf(" failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret);
    goto exit;
  }

  ret =  mbedtls_pk_parse_key(&pkey, (const unsigned char *) mbedtls_test_srv_key, mbedtls_test_srv_key_len, NULL, 0);

  if( ret != 0 )
  {
    mbedtls_printf(" failed\n  !  mbedtls_pk_parse_key returned %d\n\n", ret);
    goto exit;
  }

  mbedtls_printf( " ok\n" );

  /*
   * 2. Setup the listening TCP socket (port 4433 used for secure socket)
   */
  mbedtls_printf( "  . Bind on https://localhost:%s/ ...", SERVER_PORT );

  if((ret = mbedtls_net_bind(&listen_fd, NULL, SERVER_PORT , MBEDTLS_NET_PROTO_TCP )) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_net_bind returned %d\n\n", ret );
    goto exit;
  }

  mbedtls_printf( " ok\n" );

  /*
   * 3. Seed the RNG ( hardware based RNG used from stm32)
   */
  mbedtls_printf( "  . Seeding the random number generator..." );

  if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers, strlen( (char *)pers))) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
    goto exit;
  }

  mbedtls_printf( " ok\n" );

  /*
   * 4. Setup stuff for SSL data, ca chains etc.
   */
  mbedtls_printf( "  . Setting up the SSL data...." );

  if((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret );
    goto exit;
  }

  mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_conf_session_cache(&conf, &cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);
#endif

  mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);
  if((ret = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey)) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret );
    goto exit;
  }

  if((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
    goto exit;
  }

  mbedtls_printf( " ok\n" );

reset:
#ifdef MBEDTLS_ERROR_C
  if(ret != 0)
  {
    uint8_t error_buf[100];
    mbedtls_strerror( ret, (char *)error_buf, 100 );
    mbedtls_printf("Last error was: %d - %s\n\n", ret, error_buf );
  }
#endif
  
  mbedtls_net_free(&client_fd);

  mbedtls_ssl_session_reset(&ssl);

  /*
   * 5. Wait until a client connects
   */
  mbedtls_printf( "  . Waiting for a remote connection ...\n" );

  if((ret = mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL)) != 0)
  {
    mbedtls_printf(" failed\n  ! mbedtls_net_accept returned %d\n\n", ret);
    goto exit;
  }

  mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

  mbedtls_printf(" ok\n");

  /*
   * 6. Handshake
   */
  mbedtls_printf("  . Performing the SSL/TLS handshake...");

  while((ret = mbedtls_ssl_handshake(&ssl)) != 0)
  {
    if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      mbedtls_printf(" failed\n  ! mbedtls_ssl_handshake returned %d\n\n", ret);
      goto reset;
    }
  }

  mbedtls_printf(" ok\n");

  /*
   * 7. Read the HTTP Request
   */
  mbedtls_printf("  < Read from client:");
  do
  {
    len = sizeof(In_buf) - 1;
    memset(In_buf, 0, sizeof(In_buf));
    BSP_LED_Off(LED_GREEN);
    BSP_LED_Off(LED_RED);
    ret = mbedtls_ssl_read(&ssl, (unsigned char *)In_buf, len);

    if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      continue;
    }
    if(ret <= 0)
    {
      switch(ret)
      {
        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
          mbedtls_printf(" connection was closed gracefully\n");
          HAL_Delay(100);
          BSP_LED_On(LED_RED);
          break;

        case MBEDTLS_ERR_NET_CONN_RESET:
          mbedtls_printf(" connection was reset by peer\n");
          HAL_Delay(100);
          BSP_LED_On(LED_RED);
          break;

        default:
          mbedtls_printf(" mbedtls_ssl_read returned -0x%x\n", -ret);
          HAL_Delay(100);
          BSP_LED_On(LED_RED);
          break;
      }
      
      HAL_Delay(100);
      BSP_LED_On(LED_GREEN);
      break;
    }

    //len = ret;
   // mbedtls_printf(" %d bytes read\n\n%s", len, (char *) buf);
    if(ret > 0)
    {
     
      //breaks after read successful (positive ret value)
      break;
    }
  } while(1);

  /*
   * 8. Write the HTTP response 200 in case of success
   */
  mbedtls_printf( "  > Write to client:" );
//  len = sprintf((char *) buf, HTTP_RESPONSE, mbedtls_ssl_get_ciphersuite(&ssl));
//  
//  while((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0)
//   // while((ret = mbedtls_ssl_write(&ssl, (const unsigned char*)(HTTP_AUTH_RESPONSE), (size_t)strlen(HTTP_AUTH_RESPONSE))) <= 0)
//  {
//    if(ret == MBEDTLS_ERR_NET_CONN_RESET)
//    {
//      mbedtls_printf(" failed\n  ! peer closed the connection\n\n");
//      goto reset;
//    }
//
//    if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
//    {
//      mbedtls_printf( " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
//      goto exit;
//    }
//  }
  len = sprintf((char *) buf, HTTP_AUTH_RESPONSE, mbedtls_ssl_get_ciphersuite(&ssl));
  while((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0)
   // while((ret = mbedtls_ssl_write(&ssl, (const unsigned char*)(HTTP_AUTH_RESPONSE), (size_t)strlen(HTTP_AUTH_RESPONSE))) <= 0)
  {
    if(ret == MBEDTLS_ERR_NET_CONN_RESET)
    {
      mbedtls_printf(" failed\n  ! peer closed the connection\n\n");
      goto reset;
    }

    if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      mbedtls_printf( " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
      goto exit;
    }
  }
  
  if ( ret>0)
  {
    
 // sprintf((char *) header_buff, HTTP_RESPONSE, mbedtls_ssl_get_ciphersuite(&ssl));
    
 //Working webpages reading goes here   
      if((strncmp(In_buf, "GET /STM32F7xx.html", 19) == 0)||(strncmp(In_buf, "GET / ", 6) == 0)) 
        { 
        //  mbedtls_ssl_write(&ssl, (const unsigned char*)(auth_header), (size_t)strlen(auth_header));
   // fs_open(&file, "/STM32F7xx.html"); 
    
   // mbedtls_ssl_write(&ssl, (const unsigned char*)(file.data), (size_t)file.len);
   // fs_close(&file);
          
       }
    /* Check if request to get ST.gif */ 
//       else if (strncmp((char const *)In_buf,"GET /STM32F7xx_files/ST.gif",27)==0)
//        {
//          fs_open(&file, "/STM32F7xx_files/ST.gif"); 
//          mbedtls_ssl_write(&ssl, (const unsigned char*)(file.data), (size_t)file.len);
//          fs_close(&file);
//        }   
//        /* Check if request to get stm32.jpeg */
//        else if (strncmp((char const *)In_buf,"GET /STM32F7xx_files/stm32.jpg",30)==0)
//        {
//          fs_open(&file, "/STM32F7xx_files/stm32.jpg"); 
//          mbedtls_ssl_write(&ssl, (const unsigned char*)(file.data), (size_t)file.len);
//          fs_close(&file);
//        }
//        else if (strncmp((char const *)In_buf,"GET /STM32F7xx_files/logo.jpg", 29) == 0)                                           
//        {
//          /* Check if request to get ST logo.jpg */
//          fs_open(&file, "/STM32F7xx_files/logo.jpg"); 
//          mbedtls_ssl_write(&ssl, (const unsigned char*)(file.data), (size_t)file.len);
//          fs_close(&file);
//        }
//        else if(strncmp((char const *)In_buf, "GET /STM32F7xxTASKS.html", 24) == 0)
//        {
//          /* Load dynamic page */
//        //  DynWebPage(ssl);
//        }
 
  }   
#if 0   
     if((strncmp(buf, "GET /STM32F7xx.html", 19) == 0)||(strncmp(buf, "GET / ", 6) == 0)) 
        { 
    fs_open(&file, "/STM32F7xx.html"); 
    mbedtls_ssl_write(&ssl, (const unsigned char*)(file.data), (size_t)file.len);
    fs_close(&file);
        }
    /* Check if request to get ST.gif */ 
       else if (strncmp((char const *)buf,"GET /STM32F7xx_files/ST.gif",27)==0)
        {
          fs_open(&file, "/STM32F7xx_files/ST.gif"); 
          mbedtls_ssl_write(&ssl, (const unsigned char*)(file.data), (size_t)file.len);
          fs_close(&file);
        }   
        /* Check if request to get stm32.jpeg */
        else if (strncmp((char const *)buf,"GET /STM32F7xx_files/stm32.jpg",30)==0)
        {
          fs_open(&file, "/STM32F7xx_files/stm32.jpg"); 
          mbedtls_ssl_write(&ssl, (const unsigned char*)(file.data), (size_t)file.len);
          fs_close(&file);
        }
        else if (strncmp((char const *)buf,"GET /STM32F7xx_files/logo.jpg", 29) == 0)                                           
        {
          /* Check if request to get ST logo.jpg */
          fs_open(&file, "/STM32F7xx_files/logo.jpg"); 
          mbedtls_ssl_write(&ssl, (const unsigned char*)(file.data), (size_t)file.len);
          fs_close(&file);
        }
        else if(strncmp((char const *)buf, "GET /STM32F7xxTASKS.html", 24) == 0)
        {
          /* Load dynamic page */
        //  DynWebPage(ssl);
        }
#endif
     //Authorization test code start
#if 0
//char* un_auth_code = g_psHTTPHeaderStrings[HTTP_UNAUTHORIZED];
          
          if((!LoggedIn)&&(login_attempt == 0)){
          fs_open(&file, "/401.html");
          netconn_write(conn, (const unsigned char*)un_auth_code, (size_t)strlen(un_auth_code), NETCONN_NOCOPY);
          netconn_write(conn, (const unsigned char*)auth_header, (size_t)strlen(auth_header), NETCONN_NOCOPY);
          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
          login_attempt++;
          fs_close(&file);
          }
          else if((!LoggedIn)&&(login_attempt == 1)){
          
          //fs_open(&file, "/401.html");
         // netconn_write(conn, (const unsigned char*)un_auth_code, (size_t)strlen(un_auth_code), NETCONN_NOCOPY);
         // netconn_write(conn, (const unsigned char*)auth_header, (size_t)strlen(auth_header), NETCONN_NOCOPY);
         // netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
         // fs_close(&file);
   char* auth_str = strstr(buf, "Authorization: Basic");    
   char *rest;
   char *token;
   char *user_pass;
   //char* decoded_user_pass;
   size_t size;
   //Retrieve the authentication scheme and password
   token = strtok_r(auth_str, " \t", &rest);
   token = strtok_r(NULL, " \t", &rest);
   user_pass = strtok_r(NULL, " \n", &rest);
  // b64_decode(user_pass, decoded_user_pass, size);
 //  decoded_user_pass  = base64_decode(user_pass,strlen(user_pass), &size);
    //Properly terminate the string
      //      decoded_user_pass[size] = '\0';

//#if 0
//    //To do: check if decoding is Successful 
//            //Properly terminate the string
//            user_pass[size] = '\0';
//            //Check whether a separator is present
//           char* separator = strchr(user_pass, ':');
//
//            //Separator found?
//            if(separator != NULL)
//            {
//               //Split the line
//               *separator = '\0';
//
//               //Point to the password
//               user_pass = separator + 1;
//               //Save password
//               
//               
//            }   
//#endif       
           // netconn_write(conn, (const unsigned char*)success_code, (size_t)strlen(success_code), NETCONN_NOCOPY);
            netconn_write(conn, (const unsigned char*)user_pass, (size_t)strlen(user_pass), NETCONN_NOCOPY);
            
           // base64_cleanup();
          }
          /* Load STM32F7xx page */
       //   else{
//          fs_open(&file, "/STM32F7xx.html"); 
//          netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
//          fs_close(&file);
//          }
#endif //auth test code


  len = ret;
  mbedtls_printf(" %d bytes written\n\n%s\n", len, (char *) buf);

  mbedtls_printf("  . Closing the connection...");

  while((ret = mbedtls_ssl_close_notify(&ssl)) < 0)
  {
    if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      mbedtls_printf( " failed\n  ! mbedtls_ssl_close_notify returned %d\n\n", ret );
      goto reset;
    }
  }

  if (ret == 0)
  {
    BSP_LED_On(LED_GREEN);
    mbedtls_printf( " ok\n" );
  }
  
  ret = 0;
  goto reset;

exit:
  BSP_LED_Off(LED_GREEN);
  BSP_LED_On(LED_RED);

  mbedtls_net_free( &client_fd );
  mbedtls_net_free( &listen_fd );

  mbedtls_x509_crt_free( &srvcert );
  mbedtls_pk_free( &pkey );
  mbedtls_ssl_free( &ssl );
  mbedtls_ssl_config_free( &conf );
#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_free( &cache );
#endif
  mbedtls_ctr_drbg_free( &ctr_drbg );
  mbedtls_entropy_free( &entropy );
#if defined(MBEDTLS_PLATFORM_C)
 // mbedtls_platform_teardown( &platform_ctx );
#endif

}
