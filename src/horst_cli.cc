#include <horst/horst.h>
#include <unistd.h>
#include <time.h>

int main (int argc, char *argv[]) {
  horst::horst h;
  horst::unit_ptr unit = h.lv2(argv[1], "test", false);

  return 0;
}
