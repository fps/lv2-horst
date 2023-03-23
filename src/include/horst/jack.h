#pragma once

namespace horst {

  struct jack_client {
    jack_client_t *m;

    jack_client (const std::string &client_name, jack_options_t options) :
      m (jack_client_open (client_name.c_str (), options, 0)) {
      if (m == 0) throw std::runtime_error ("horst: jack_client: Failed to open. Name: " + client_name);
    }

    ~jack_client () {
      jack_client_close (m);
    }
  };

  typedef std::shared_ptr<jack_client> jack_client_ptr;
}