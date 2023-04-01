#pragma once

#include <horst/lv2.h>

namespace horst {

  struct port_properties {
    bool m_is_audio;
    bool m_is_control;
    bool m_is_cv;
    bool m_is_input;
    bool m_is_output;
    bool m_is_side_chain;
    float m_minimum_value;
    float m_default_value;
    float m_maximum_value;
    bool m_is_logarithmic;
    std::string m_name;
  };

  struct plugin_base {
    std::vector<port_properties> m_port_properties;

    virtual const std::string &get_name () const = 0;

    virtual void instantiate (double sample_rate, size_t buffer_size) = 0;

    virtual void connect_port (size_t index, float *data) = 0;

    virtual void run (size_t nframes) = 0;

    virtual size_t find_port (const std::string &name) {
      for (size_t index = 0; index < m_port_properties.size(); ++index) {
        if (m_port_properties[index].m_name == name) { return index; }
      }

      throw std::runtime_error ("horst: lv2_plugin: Port not found: " + name);
    }

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

  extern "C" {
    LV2_URID urid_map (LV2_URID_Map_Handle handle, const char *uri);
  }

  struct lv2_plugin : public plugin_base {
    lilv_uri_node_ptr m_lilv_plugin_uri;
    lilv_plugin_ptr m_lilv_plugin;

    lilv_plugin_instance_ptr m_plugin_instance;

    const std::string m_uri;
    std::string m_name;

    uint32_t m_min_block_length;
    uint32_t m_max_block_length;

    std::vector<LV2_Options_Option> m_options;

    LV2_URID_Map m_urid_map;
    LV2_Feature m_urid_map_feature;
    LV2_Feature m_is_live_feature;
    LV2_Feature m_bounded_block_length_feature;
    LV2_Feature m_options_feature;
    std::vector<LV2_Feature*> m_supported_features;

