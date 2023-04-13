#include <horst/horst.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <stdexcept>

int main (int argc, char *argv[]) {
  horst::horst h;
  if (argc < 2) throw std::runtime_error ("Missing argument: LV2 plugin URI");
  std::string client_name = "horst_cli";
  if (argc > 2) client_name = argv[2];
  horst::plugin_unit_ptr unit = h.lv2(argv[1], client_name,  false);

  std::string input;
  std::cin >> input;
  return 0;
}
