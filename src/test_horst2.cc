#include <vector>
#include <horst/horst.h>
#include <iostream>

horst::horst h;

std::vector<std::string> uris = {
  "http://calf.sourceforge.net/plugins/Gate",
  "http://calf.sourceforge.net/plugins/Compressor",
  "http://fps.io/plugins/clipping.tanh",
  "http://calf.sourceforge.net/plugins/EnvelopeFilter",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://guitarix.sourceforge.net/plugins/gxts9#ts9sim",
  "http://moddevices.com/plugins/mod-devel/BigMuffPi",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://guitarix.sourceforge.net/plugins/gx_cabinet#CABINET",
  "http://drobilla.net/plugins/mda/Leslie",
  "http://calf.sourceforge.net/plugins/MultiChorus",
  "http://calf.sourceforge.net/plugins/Phaser",
  "http://calf.sourceforge.net/plugins/Reverb",
  "http://calf.sourceforge.net/plugins/Reverb",
  "http://drobilla.net/plugins/mda/DubDelay",
  "https://ca9.eu/lv2/bolliedelay",
  "http://calf.sourceforge.net/plugins/Saturator",
  "http://calf.sourceforge.net/plugins/Limiter"
};

std::vector<horst::plugin_unit_ptr> units;

int main (int argc, char *argv[]) {
  for (size_t index = 0; index < uris.size(); ++index) {
    units.push_back (h.lv2 (uris[index], "", false));
  }

  std::cout << "Send EOF to exit (CTRL-D)\n";
  int n;
  std::cin >> n;
}
