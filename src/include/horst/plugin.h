#pragma once

#include <horst/lv2.h>

namespace horst {

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

    virtual const std::string &get_name () const = 0;
    virtual void instantiate (double sample_rate) = 0;
    virtual void connect_port (size_t index, float *data) = 0;
    virtual void run (size_t nframes) = 0;

    virtual ~plugin_base () {}
  };

  typedef std::shared_ptr<plugin_base> plugin_ptr;

  struct plugin_base_wrapper {
    plugin_ptr m;
    plugin_base_wrapper (plugin_ptr plugin = plugin_ptr()) :
      m (plugin) {
    }
  };

  struct ladspa_plugin : public plugin_base {

  };

  struct lv2_plugin : public plugin_base {
    const lilv_uri_node_ptr m_lilv_plugin_uri;
    const lilv_plugin_ptr m_lilv_plugin;

    lilv_plugin_instance_ptr m_plugin_instance;

    const std::string m_uri;
    std::string m_name;

    lv2_plugin (lilv_world_ptr world, lilv_plugins_ptr plugins, const std::string &uri) :
      m_lilv_plugin_uri (new lilv_uri_node (world, uri)),
      m_lilv_plugin (new lilv_plugin (plugins, m_lilv_plugin_uri)),
      m_uri (uri)
    {
      lilv_uri_node input (world, LILV_URI_INPUT_PORT);
      lilv_uri_node output (world, LILV_URI_OUTPUT_PORT);
      lilv_uri_node audio (world, LILV_URI_AUDIO_PORT);
      lilv_uri_node control (world, LILV_URI_CONTROL_PORT);

      m_port_properties.resize (lilv_plugin_get_num_ports (m_lilv_plugin->m));
      for (size_t index = 0; index < m_port_properties.size(); ++index) {
        const LilvPort *lilv_port = lilv_plugin_get_port_by_index (m_lilv_plugin->m, index);
        port_properties &p = m_port_properties[index];
        p.m_name = lilv_node_as_string (lilv_port_get_symbol (m_lilv_plugin->m, lilv_port));

        p.m_is_audio = lilv_port_is_a (m_lilv_plugin->m, lilv_port, audio.m);
        p.m_is_control = lilv_port_is_a (m_lilv_plugin->m, lilv_port, control.m);
        p.m_is_input = lilv_port_is_a (m_lilv_plugin->m, lilv_port, input.m);
        p.m_is_output = lilv_port_is_a (m_lilv_plugin->m, lilv_port, output.m);

        if (p.m_is_input && p.m_is_control) {
          LilvNode *def;
          LilvNode *min;
          LilvNode *max;

          lilv_port_get_range (m_lilv_plugin->m, lilv_port, &def, &min, &max);

          p.m_minimum_value = lilv_node_as_float (min);
          p.m_default_value = lilv_node_as_float (def);
          p.m_maximum_value = lilv_node_as_float (max);
        }
      }

      lilv_uri_node doap_name (world, "http://usefulinc.com/ns/doap#name");
      LilvNode *name_node = lilv_world_get (world->m, m_lilv_plugin_uri->m, doap_name.m, 0);
      if (name_node == 0) throw std::runtime_error ("horst: lv2_plugin: Failed to get name of plugin. URI: " + m_uri);
      m_name = lilv_node_as_string (name_node);
      lilv_node_free (name_node);

      LilvNodes *features = lilv_plugin_get_required_features (m_lilv_plugin->m);
      if (features != 0) {
        throw std::runtime_error (std::string ("horst: lv2_plugin: Unsupported feature: "));
      }
    }

    virtual const std::string &get_name () const { return m_name; }

    virtual void instantiate (double sample_rate) {
      m_plugin_instance = lilv_plugin_instance_ptr (new lilv_plugin_instance (m_lilv_plugin, sample_rate));
    }

    virtual void connect_port (size_t port_index, float *data) override {
      lilv_instance_connect_port (m_plugin_instance->m, port_index, data);
    }

    virtual void run (size_t nframes) override {
      lilv_instance_run (m_plugin_instance->m, nframes);
    }
  };

  typedef std::shared_ptr<lv2_plugin> lv2_plugin_ptr;
}
