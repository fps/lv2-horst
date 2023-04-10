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

std::vector<horst::lv2_plugin_ptr> plugins;

extern "C" {
  int process (jack_nframes_t n, void *arg) {
    for (size_t index = 0; index < plugins.size(); ++index) {
      plugins[index]->run (128);
    }
      
    return 0;
  }
}

int main (int argc, char *argv[]) {
  jack_client_t *client = jack_client_open ("test-horst", JackNullOption, 0);
  for (size_t index = 0; index < uris.size(); ++index) {
    horst::lv2_plugin_ptr p(new horst::lv2_plugin (h.m_lilv_world, h.m_lilv_plugins, uris[index]));
    p->instantiate (jack_get_sample_rate (client), jack_get_buffer_size (client));
    plugins.push_back (p);
  }

  jack_set_process_callback (client, process, 0);
  jack_activate (client);

  std::cout << "Send EOF to exit (CTRL-D)\n";
  int n;
  std::cin >> n;
  jack_deactivate (client);
  jack_client_close (client);
}
