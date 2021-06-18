#ifndef SERVER_AUTH_H
#define SERVER_AUTH_H

#include "httpserver-netconn.h"
#include <stdint.h>
#include <stdio.h>
#define HTTP_EN_USER_AUTH    1


/**
 * @brief HTTP status code
 **/

typedef struct
{
  /*
   Note: Based on the 
  ref: https://developer.mozilla.org/en-US/docs/Web/HTTP/Status 
  max code is 599 therefore using uint16_t
  */
   uint32_t status_code;                  
   const char message[23];
} HttpStatus;

/**
 * @brief HTTP status codes
 **/
static const HttpStatus statusCodeList[] =
{
   //Success
   {200, "OK"},
   {201, "Created"},
   {202, "Accepted"},
   {204, "No Content"},
   //Redirection
   {301, "Moved Permanently"},
   {302, "Found"},
   {304, "Not Modified"},
   //Client error
   {400, "Bad Request"},
   {401, "Unauthorized"},
   {403, "Forbidden"},
   {404, "Not Found"},
   //Server error
   {500, "Internal Server Error"},
   {501, "Not Implemented"},
   {502, "Bad Gateway"},
   {503, "Service Unavailable"}
};


/*The check_account function checks if an user account for provided credentials exist in the user database. 

It is called from the Web server to check if the user with provided credentials is allowed to access the Web pages or not.

The argument user points to a buffer containing the user name which was typed in from the browser. The argument passw specifies the password. Both arguments are 0-terminated strings.

*/

uint8_t check_user( char* user, char* password);


uint8_t check_host( struct netconn *conn);


uint8_t parse_header(char* payload_buf);






#endif   //end of SERVER_AUTH_H