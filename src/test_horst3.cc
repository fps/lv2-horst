#include <horst/horst.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <stdexcept>
#include <vector>

int main (int argc, char *argv[]) {
  horst::horst h;

  const int N = 100;
  std::vector<horst::plugin_unit_ptr> units(N);
  for (size_t index = 0; index < N; ++index) {
    horst::plugin_unit_ptr unit = h.lv2("http://fps.io/plugins/state-variable-filter-v2", "",  false);
    units[index] = unit;
  }

  horst::connections c;


  c.add("system:capture_1", units[0]->get_jack_client_name() + ":in");
  for (size_t index = 1; index < N; ++index) {
    c.add(units[index-1]->get_jack_client_name() + ":out", units[index]->get_jack_client_name() + ":in");
  }
  c.add(units[N-1]->get_jack_client_name() + ":out", "system:playback_1");
  h.connect(c);
  std::string input;
  std::cin >> input;
  return 0;
}
