#pragma once

#include <vector>
#include <list>
#include <string>
#include <functional>
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
    const std::string m_uri;
    LilvNode *m;

    lilv_uri_node (const lilv_world &world, const std::string &uri) :
      m_uri (uri),
      m (lilv_new_uri (world.m, uri.c_str ())) {
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

  struct lilv_plugin_instance {
    LilvInstance *m;
    lilv_plugin_instance (const lilv_plugin &plugin, double sample_rate) :
      m (lilv_plugin_instantiate (plugin.m, sample_rate, 0))
    {
      if (m == 0) throw std::runtime_error ("Failed to instantiate plugin");
      lilv_instance_activate (m);
    }

    ~lilv_plugin_instance () {
      lilv_instance_deactivate (m);
      lilv_instance_free (m);
    }
  };

  typedef std::shared_ptr<lilv_plugin_instance> lilv_plugin_instance_ptr;

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
    virtual void connect_port (size_t index, float *data) = 0;
    virtual void run (size_t nframes) = 0;

    virtual ~plugin_base () {}
  };

  typedef std::shared_ptr<plugin_base> plugin_ptr;

  struct ladspa_plugin : public plugin_base {

  };

  struct lv2_plugin : public plugin_base {
    const lilv_uri_node m_lilv_plugin_uri;
    const lilv_plugin m_lilv_plugin;

    lilv_plugin_instance_ptr m_plugin_instance;

    const std::string m_uri;
    std::string m_name;

    lv2_plugin (const lilv_world &world, const lilv_plugins &plugins, const std::string &uri) : 
      m_lilv_plugin_uri (world, uri),
      m_lilv_plugin (plugins, m_lilv_plugin_uri),
      m_uri (uri)
    {
      lilv_uri_node input (world, LILV_URI_INPUT_PORT);
      lilv_uri_node output (world, LILV_URI_OUTPUT_PORT);
      lilv_uri_node audio (world, LILV_URI_AUDIO_PORT);
      lilv_uri_node control (world, LILV_URI_CONTROL_PORT);

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

      lilv_uri_node doap_name (world, "http://usefulinc.com/ns/doap#name");
      LilvNode *name_node = lilv_world_get (world.m, m_lilv_plugin_uri.m, doap_name.m, 0);
      if (name_node == 0) throw std::runtime_error ("Failed to get name of plugin. URI: " + m_uri);
      m_name = lilv_node_as_string (name_node);
      lilv_node_free (name_node);

      LilvNodes *features = lilv_plugin_get_required_features (m_lilv_plugin.m);
      if (features != 0) throw std::runtime_error ("Unsupported feature");
    }

    virtual const std::string &get_name () const { return m_name; }

    virtual void instantiate (double sample_rate) {
      m_plugin_instance = lilv_plugin_instance_ptr (new lilv_plugin_instance (m_lilv_plugin, sample_rate));
    }

    virtual void set_control_port_value (int port_index, float value) {

    }

    virtual void connect_port (size_t port_index, float *data) override {
      lilv_instance_connect_port (m_plugin_instance->m, port_index, data);
    }

    virtual void run (size_t nframes) override {
      lilv_instance_run (m_plugin_instance->m, nframes);
    }

    virtual ~lv2_plugin () {

    }
  };

  typedef std::shared_ptr<lv2_plugin> lv2_plugin_ptr;

  extern "C" {
    int unit_jack_process_callback (jack_nframes_t nframes, void *arg);    
    int unit_jack_buffer_size_callback (jack_nframes_t buffer_size, void *arg);
    int unit_jack_sample_rate_callback (jack_nframes_t sample_rate, void *arg);
  }

  struct unit {
    virtual ~unit () {

    }

    virtual int process_callback (jack_nframes_t nframes) = 0;
    virtual int buffer_size_callback (jack_nframes_t buffer_size) = 0;
    virtual int sample_rate_callback (jack_nframes_t sample_rate) = 0;
  };

  extern "C" {
    int unit_jack_process_callback (jack_nframes_t nframes, void *arg) {
      return ((unit *)arg)->process_callback (nframes);
    }

    int unit_jack_buffer_size_callback (jack_nframes_t buffer_size, void *arg) {
      return ((unit *)arg)->buffer_size_callback (buffer_size);
    }

    int unit_jack_sample_rate_callback (jack_nframes_t sample_rate, void *arg) {
      return ((unit *)arg)->sample_rate_callback (sample_rate);
    }
  }

  typedef std::shared_ptr<unit> unit_ptr;

  struct plugin_unit : public unit {
    jack_client_t *m_jack_client;
    std::vector<jack_port_t *> m_jack_ports;

    std::vector<std::vector<float>> m_port_buffers;
    plugin_ptr m_plugin;

    plugin_unit (plugin_ptr plugin, const std::string &jack_client_name = "") :
      m_port_buffers (plugin->m_port_properties.size (), std::vector<float> (32, 0.0f)),
      m_plugin (plugin) {
      std::string client_name = jack_client_name;
      if (jack_client_name == "") client_name = "horst:" + m_plugin->get_name ();
      m_jack_client = jack_client_open (client_name.c_str(), JackUseExactName, 0);
      if (m_jack_client == 0) throw std::runtime_error ("Failed to open jack client. Name: " + jack_client_name);

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

      int ret;

      ret = jack_set_sample_rate_callback (m_jack_client, unit_jack_sample_rate_callback, (void *)this);
      if (ret != 0) {
        throw std::runtime_error ("Failed to set sample rate callback");
      }

      ret = jack_set_process_callback (m_jack_client, unit_jack_process_callback, (void *)this);
      if (ret != 0) {
        throw std::runtime_error ("Failed to set buffer size callback");
      }

      ret = jack_set_buffer_size_callback (m_jack_client, unit_jack_buffer_size_callback, (void *)this);
      if (ret != 0) {
        throw std::runtime_error ("Failed to set buffer size callback");
      }

      ret = jack_activate (m_jack_client);
      if (ret != 0) {
        throw std::runtime_error ("Failed to activate client");
      }
    }

    virtual ~plugin_unit () {
      jack_deactivate (m_jack_client);
      jack_client_close (m_jack_client);
    }

    virtual int process_callback (jack_nframes_t nframes) override {
      m_plugin->run (nframes);
      return 0;
    }

    void connect_ports () {
      for (size_t index = 0; index < m_port_buffers.size (); ++index) {
        port_properties &p = m_plugin->m_port_properties[index];
        if (p.m_is_audio || p.m_is_control) {
          m_plugin->connect_port (index, &m_port_buffers[index][0]);
        }
      }
    }

    virtual int buffer_size_callback (jack_nframes_t buffer_size) override {
      // std::cout << "buffer size callback. buffer size: " << buffer_size << "\n";
      for (size_t index = 0; index < m_port_buffers.size (); ++index) {
        m_port_buffers[index].resize (buffer_size, 0.0f);
      }
      connect_ports ();
      return 0;
    }

    virtual int sample_rate_callback (jack_nframes_t sample_rate) override {
      // std::cout << "sample rate callback. sample rate: " << sample_rate << "\n";
      m_plugin->instantiate ((double)sample_rate);
      connect_ports ();
      return 0;
    }
  };

  struct horst_jack {
    lilv_world m_lilv_world;
    lilv_plugins m_lilv_plugins;
    std::list<unit_ptr> m_units;

    horst_jack () :
      m_lilv_plugins (m_lilv_world) {
    }

    ~horst_jack () {

    }
  
    void insert_lv2_plugin (int plugin_index, const std::string &uri, const std::string &jack_client_name = "") {
      auto it = m_units.begin ();
      for (int index = 0; index < plugin_index; ++index) ++it; 
      m_units.insert(it, unit_ptr (new plugin_unit (plugin_ptr (new lv2_plugin (m_lilv_world, m_lilv_plugins, uri)), jack_client_name)));
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
}

