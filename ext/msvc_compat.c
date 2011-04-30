/* These methods are needed to make MSVC link with the GCC precompiled
   libraries.
 */

#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int snprintf(char *str, size_t s, const char *fmt, ...) 
{
  va_list ap;
  int str_l;

  s;

  va_start(ap, fmt);
  str_l = vsprintf(str, fmt, ap);
  va_end(ap);

  return str_l;
}

void __chkstk(void) 
{
}

int32_t __divdi3(int64_t divident, int32_t divisor)
{
  return (int32_t)(divident / divisor);
}

int32_t __moddi3(int64_t divident, int32_t divisor)
{
  return (int32_t)(divident % divisor);
}

uint32_t __udivdi3(uint64_t divident, uint32_t divisor)
{
  return (uint32_t)(divident / divisor);
}

uint32_t __umoddi3(uint64_t divident, uint32_t divisor)
{
  return (uint32_t)(divident % divisor);
}
