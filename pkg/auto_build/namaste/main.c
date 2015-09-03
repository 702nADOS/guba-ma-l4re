#include <stdio.h>
#include <l4/util/util.h>
#include <unistd.h>

int
main(void)
{
  for (;;)
    {
      puts("namaste");
      l4_sleep(100);
    }
}
