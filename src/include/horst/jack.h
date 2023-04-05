#pragma once

namespace horst {

  struct raw_jack_client {
    jack_client_t *m;

    jack_nframes_t m_sample_rate;
    jack_nframes_t m_buffer_size;

    raw_jack_client (jack_client_t *client) :
      m (client),
      m_sample_rate (jack_get_sample_rate (m)),
      m_buffer_size (jack_get_buffer_size (m))
    {
      DBG_ENTER_EXIT
    }

    virtual ~raw_jack_client () {
        DBG_ENTER_EXIT
    }
  };

  typedef std::shared_ptr<raw_jack_client> raw_jack_client_ptr;

  struct jack_client : public raw_jack_client {
    jack_client (const std::string &client_name, jack_options_t options) :
      raw_jack_client (jack_client_open (client_name.c_str (), options, 0)) {
      DBG("name: " << client_name.c_str() << " options: " << options)

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
