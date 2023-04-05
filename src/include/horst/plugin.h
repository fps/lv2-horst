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

    virtual ~plugin_base () { DBG_ENTER_EXIT }
  };

  typedef std::shared_ptr<plugin_base> plugin_ptr;

  struct plugin_base_wrapper {
    plugin_ptr m;
    plugin_base_wrapper (plugin_ptr plugin = plugin_ptr()) :
      m (plugin) {
      DBG_ENTER_EXIT
    }
  };

  struct ladspa_plugin : public plugin_base {

  };

  extern "C" {
    LV2_URID urid_map (LV2_URID_Map_Handle handle, const char *uri);
    LV2_Worker_Status schedule_work (LV2_Worker_Schedule_Handle handle, uint32_t size, const void *data);
    void *worker_thread (void *);
    LV2_Worker_Status worker_respond (LV2_Worker_Respond_Handle handle, uint32_t size, const void *data);
  }

#define HORST_WORK_ITEMS 32
#define HORST_WORK_ITEM_MAX_SIZE (1024 * 1024)

  struct lv2_plugin : public plugin_base {
    lilv_world_ptr m_lilv_world;
    lilv_plugins_ptr m_lilv_plugins;
    lilv_uri_node_ptr m_lilv_plugin_uri;
    lilv_plugin_ptr m_lilv_plugin;

    lilv_plugin_instance_ptr m_plugin_instance;

    const std::string m_uri;
    std::string m_name;

    uint32_t m_min_block_length;
    uint32_t m_max_block_length;
    uint32_t m_nominal_block_length;

    std::vector<LV2_Options_Option> m_options;

    LV2_Worker_Schedule m_worker_schedule;
    std::atomic<LV2_Worker_Interface*> m_worker_interface;

    std::array<std::pair<unsigned, std::array<uint8_t, HORST_WORK_ITEM_MAX_SIZE>>, HORST_WORK_ITEMS> m_work_items;
    std::atomic<size_t> m_work_items_head;
    std::atomic<size_t> m_work_items_tail;

    std::array<std::pair<unsigned, std::array<uint8_t, HORST_WORK_ITEM_MAX_SIZE>>, HORST_WORK_ITEMS> m_work_responses;
    std::atomic<size_t> m_work_responses_head;
    std::atomic<size_t> m_work_responses_tail;

    std::atomic<bool> m_worker_quit;
    pthread_t m_worker_thread;

    std::vector<std::string> m_mapped_uris;

    LV2_URID_Map m_urid_map;
    LV2_Feature m_urid_map_feature;
    LV2_Feature m_is_live_feature;
    LV2_Feature m_bounded_block_length_feature;
    LV2_Feature m_nominal_block_length_feature;
    LV2_Feature m_options_feature;
    LV2_Feature m_worker_feature;
    std::vector<LV2_Feature*> m_supported_features;

    lv2_plugin (lilv_world_ptr world, lilv_plugins_ptr plugins, const std::string &uri) :
      m_lilv_world (world),
      m_lilv_plugins (plugins),
      m_lilv_plugin_uri (new lilv_uri_node (world, uri)),
      m_lilv_plugin (new lilv_plugin (plugins, m_lilv_plugin_uri)),
      m_uri (uri),

      m_worker_schedule { (LV2_Worker_Schedule_Handle)this, horst::schedule_work },
      m_worker_interface (0),

      m_work_items_head (0),
      m_work_items_tail (0),

      m_work_responses_head (0),
      m_work_responses_tail (0),

      m_worker_quit (false),

      m_urid_map { .handle = (LV2_URID_Map_Handle)this, .map = horst::urid_map },

      m_urid_map_feature { .URI = LV2_URID__map, .data = &m_urid_map },
      m_is_live_feature { .URI = LV2_CORE__isLive, .data = 0 },
      m_bounded_block_length_feature { .URI = LV2_BUF_SIZE__boundedBlockLength, .data = 0 },
      m_nominal_block_length_feature { .URI = LV2_BUF_SIZE__nominalBlockLength, .data = 0 },
      m_options_feature { .URI = LV2_OPTIONS__options, .data = &m_options[0] },
      m_worker_feature { .URI = LV2_WORKER__schedule, .data = &m_worker_schedule }
    {
      DBG_ENTER
      for (size_t index = 0; index < HORST_WORK_ITEMS; ++index) {
        m_work_items[index].first = 0;
        m_work_items[index].second = std::array<uint8_t, HORST_WORK_ITEM_MAX_SIZE>{};
        m_work_responses[index].first = 0;
        m_work_responses[index].second = std::array<uint8_t, HORST_WORK_ITEM_MAX_SIZE>{};
      }

      m_options.push_back (LV2_Options_Option { .context = LV2_OPTIONS_INSTANCE, .subject = 0, .key = urid_map (LV2_BUF_SIZE__minBlockLength), .size = sizeof (int32_t), .type = urid_map (LV2_ATOM__Int), .value = &m_min_block_length });
      m_options.push_back (LV2_Options_Option { .context = LV2_OPTIONS_INSTANCE, .subject = 0, .key = urid_map (LV2_BUF_SIZE__maxBlockLength), .size = sizeof (int32_t), .type = urid_map (LV2_ATOM__Int), .value = &m_max_block_length });
      m_options.push_back (LV2_Options_Option { .context = LV2_OPTIONS_INSTANCE, .subject = 0, .key = urid_map (LV2_BUF_SIZE__nominalBlockLength), .size = sizeof (int32_t), .type = urid_map (LV2_ATOM__Int), .value = &m_max_block_length });
      m_options.push_back (LV2_Options_Option { .context = LV2_OPTIONS_INSTANCE, .subject = 0, .key = 0, .size = 0, .type = 0, .value = 0 });
      m_options_feature.data = &m_options[0];

      m_supported_features.push_back (&m_urid_map_feature);
      m_supported_features.push_back (&m_is_live_feature);
      m_supported_features.push_back (&m_options_feature);
      m_supported_features.push_back (&m_bounded_block_length_feature);
      m_supported_features.push_back (&m_nominal_block_length_feature);
      m_supported_features.push_back (&m_worker_feature);
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

      #ifdef HORST_DEBUG
      lilv_uri_node required_options_uri (world, LV2_OPTIONS__requiredOption);
      LilvNodes *required_options = lilv_plugin_get_value (m_lilv_plugin->m, required_options_uri.m);
      LILV_FOREACH (nodes, i, required_options) {
        const LilvNode *node = lilv_nodes_get (required_options, i);
        DBG("Required options: " << lilv_node_as_string (node))
      }
      lilv_nodes_free (required_options);
      #endif

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

      int ret = pthread_create (&m_worker_thread, 0, horst::worker_thread, this);
      if (ret != 0) throw std::runtime_error ("horst: lv2_plugin: Failed to create worker thread");
      DBG_EXIT
    }

    virtual const std::string &get_name () const { return m_name; }

    virtual void instantiate (double sample_rate, size_t buffer_size) {
      DBG(sample_rate << " " << buffer_size)
      m_min_block_length = 0;
      m_max_block_length = (int32_t)buffer_size;
      m_nominal_block_length = (int32_t)buffer_size;

      m_plugin_instance = lilv_plugin_instance_ptr (new lilv_plugin_instance (m_lilv_plugin, sample_rate, &m_supported_features[0]));

      m_worker_interface = (LV2_Worker_Interface*)lilv_instance_get_extension_data (m_plugin_instance->m, LV2_WORKER__interface); 

      DBG("worker interface: " << m_worker_interface);

      if (m_worker_interface)
      {
        DBG((void*)(m_worker_interface.load ()->work) << " " << (void*)(m_worker_interface.load ()->work_response) << " " << (void*)(m_worker_interface.load ()->end_run));
      }
      // usleep (500000);
    }

    virtual void connect_port (size_t port_index, float *data) override {
      lilv_instance_connect_port (m_plugin_instance->m, port_index, data);
    }

    virtual void run (size_t nframes) override {
      LV2_Worker_Interface *interface = m_worker_interface;
      if (interface) {
        if (number_of_items (m_work_responses_head, m_work_responses_tail, m_work_responses.size ())) {
          DBG_JACK("has responses")
          auto &item = m_work_responses[m_work_responses_tail];
          if (interface->work_response) {
            DBG_JACK("item: " << item.first << " " << (int)item.second[0])
            interface->work_response (m_plugin_instance->m_lv2_handle, item.first, &item.second[0]);
          }
          advance (m_work_responses_tail, m_work_responses.size ());
        }
      }

      lilv_instance_run (m_plugin_instance->m, nframes);

      if (interface && interface->end_run) {
        interface->end_run (m_plugin_instance->m_lv2_handle);
      }
    }

    LV2_URID urid_map (const char *uri) {
      auto it = std::find (m_mapped_uris.begin (), m_mapped_uris.end (), uri);
      LV2_URID urid = it - m_mapped_uris.begin ();
      if (it == m_mapped_uris.end ()) {
        m_mapped_uris.push_back (uri);
      }

      urid += 1;

      // std::cout << "URI: " << uri << " -> " << urid << "\n";
      DBG("URI: " << uri << " -> URID: " << urid)
      return urid;
    }

    int number_of_items (const int &head, const int &tail, const size_t &size) {
      const int items = (head >= tail) ? (head - tail) : (head + size - tail);
      // DBG("#items: " << items) 
      return items;
    }

    void advance (std::atomic<size_t> &item, const size_t &size) {
      DBG("advance: " << item);
      int prev = item;
      item = (prev + 1) % size;
      DBG("advanced: " << item)
    }

    LV2_Worker_Status schedule_work (uint32_t size, const void *data) {
      DBG_ENTER
      if (m_worker_quit == true) {
        DBG("quit!")
        return LV2_WORKER_ERR_UNKNOWN;
      }

      if (m_worker_interface) {
        DBG("schedule_work")
        if (number_of_items (m_work_items_head, m_work_items_tail, m_work_items.size ()) < (int)m_work_items.size() - 1) {
          if (size > HORST_WORK_ITEM_MAX_SIZE) return LV2_WORKER_ERR_NO_SPACE;

          auto &item = m_work_items[m_work_items_head];
          item.first = size;
          memcpy (&item.second[0], data, size);
          DBG_JACK("item: " << item.first << " " << (int)item.second[0])
          advance (m_work_items_head, m_work_items.size ());
          DBG_EXIT
          return LV2_WORKER_SUCCESS; // m_worker_interface->work (m_plugin_instance->m, horst::worker_respond, this, size, data);
        }
        DBG_EXIT
        return LV2_WORKER_ERR_NO_SPACE;
      }
      DBG_EXIT
      return LV2_WORKER_ERR_UNKNOWN;
    }

    LV2_Worker_Status worker_respond (uint32_t size, const void *data) {
      DBG_ENTER
      if (m_worker_interface) {
        DBG("respond.");
        if (size > HORST_WORK_ITEM_MAX_SIZE) return LV2_WORKER_ERR_NO_SPACE;
        auto &item = m_work_responses[m_work_responses_head];
        memcpy (&item.second[0], data, size);
        DBG("item: " << item.first << " " << (int)item.second[0])
        advance (m_work_responses_head, m_work_responses.size ());
      }
      DBG_EXIT
      return LV2_WORKER_SUCCESS;
    }

    void *worker_thread () {
      DBG_ENTER
      while (!m_worker_quit) {
        LV2_Worker_Interface *interface = m_worker_interface;
        // std::cout << "m: " << m_worker_interface << " interface: " << interface << " number: " << number_of_items (m_work_items_head, m_work_items_tail, m_work_items.size ()) << "\n";
        while (m_worker_interface && number_of_items (m_work_items_head, m_work_items_tail, m_work_items.size ())) {
          DBG("getting to work: " << (void*)(interface->work) << " " << (void*)(interface->work_response) << " " << (void*)(interface->end_run))
          auto &item = m_work_items[m_work_items_tail];
          DBG("item: " << item.first << " " << (int)item.second[0])
          if (m_worker_interface.load ()->work) {
            DBG(m_plugin_instance->m)
            #ifdef DEBUG_HORST
            LV2_Worker_Status res = interface->work (m_plugin_instance->m_lv2_handle, &horst::worker_respond, (LV2_Worker_Respond_Handle)this, item.first, &item.second[0]);
            DBG("worker_thread: res: " << res)
            #else
            interface->work (m_plugin_instance->m_lv2_handle, &horst::worker_respond, (LV2_Worker_Respond_Handle)this, item.first, &item.second[0]);
            #endif
          }
          advance (m_work_items_tail, m_work_items.size ());
        }
        // std::cout << "horst: lv2_plugin: worker_thread ()\n";
        usleep (1000);
      }
      DBG_EXIT
      return 0;
    }

    ~lv2_plugin () {
      DBG_ENTER
      m_worker_quit = true;
      pthread_join (m_worker_thread, 0);
      m_plugin_instance = lilv_plugin_instance_ptr();
      DBG_EXIT
    }
  };

  extern "C" {
    LV2_URID urid_map (LV2_URID_Map_Handle handle, const char *uri) {
      return ((lv2_plugin*)handle)->urid_map(uri);
    }

    LV2_Worker_Status schedule_work (LV2_Worker_Schedule_Handle handle, uint32_t size, const void *data) {
      return  ((lv2_plugin*)handle)->schedule_work (size, data);
    }

    LV2_Worker_Status worker_respond (LV2_Worker_Respond_Handle handle, uint32_t size, const void *data) {
      return ((lv2_plugin*)handle)->worker_respond (size, data);
    }

    void *worker_thread (void *arg) {
      return ((lv2_plugin*)arg)->worker_thread ();
    }
  }


  typedef std::shared_ptr<lv2_plugin> lv2_plugin_ptr;
}
