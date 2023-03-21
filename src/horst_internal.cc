#include <horst/horst.h>

extern "C" {
  static horst::horst_jack *the_horst = 0;

  static void init () __attribute__((constructor));
  static void init () {
    // std::cout << "init\n";
    the_horst = new horst::horst_jack;
  }

  static void fini () __attribute((destructor));
  static void fini () {
    // std::cout << "fini\n";
    delete the_horst;
  }

  // JACK_LIB_EXPORT
  int jack_initialize (jack_client_t *jack_client, const char *load_init) {
    using namespace horst;
    // std::cout << "horst: jack_initialize\n";
    std::stringstream stream (load_init);
    std::string type;
    stream >> type;
    std::string uri;
    stream >> uri;

    try {
      plugin_unit *unit =  new plugin_unit (plugin_ptr (new lv2_plugin (the_horst->m_lilv_world, the_horst->m_lilv_plugins, uri)), "", jack_client);
    } catch (std::runtime_error &e) {
      std::cout << "horst: Failed to load plugin: " << e.what () << "\n";
      return 1;
    }
    return 0;
  }

  // JACK_LIB_EXPORT
  void jack_finish (void *arg) {
    delete (horst::plugin_unit*)arg;
  }
}
