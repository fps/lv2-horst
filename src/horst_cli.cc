#include <horst/horst.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <stdexcept>

int main (int argc, char *argv[]) {
  horst::horst h;
  if (argc < 2) throw std::runtime_error ("Missing argument: LV2 plugin URI");
  horst::unit_ptr unit = h.lv2(argv[1], "test", false);

  std::string input;
  std::cin >> input;
  return 0;
}
