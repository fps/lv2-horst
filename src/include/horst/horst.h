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

  struct lilv_world {
    LilvWorld *m;

    lilv_world () {
      m = lilv_world_new ();
      if (m == 0) throw std::runtime_error ("Failed to create lilv world");
      lilv_world_load_all (m);
    }

    ~lilv_world () {
      lilv_world_free (m);
    } 
  };

  typedef std::shared_ptr<lilv_world> lilv_world_ptr;

  struct lilv_plugins {
    const LilvPlugins *m;
    lilv_plugins (const lilv_world &world) :
      m (lilv_world_get_all_plugins (world.m)) {
    }
  };

  typedef std::shared_ptr<lilv_plugins> lilv_plugins_ptr;

  struct lilv_uri_node {
    LilvNode *m;
    lilv_uri_node (const lilv_world &world, const std::string &uri) {
      m = lilv_new_uri (world.m, uri.c_str());
      if (m == 0) throw std::runtime_error ("Failed to create lilv uri node. URI: " + uri);
    }

    ~lilv_uri_node () {
      lilv_node_free (m);
    }
  };

  typedef std::shared_ptr<lilv_uri_node> lilv_uri_node_ptr;

  struct lilv_plugin {
    const LilvPlugin *m;
    lilv_plugin (const lilv_plugins &plugins, const lilv_uri_node &node) :
      m (lilv_plugins_get_by_uri (plugins.m, node.m)) {
    }
  };

  typedef std::shared_ptr<lilv_plugin> lilv_plugin_ptr;

  struct port_properties {
    bool m_is_audio;
    bool m_is_control;
    bool m_is_input;
    bool m_is_output;
    float m_minimum_value;
    float m_default_value;
    float m_maximum_value;
    bool m_is_logarithmic;
    std::string m_name;
  };

  struct plugin_base {
    std::vector<port_properties> m_port_properties;

    virtual void set_control_port_value (int port_index, float value) = 0;
    virtual const std::string &get_name () const = 0;
    virtual void instantiate (double sample_rate) = 0;

    virtual ~plugin_base () {}
  };

  typedef std::shared_ptr<plugin_base> plugin_ptr;

  struct ladspa_plugin : public plugin_base {

  };

  struct lv2_plugin : public plugin_base {
    const lilv_uri_node m_lilv_plugin_uri;
    const lilv_plugin m_lilv_plugin;

    const std::string m_uri;
    std::string m_name;

    LilvInstance *m_lilv_instance;

    lv2_plugin (const lilv_world &world, const lilv_plugins &plugins, const std::string &uri) : 
      m_lilv_plugin_uri (world, uri),
      m_lilv_plugin (plugins, m_lilv_plugin_uri),
      m_uri (uri)
    {
      lilv_uri_node input (world, LILV_URI_INPUT_PORT);
      lilv_uri_node output (world, LILV_URI_OUTPUT_PORT);
      lilv_uri_node audio (world, LILV_URI_AUDIO_PORT);
      lilv_uri_node control (world, LILV_URI_CONTROL_PORT);

      lilv_uri_node doap_name (world, "http://usefulinc.com/ns/doap#name");
      LilvNode *name_node = lilv_world_get (world.m, m_lilv_plugin_uri.m, doap_name.m, NULL);
      if (name_node == 0) throw std::runtime_error ("Failed to get name of plugin. URI: " + m_uri);
      m_name = lilv_node_as_string (name_node);

      m_port_properties.resize (lilv_plugin_get_num_ports (m_lilv_plugin.m));  
      for (size_t index = 0; index < m_port_properties.size(); ++index) {
        const LilvPort *lilv_port = lilv_plugin_get_port_by_index (m_lilv_plugin.m, index);
        port_properties &p = m_port_properties[index];
        p.m_name = lilv_node_as_string (lilv_port_get_symbol (m_lilv_plugin.m, lilv_port));

        p.m_is_audio = lilv_port_is_a (m_lilv_plugin.m, lilv_port, audio.m);
        p.m_is_control = lilv_port_is_a (m_lilv_plugin.m, lilv_port, control.m);
        p.m_is_input = lilv_port_is_a (m_lilv_plugin.m, lilv_port, input.m);
        p.m_is_output = lilv_port_is_a (m_lilv_plugin.m, lilv_port, output.m);
      }
    }

    virtual const std::string &get_name () const { return m_name; }

    virtual void instantiate (double sample_rate) {
      m_lilv_instance = lilv_plugin_instantiate (m_lilv_plugin.m, sample_rate, 0);
      if (m_lilv_instance == 0) throw std::runtime_error ("Failed to instantiate plugin. URI: " + m_uri);
    }

    virtual void set_control_port_value (int port_index, float value) {

    }
  };

  extern "C" {
    int unit_jack_process_callback (jack_nframes_t nframes, void *arg);    
    int unit_jack_buffer_size_callback (jack_nframes_t buffer_size, void *arg);
    int unit_jack_sample_rate_callback (jack_nframes_t sample_rate, void *arg);
  }

  struct unit {
    lart::ringbuffer<std::function<void ()>> m_commands;
    jack_client_t *m_jack_client;
    std::vector<jack_port_t *> m_jack_ports;

    unit (const std::string &jack_client_name, const std::vector<port_properties> &port_properties) :
      m_commands (1024) {
      m_jack_client = jack_client_open (("horst-" + jack_client_name).c_str(), JackUseExactName, 0);
      if (m_jack_client == 0) throw std::runtime_error ("Failed to open jack client. Name: " + jack_client_name);
    }

    virtual ~unit () {
      jack_client_close (m_jack_client);
    }
  };

  typedef std::shared_ptr<unit> unit_ptr;

  struct plugin_unit : public unit {
    plugin_ptr m_plugin;
    plugin_unit (plugin_ptr plugin) :
      unit (plugin->get_name (), plugin->m_port_properties),
      m_plugin (plugin) {
      for (size_t index = 0; index < plugin->m_port_properties.size(); ++index) {
        port_properties &p = m_plugin->m_port_properties[index];
        if (p.m_is_audio) {
          if (p.m_is_input) {
            jack_port_t *port = jack_port_register (m_jack_client, p.m_name.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
            if (port == 0) throw std::runtime_error (std::string ("Failed to register port: ") + m_plugin->get_name () + ":" + p.m_name);
            m_jack_ports.push_back(port);
          } else {
            jack_port_t *port = jack_port_register (m_jack_client, p.m_name.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
            if (port == 0) throw std::runtime_error (std::string ("Failed to register port: ") + m_plugin->get_name () + ":" + p.m_name);
            m_jack_ports.push_back(port);
          }
        }
      }

      plugin->instantiate (jack_get_sample_rate (m_jack_client));
    }
  };

  struct horst_jack {
    lilv_world m_lilv_world;
    lilv_plugins m_lilv_plugins;
    std::list<unit_ptr> m_units;

    horst_jack () :
      m_lilv_plugins (m_lilv_world) {
      /*
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
      */
    }

    ~horst_jack () {
      /*
      assert (pthread_self () == m_self);
      jack_client_close (m_jack_client);
      */
    }

    /*
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
    */
  
    void insert_lv2_plugin (int plugin_index, const std::string &uri, const std::string &name) {
      auto it = m_units.begin ();
      for (int index = 0; index < plugin_index; ++index) ++it; 
      m_units.insert(it, unit_ptr (new plugin_unit (plugin_ptr (new lv2_plugin (m_lilv_world, m_lilv_plugins, uri)))));
    }
  
    void insert_ladspa_plugin (int plugin_index, std::string library_file_name, std::string plugin_label) {
  
    }
  
    void set_plugin_parameter (int plugin_index, int port_index, float value) {
  
    }

    void remove_plugin (int plugin_index) {
  
    }

    int number_of_plugins () {
      return 0;
    }
  };

  extern "C" {
    /*
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
    */
  }

}

namespace bp = boost::python;
BOOST_PYTHON_MODULE(horst)
{
  bp::class_<horst::lilv_world>("lilv_world");
  bp::class_<horst::horst_jack>("horst")
    .def ("insert_ladspa_plugin", &horst::horst_jack::insert_ladspa_plugin)
    .def ("insert_lv2_plugin", &horst::horst_jack::insert_lv2_plugin)
    .def ("remove_plugin", &horst::horst_jack::remove_plugin)
    .def ("set_plugin_parameter", &horst::horst_jack::set_plugin_parameter)
    .def ("number_of_plugins", &horst::horst_jack::number_of_plugins)
  ;
}

