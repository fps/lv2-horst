#pragma once

#include <vector>
#include <string>
#include <boost/python.hpp>
#include <horst/lart/junk.h>
#include <jack/jack.h>


namespace horst {
  typedef std::vector<float> port_buffer_t;
  typedef std::vector<port_buffer_t> port_buffers_t;

  struct unit {
    port_buffers_t m_port_buffers;

    virtual ~unit () {}
  };

  typedef std::shared_ptr<lart::junk<unit>> unit_ptr;

  struct ladspa_unit : public unit {

  };

  struct lv2_unit : public unit {

  };

  struct rack {
    std::vector<unit_ptr> units;
  };

  typedef std::shared_ptr<lart::junk<rack>> rack_ptr;

  struct horst {
    std::vector<rack_ptr> m_racks;
  };

  typedef std::shared_ptr<lart::junk<horst>> horst_ptr;

  extern "C" {
    int horst_jack_process_callback (jack_nframes_t nframes, void *arg) {
      // horst &the_horst = *((horst*)arg);
      return 0;
    }

    int horst_jack_buffer_size_callback (jack_nframes_t buffer_size, void *arg) {
      // horst &the_horst = *((horst*)arg);
      return 0;
    }

    int horst_jack_sample_rate_callback (jack_nframes_t sample_rate, void *arg) {
      // horst &the_horst = *((horst*)arg);
      return 0;
    }
  }

  struct horst_jack {
    horst_ptr m_horst;

    jack_client_t *m_jack_client;

    horst_jack (std::string jack_client_name) :
      m_horst (new lart::junk<horst>) {
      m_jack_client = jack_client_open (jack_client_name.c_str(), JackUseExactName, 0);
      if (m_jack_client == 0) {
        throw std::runtime_error ("Failed to open jack client");
      }

      int ret;

      ret = jack_set_process_callback (m_jack_client, horst_jack_process_callback, (void *)this);
      if (ret != 0) {
        throw std::runtime_error ("Failed to register process callback");
      }

      ret = jack_set_buffer_size_callback(m_jack_client, horst_jack_buffer_size_callback, (void *)this);
      if (ret != 0) {
        throw std::runtime_error ("Failed to register buffer size callback");
      }

      ret = jack_set_sample_rate_callback(m_jack_client, horst_jack_buffer_size_callback, (void *)this);
      if (ret != 0) {
        throw std::runtime_error ("Failed to register sample rate callback");
      }

      ret = jack_activate (m_jack_client);
      if (ret != 0) {
        throw std::runtime_error ("Failed to activate jack client");
      }
    }

    ~horst_jack () {
      jack_client_close (m_jack_client);
    }
  };

}

namespace bp = boost::python;
BOOST_PYTHON_MODULE(horst)
{
  bp::class_<horst::horst_jack>("horst", bp::init<std::string>());
}

