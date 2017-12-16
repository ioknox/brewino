/*#include "msgpck.hpp"
#include "../main.hpp"

bool msgpck_read_string(Stream *stream, const char *excepted)
{
  uint32_t length = strlen(excepted)+1;
  uint32_t size;
  char buf[length];
  memset(buf, 0, length);

  bool res = msgpck_read_string(stream, (char*)buf, length-1, &size);
  res &= size == (length-1);
  res &= strcmp(buf, excepted) == 0;
  return res;
}
*/
