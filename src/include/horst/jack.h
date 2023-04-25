#pragma once

namespace horst
{
  extern "C" 
  {
    int unit_jack_process_callback (jack_nframes_t nframes, void *arg);
    int unit_jack_buffer_size_callback (jack_nframes_t buffer_size, void *arg);
    int unit_jack_sample_rate_callback (jack_nframes_t sample_rate, void *arg);
  }

  struct raw_jack_client 
  {
    jack_client_t *m_jack_client;

    jack_nframes_t m_sample_rate;
    jack_nframes_t m_buffer_size;

    raw_jack_client (jack_client_t *client) :
      m_jack_client (client),
      m_sample_rate (jack_get_sample_rate (m_jack_client)),
      m_buffer_size (jack_get_buffer_size (m_jack_client))
    {
      DBG_ENTER

      DBG("name: \"" << get_jack_client_name () << "\" sample_rate: " << m_sample_rate << " buffer_size: " << m_buffer_size)

      int ret;

      ret = jack_set_sample_rate_callback (m_jack_client, unit_jack_sample_rate_callback, (void *)this);
      if (ret != 0) throw std::runtime_error ("horst: plugin_unit: Failed to set sample rate callback");

      ret = jack_set_process_callback (m_jack_client, unit_jack_process_callback, (void *)this);
      if (ret != 0) throw std::runtime_error ("horst: plugin_unit: Failed to set buffer size callback");

      ret = jack_set_buffer_size_callback (m_jack_client, unit_jack_buffer_size_callback, (void *)this);
      if (ret != 0) throw std::runtime_error ("horst: plugin_unit: Failed to set buffer size callback");

      DBG_EXIT
    }

    virtual std::string get_jack_client_name () 
    {
      return jack_get_client_name (m_jack_client);
    }

    virtual int process_callback (jack_nframes_t nframes)
    {
      DBG_ENTER_EXIT
      return 0;
    }

    virtual int buffer_size_callback (jack_nframes_t buffer_size)
    {
      DBG_ENTER_EXIT
      return 0;
    }

    virtual int sample_rate_callback (jack_nframes_t sample_rate)
    {
      DBG_ENTER_EXIT
      return 0;
    }


    virtual ~raw_jack_client () {
        DBG_ENTER_EXIT
    }
  };

  extern "C" {
    int unit_jack_process_callback (jack_nframes_t nframes, void *arg)
    {
      return ((raw_jack_client *)arg)->process_callback (nframes);
    }

    int unit_jack_buffer_size_callback (jack_nframes_t buffer_size, void *arg)
    {
      return ((raw_jack_client *)arg)->buffer_size_callback (buffer_size);
    }

    int unit_jack_sample_rate_callback (jack_nframes_t sample_rate, void *arg)
    {
      return ((raw_jack_client *)arg)->sample_rate_callback (sample_rate);
    }
  }

  typedef std::shared_ptr<raw_jack_client> raw_jack_client_ptr;

  struct jack_client : public raw_jack_client 
  {
    jack_client (const std::string &client_name, jack_options_t options) :
      raw_jack_client (jack_client_open (client_name.c_str (), options, 0)) 
    {
      DBG("name: \"" << client_name.c_str() << "\" options: " << options)

      if (m_jack_client == 0) throw std::runtime_error ("horst: jack_client: Failed to open. Name: " + client_name);
      DBG_EXIT
    }

    ~jack_client () 
    {
      DBG_ENTER
      jack_client_close (m_jack_client);
      DBG_EXIT
    }
  };

  typedef std::shared_ptr<jack_client> jack_client_ptr;
}
