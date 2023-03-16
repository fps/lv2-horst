#pragma once

#include <vector>
#include <list>
#include <string>
#include <functional>
#include <boost/python.hpp>
#include <horst/lart/junk.h>
#include <horst/lart/ringbuffer.h>
#include <horst/lart/heap.h>
#include <jack/jack.h>
#include <lilv/lilv.h>


namespace horst {
  struct jack_port {
    pthread_t m_self;
    jack_client_t *m_client;
    jack_port_t *m_port;

    jack_port (jack_client_t *client, jack_port_t *port) :
      m_self (pthread_self ()),
      m_client (client),
      m_port (port) {

    }

    ~jack_port () {
      assert (pthread_self () == m_self);
      std::cout << "~jack_port\n";
      jack_port_unregister (m_client, m_port);
    }
  };

  typedef std::shared_ptr<jack_port> jack_port_ptr;

  struct unit {
    std::vector<jack_port_ptr> m_ports;

    virtual ~unit () {}
  };

  typedef std::shared_ptr<unit> unit_ptr;

  struct ladspa_unit : public unit {

  };

  struct lv2_unit : public unit {

  };

  struct rack {
    std::list<unit_ptr> units;
  };

  typedef std::shared_ptr<rack> rack_ptr;

  struct horst {
    pthread_t m_self;
    std::list<rack_ptr> m_racks;
    std::vector<horst_jack_port> m_ports;

    horst () :
      m_self (pthread_self ()) {

    }

    ~horst () {
      assert (m_self == pthread_self ());
      std::cout << "~horst\n";
    }
  };

  typedef std::shared_ptr<lart::junk<horst>> horst_ptr;

  extern "C" {
    int horst_jack_process_callback (jack_nframes_t nframes, void *arg);    
    int horst_jack_buffer_size_callback (jack_nframes_t buffer_size, void *arg);
    int horst_jack_sample_rate_callback (jack_nframes_t sample_rate, void *arg);
  }

  struct horst_jack {
    pthread_t m_self;

    lart::ringbuffer<std::function<void()>> m_commands;
    lart::heap m_heap;

    horst_ptr m_processing_horst;
    horst m_horst;

    jack_client_t *m_jack_client;

    horst_jack (const std::string &jack_client_name) :
      m_self (pthread_self ()),
      m_commands(1024),
      m_processing_horst (m_heap.add(horst())) {

      m_jack_client = jack_client_open (jack_client_name.c_str (), JackUseExactName, 0);
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
      assert (pthread_self () == m_self);
      jack_client_close (m_jack_client);
    }

    void insert_rack (int rack_index) {
      if ((size_t)rack_index > m_horst.m_racks.size ()) {
        throw std::runtime_error ("Rack index out of bounds");
      }
      auto it = m_horst.m_racks.begin ();
      for (int index = 0; index < rack_index; ++index) ++it;
      m_horst.m_racks.insert (it, std::make_shared<rack> (rack ()));

      horst_ptr the_new_horst = m_heap.add (horst (m_horst)); 
      m_commands.write([the_new_horst, this]() mutable { this->m_processing_horst = the_new_horst; the_new_horst = horst_ptr(); }); 
    }
  
    void remove_rack (int rack_index) {
  
    }

    int number_of_racks () {
      return 0;
    }
  
    void insert_lv2_plugin (int rack_index, int plugin_index, std::string &uri) {
  
    }
  
    void insert_ladspa_plugin (int rack_index, int plugin_index, std::string library_file_name, std::string plugin_label) {
  
    }
  
    void set_plugin_parameter (int rack_index, int plugin_index, int port_index, float value) {
  
    }

    void remove_plugin (int rack_index, int plugin_index) {
  
    }

    int number_of_plugins (int rack_index) {
      return 0;
    }
  };

  extern "C" {
    int horst_jack_process_callback (jack_nframes_t nframes, void *arg) {
      horst_jack &the_horst_jack = *((horst_jack*)arg);

      while (the_horst_jack.m_commands.can_read ()) {
        the_horst_jack.m_commands.snoop () ();
        the_horst_jack.m_commands.read_advance ();
      } 
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

}

namespace bp = boost::python;
BOOST_PYTHON_MODULE(horst)
{
  bp::class_<horst::horst_jack>("horst", bp::init<std::string>())
    .def ("insert_rack", &horst::horst_jack::insert_rack)
    .def ("remove_rack", &horst::horst_jack::insert_rack)
    .def ("number_of_racks", &horst::horst_jack::number_of_racks)
    .def ("insert_ladspa_plugin", &horst::horst_jack::insert_rack)
    .def ("insert_lv2_plugin", &horst::horst_jack::insert_rack)
    .def ("remove_plugin", &horst::horst_jack::insert_rack)
    .def ("set_plugin_parameter", &horst::horst_jack::insert_rack)
    .def ("number_of_plugins", &horst::horst_jack::number_of_plugins)
  ;
}

