//Web server Authentication
#include "server_auth.h"

uint8_t check_user( char* user, char* password)
{
  uint8_t valid_user = 0;
  
  
  
  return valid_user;
  
}
uint8_t check_host( struct netconn *conn)
{
  uint8_t valid_host = 0;
  
  
  
  return valid_host;
  
  
}
uint8_t parse_header(char* payload_buf)
{
uint8_t success = 0;

  if((strstr(payload_buf, "Authentication: Basic"))!=NULL)
  {
  
  }

return success;
}