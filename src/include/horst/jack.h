#pragma once

namespace horst {

  struct jack_client {
    jack_client_t *m;

    jack_client (const std::string &client_name, jack_options_t options) :
      m (jack_client_open (client_name.c_str (), options, 0)) {
      DBG_ENTER
      if (m == 0) throw std::runtime_error ("horst: jack_client: Failed to open. Name: " + client_name);
      DBG_EXIT
    }

    ~jack_client () {
      DBG_ENTER
      jack_client_close (m);
      DBG_EXIT
    }
  };

  typedef std::shared_ptr<jack_client> jack_client_ptr;
}
