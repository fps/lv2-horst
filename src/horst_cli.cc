#include <horst/horst.h>
#include <unistd.h>
#include <time.h>
#include <iostream>

int main (int argc, char *argv[]) {
  horst::horst h;
  horst::unit_ptr unit = h.lv2(argv[1], "test", false);

  std::string input;
  std::cin >> input;
  return 0;
}