    lv2_plugin (lilv_world_ptr world, lilv_plugins_ptr plugins, const std::string &uri) :
      m_lilv_plugin_uri (new lilv_uri_node (world, uri)),
      m_lilv_plugin (new lilv_plugin (plugins, m_lilv_plugin_uri)),
      m_uri (uri),
      m_urid_map { .handle = (LV2_URID_Map_Handle)this, .map = horst::urid_map },
      m_urid_map_feature { .URI = LV2_URID__map, .data = &m_urid_map },
      m_is_live_feature { .URI = LV2_CORE__isLive, .data = 0 },
      m_bounded_block_length_feature { .URI = LV2_BUF_SIZE__boundedBlockLength, .data = 0 },
      m_options_feature { .URI = LV2_OPTIONS__options, .data = &m_options[0] }
    {
      m_options.push_back (LV2_Options_Option { .context = LV2_OPTIONS_INSTANCE, .subject = 0, .key = urid_map (LV2_BUF_SIZE__minBlockLength), .size = sizeof (int32_t), .type = urid_map (LV2_ATOM__Int), .value = &m_min_block_length });
      m_options.push_back (LV2_Options_Option { .context = LV2_OPTIONS_INSTANCE, .subject = 0, .key = urid_map (LV2_BUF_SIZE__maxBlockLength), .size = sizeof (int32_t), .type = urid_map (LV2_ATOM__Int), .value = &m_max_block_length });
      m_options.push_back (LV2_Options_Option { .context = LV2_OPTIONS_INSTANCE, .subject = 0, .key = 0, .size = 0, .type = 0, .value = 0 });
      m_options_feature.data = &m_options[0];

      m_supported_features.push_back (&m_urid_map_feature);
      m_supported_features.push_back (&m_is_live_feature);
      m_supported_features.push_back (&m_options_feature);
      m_supported_features.push_back (&m_bounded_block_length_feature);
      m_supported_features.push_back (0);

      LilvNodes *features = lilv_plugin_get_required_features (m_lilv_plugin->m);
      if (features != 0) {
        std::stringstream s;
        LILV_FOREACH(nodes, i, features) {
          const LilvNode *node = lilv_nodes_get (features, i);
          std::string feature_uri =  lilv_node_as_uri (node);
          bool supported = false;
          for (size_t feature_index = 0; feature_index < m_supported_features.size () - 1; ++feature_index) {
            if (m_supported_features[feature_index]->URI == feature_uri) {
              supported = true;
              break;
            }
          }
          if (!supported) {
            lilv_nodes_free (features);
            throw std::runtime_error ("horst: lv2_plugin: Unsupported feature: " + feature_uri);
          }
        }
        lilv_nodes_free (features);
      }

      lilv_uri_node required_options_uri (world, LV2_OPTIONS__requiredOption);
      LilvNodes *required_options = lilv_plugin_get_value (m_lilv_plugin->m, required_options_uri.m);
      LILV_FOREACH (nodes, i, required_options) {
        const LilvNode *node = lilv_nodes_get (required_options, i);
        std::cout << "Required options: " << lilv_node_as_string (node) << "\n";
      }
      lilv_nodes_free (required_options);

      lilv_uri_node input (world, LILV_URI_INPUT_PORT);
      lilv_uri_node output (world, LILV_URI_OUTPUT_PORT);
      lilv_uri_node audio (world, LILV_URI_AUDIO_PORT);
      lilv_uri_node control (world, LILV_URI_CONTROL_PORT);
      lilv_uri_node cv (world, LILV_URI_CV_PORT);
      lilv_uri_node side_chain (world, "https://lv2plug.in/ns/lv2core#isSideChain");

      m_port_properties.resize (lilv_plugin_get_num_ports (m_lilv_plugin->m));
      for (size_t index = 0; index < m_port_properties.size(); ++index) {
        const LilvPort *lilv_port = lilv_plugin_get_port_by_index (m_lilv_plugin->m, index);
        port_properties &p = m_port_properties[index];
        p.m_name = lilv_node_as_string (lilv_port_get_symbol (m_lilv_plugin->m, lilv_port));

        p.m_is_audio = lilv_port_is_a (m_lilv_plugin->m, lilv_port, audio.m);
        p.m_is_control = lilv_port_is_a (m_lilv_plugin->m, lilv_port, control.m);
        p.m_is_cv = lilv_port_is_a (m_lilv_plugin->m, lilv_port, cv.m);
        p.m_is_input = lilv_port_is_a (m_lilv_plugin->m, lilv_port, input.m);
        p.m_is_output = lilv_port_is_a (m_lilv_plugin->m, lilv_port, output.m);
        p.m_is_side_chain = lilv_port_has_property (m_lilv_plugin->m, lilv_port, side_chain.m);

        if (p.m_is_input && p.m_is_control) {
          LilvNode *def;
          LilvNode *min;
          LilvNode *max;

          lilv_port_get_range (m_lilv_plugin->m, lilv_port, &def, &min, &max);

          p.m_minimum_value = lilv_node_as_float (min);
          p.m_default_value = lilv_node_as_float (def);
          p.m_maximum_value = lilv_node_as_float (max);

          lilv_node_free (def);
          lilv_node_free (min);
          lilv_node_free (max);
        }
      }

      lilv_uri_node doap_name (world, "http://usefulinc.com/ns/doap#name");
      LilvNode *name_node = lilv_world_get (world->m, m_lilv_plugin_uri->m, doap_name.m, 0);
      if (name_node == 0) throw std::runtime_error ("horst: lv2_plugin: Failed to get name of plugin. URI: " + m_uri);
      m_name = lilv_node_as_string (name_node);
      lilv_node_free (name_node);

    }

    virtual const std::string &get_name () const { return m_name; }

    virtual void instantiate (double sample_rate, size_t buffer_size) {
      // std::cout << "instantiate () " << sample_rate << " " << buffer_size << "\n";
      m_min_block_length = 0;
      m_max_block_length = (int32_t)buffer_size;

      m_plugin_instance = lilv_plugin_instance_ptr (new lilv_plugin_instance (m_lilv_plugin, sample_rate, &m_supported_features[0]));
    }

    virtual void connect_port (size_t port_index, float *data) override {
      lilv_instance_connect_port (m_plugin_instance->m, port_index, data);
    }

    virtual void run (size_t nframes) override {
      lilv_instance_run (m_plugin_instance->m, nframes);
    }

    std::vector<std::string> m_mapped_uris;

    LV2_URID urid_map (const char *uri) {
      auto it = std::find (m_mapped_uris.begin (), m_mapped_uris.end (), uri);
      LV2_URID urid = it - m_mapped_uris.begin ();
      if (it == m_mapped_uris.end ()) {
        m_mapped_uris.push_back (uri);
      }

      // std::cout << "URI: " << uri << " -> " << urid << "\n";
      return urid + 1;
    }
  };

  extern "C" {
    LV2_URID urid_map (LV2_URID_Map_Handle handle, const char *uri) {
      return ((lv2_plugin*)handle)->urid_map(uri);
    }
  }


  typedef std::shared_ptr<lv2_plugin> lv2_plugin_ptr;
}
