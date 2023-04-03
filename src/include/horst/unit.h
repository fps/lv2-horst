#pragma once

#include <horst/plugin.h>
#include <horst/jack.h>

namespace horst {
  extern "C" {
    int unit_jack_process_callback (jack_nframes_t nframes, void *arg);
    int unit_jack_buffer_size_callback (jack_nframes_t buffer_size, void *arg);
    int unit_jack_sample_rate_callback (jack_nframes_t sample_rate, void *arg);
  }

  const int cc_mask = 128 + 32 + 16;

  struct midi_binding {
    bool m_enabled;
    int m_channel;
    int m_cc;
    float m_factor;
    float m_offset;

    midi_binding (bool enabled = false, int channel = 0, int cc = 0, float factor = 1.0f, float offset = 0.0f) :
      m_enabled (enabled),
      m_channel (channel),
      m_cc (cc),
      m_factor (factor),
      m_offset (offset)
    {
      DBG_EXIT
    }
  };

  struct unit {
    std::atomic<bool> m_atomic_enabled;

    unit () :
      m_atomic_enabled (true)
    {
      DBG_EXIT
    }

    virtual ~unit () {
      DBG_EXIT
    }

    virtual std::string get_jack_client_name () {
      throw std::runtime_error ("horst: unit: not implemented yet");
    }

    virtual void set_control_port_value (size_t index, float value) {
      throw std::runtime_error ("horst: unit: not implemented yet");
    }

    virtual float get_control_port_value (size_t index) {
      throw std::runtime_error ("horst: unit: not implemented yet");
    }

    virtual void set_midi_binding (size_t index, const midi_binding &binding) {
      throw std::runtime_error ("horst: unit: not implemented yet");
    }

    virtual midi_binding get_midi_binding (size_t index) {
      throw std::runtime_error ("horst: unit: not implemented yet");
    }

    virtual int get_number_of_ports () {
      throw std::runtime_error ("horst: unit: not implemented yet");
    }

    virtual port_properties get_port_properties (int index) {
      throw std::runtime_error ("horst: unit: not implemented yet");
    }

    virtual void set_enabled (bool enabled) {
      m_atomic_enabled = enabled;
    }
  };

  typedef std::shared_ptr<unit> unit_ptr;

  struct unit_wrapper {
    unit_ptr m_unit;
    unit_wrapper (unit_ptr unit) :
      m_unit (unit)
    {
      DBG_EXIT
    }
  };

  struct jack_unit : public unit
  {
    bool m_expose_control_ports;

    virtual int process_callback (jack_nframes_t nframes) = 0;
    virtual int buffer_size_callback (jack_nframes_t buffer_size) = 0;
    virtual int sample_rate_callback (jack_nframes_t sample_rate) = 0;

    jack_unit (bool expose_control_ports) :
      m_expose_control_ports (expose_control_ports)
    {
      DBG_EXIT
    }
  };

  extern "C" {
    int unit_jack_process_callback (jack_nframes_t nframes, void *arg) {
      return ((jack_unit *)arg)->process_callback (nframes);
    }

    int unit_jack_buffer_size_callback (jack_nframes_t buffer_size, void *arg) {
      return ((jack_unit *)arg)->buffer_size_callback (buffer_size);
    }

    int unit_jack_sample_rate_callback (jack_nframes_t sample_rate, void *arg) {
      return ((jack_unit *)arg)->sample_rate_callback (sample_rate);
    }
  }

  struct plugin_unit : public jack_unit {
    bool m_internal_client;
    jack_client_t *m_jack_client;
    std::vector<jack_port_t *> m_jack_ports;
    std::vector<float *> m_jack_port_buffers;
    std::vector<std::vector<float>> m_zero_buffers;
    std::vector<float *> m_port_data_locations;
    std::vector<size_t> m_jack_input_port_indices;
    std::vector<size_t> m_jack_output_port_indices;
    jack_port_t *m_jack_midi_port;
    // TODO: allow more than one binding per port:
    std::vector<std::atomic<float>> m_atomic_port_values;
    std::vector<float> m_port_values;
    std::vector<std::atomic<midi_binding>> m_atomic_midi_bindings;

    plugin_ptr m_plugin;

    jack_nframes_t m_sample_rate;
    jack_nframes_t m_buffer_size;

    plugin_unit (plugin_ptr plugin, const std::string &jack_client_name, jack_client_t *jack_client, bool expose_control_ports) :
      jack_unit (expose_control_ports),
      m_internal_client (jack_client != 0),
      m_jack_client (jack_client),
      m_jack_ports (plugin->m_port_properties.size ()),
      m_jack_port_buffers (plugin->m_port_properties.size ()),
      m_zero_buffers (plugin->m_port_properties.size ()),
      m_port_data_locations (plugin->m_port_properties.size ()),
      m_atomic_port_values (plugin->m_port_properties.size ()),
      m_port_values (plugin->m_port_properties.size ()),
      m_atomic_midi_bindings (plugin->m_port_properties.size ()),
      m_plugin (plugin)
    {
      DBG_ENTER
      std::string client_name = jack_client_name;
      if (jack_client_name == "") client_name = "horst:" + m_plugin->get_name ();
      if (m_jack_client == 0) {
        m_jack_client = jack_client_open (client_name.c_str(), JackNullOption, 0);
      }
      if (m_jack_client == 0) throw std::runtime_error ("horst: plugin_unit: Failed to open jack client. Name: " + jack_client_name);

      m_sample_rate = jack_get_sample_rate (m_jack_client);
      m_buffer_size = jack_get_buffer_size (m_jack_client);

      m_plugin->instantiate (m_sample_rate, m_buffer_size);

      m_jack_midi_port = jack_port_register (m_jack_client, "midi-in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
      if (m_jack_midi_port == 0) throw std::runtime_error ("horst: plugin_unit: Failed to register midi port: " + m_plugin->get_name () + ":midi-in");

      for (size_t index = 0; index < plugin->m_port_properties.size(); ++index) {
        port_properties &p = m_plugin->m_port_properties[index];
        if (p.m_is_control && p.m_is_input) {
          DBG("port index: " << index << " default value: " << p.m_default_value)
          m_atomic_port_values[index] = p.m_default_value;
          m_port_values[index] = p.m_default_value;
        }
        if ((p.m_is_control && m_expose_control_ports) || p.m_is_audio || p.m_is_cv) {
          if (p.m_is_input) {
            DBG("port index: " << index << " - registering jack input port")
            m_jack_ports[index] = jack_port_register (m_jack_client, p.m_name.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
            if (m_jack_ports[index] == 0) throw std::runtime_error (std::string ("horst: plugin_unit: Failed to register port: ") + m_plugin->get_name () + ":" + p.m_name);
            m_jack_input_port_indices.push_back (index);
          } else {
            DBG("port index: " << index << " - registering jack output port")
            m_jack_ports[index] = jack_port_register (m_jack_client, p.m_name.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
            if (m_jack_ports[index] == 0) throw std::runtime_error (std::string ("horst: plugin_unit: Failed to register port: ") + m_plugin->get_name () + ":" + p.m_name);
            m_jack_output_port_indices.push_back (index);
          }
        }
      }

      int ret;

      ret = jack_set_sample_rate_callback (m_jack_client, unit_jack_sample_rate_callback, (void *)this);
      if (ret != 0) {
        throw std::runtime_error ("horst: plugin_unit: Failed to set sample rate callback");
      }

      ret = jack_set_process_callback (m_jack_client, unit_jack_process_callback, (void *)this);
      if (ret != 0) {
        throw std::runtime_error ("horst: plugin_unit: Failed to set buffer size callback");
      }

      ret = jack_set_buffer_size_callback (m_jack_client, unit_jack_buffer_size_callback, (void *)this);
      if (ret != 0) {
        throw std::runtime_error ("horst: plugin_unit: Failed to set buffer size callback");
      }

      ret = jack_activate (m_jack_client);
      if (ret != 0) {
        throw std::runtime_error ("horst: plugin_unit: Failed to activate client");
      }
    }

    std::string get_jack_client_name () override {
      return jack_get_client_name (m_jack_client);
    }

    virtual ~plugin_unit () {
      DBG_ENTER
      jack_deactivate (m_jack_client);

      if (m_internal_client) return;

      jack_client_close (m_jack_client);
      DBG_EXIT
    }

    virtual int process_callback (jack_nframes_t nframes) override {
      if (!m_plugin) return 0;

      bool enabled = m_atomic_enabled;
 
      for (size_t index = 0; index < m_plugin->m_port_properties.size(); ++index) {
        const port_properties &p = m_plugin->m_port_properties[index];
        if ((p.m_is_control && m_expose_control_ports) || p.m_is_audio || p.m_is_cv) {
          m_jack_port_buffers[index] = (float*)jack_port_get_buffer (m_jack_ports[index], nframes);
          if (p.m_is_input) {
            if (enabled) {
              m_port_data_locations[index] = m_jack_port_buffers[index];
            } else {
              m_port_data_locations[index] = &m_zero_buffers[index][0];
            }
          } else {
            m_port_data_locations[index] = m_jack_port_buffers[index];
          }
          m_plugin->connect_port (index, m_port_data_locations[index]);
        }
        if (p.m_is_control && !m_expose_control_ports) {
          if (p.m_is_input) {
            m_port_values[index] = m_atomic_port_values[index];
          } else {
            m_atomic_port_values[index] = m_port_values[index];
          }
          m_plugin->connect_port (index, &m_port_values[index]);
        }
      }

      void *midi_port_buffer = jack_port_get_buffer (m_jack_midi_port, nframes);
      int event_count = jack_midi_get_event_count (midi_port_buffer);

      jack_nframes_t processed_frames = 0;

      for (int event_index = 0; event_index < event_count; ++event_index) {
        jack_midi_event_t event;
        jack_midi_event_get (&event, midi_port_buffer, event_index);

        if (event.size != 3) continue;
        if ((event.buffer[0] & cc_mask) != cc_mask) continue;

        const int channel = event.buffer[0] & 15;
        const int cc = event.buffer[1];
        const float value = event.buffer[2] / 127.0f;

        bool changed = false;

        for (size_t port_index = 0; port_index < m_atomic_midi_bindings.size (); ++port_index) {
          const port_properties &props = m_plugin->m_port_properties[port_index];
          if (!(props.m_is_input && props.m_is_control)) continue;

          const midi_binding &binding = m_atomic_midi_bindings[port_index];

          if (!binding.m_enabled) continue;
          if (binding.m_cc != cc) continue;
          if (binding.m_channel != channel) continue;

          m_plugin->run (event.time - processed_frames);
          processed_frames = event.time;

          changed = true;

          const float transformed_value = binding.m_offset + binding.m_factor * value;

          const float mapped_value = props.m_minimum_value + (props.m_maximum_value - props.m_minimum_value) * transformed_value;

          // std::cout << event.time << " " << port_index << " " << mapped_value << "\n";

          m_port_values[port_index] = m_atomic_port_values[port_index] = mapped_value;
        }

        if (changed) {
          for (size_t port_index = 0; port_index < m_jack_port_buffers.size (); ++port_index) {
            const port_properties &p = m_plugin->m_port_properties[port_index];
            if ((p.m_is_control && m_expose_control_ports) || p.m_is_audio || p.m_is_cv) {
              m_plugin->connect_port (port_index, m_port_data_locations[port_index] + processed_frames);
            }
          }
        }
      }

      m_plugin->run (nframes - processed_frames);

      if (!enabled) {
        for (size_t port_index = 0; port_index < std::min (m_jack_input_port_indices.size (), m_jack_output_port_indices.size ()); ++port_index) {
          for (jack_nframes_t frame_index = 0; frame_index < nframes; ++frame_index) {
            m_jack_port_buffers[m_jack_output_port_indices[port_index]][frame_index] += m_jack_port_buffers[m_jack_input_port_indices[port_index]][frame_index];
          }
        }
      }

      return 0;
    }

    virtual int buffer_size_callback (jack_nframes_t buffer_size) override {
      DBG_ENTER
      if (buffer_size != m_buffer_size) {
        m_buffer_size = buffer_size;
        // std::cout << "buffer size callback. buffer size: " << buffer_size << "\n";
        for (size_t port_index = 0; port_index < m_plugin->m_port_properties.size (); ++port_index) {
          m_zero_buffers[port_index].resize (buffer_size, 0);
        }

        DBG("re-instantiating")
        m_plugin->instantiate ((double)m_sample_rate, m_buffer_size);
      }
      DBG_EXIT
      return 0;
    }

    virtual int sample_rate_callback (jack_nframes_t sample_rate) override {
      DBG_ENTER
      if (sample_rate != m_sample_rate) {
        m_sample_rate = sample_rate;
        // std::cout << "sample rate callback. sample rate: " << sample_rate << "\n";
        DBG("re-instantiating")
        m_plugin->instantiate ((double)sample_rate, m_buffer_size);
      }
      DBG_EXIT
      return 0;
    }

    int get_number_of_ports () override {
      return (int)(m_plugin->m_port_properties.size ());
    }

    void set_control_port_value (size_t index, float value) override {
      if (index >= m_port_values.size ()) {
        throw std::runtime_error ("horst: plugin_unit: index out of bounds");
      }
      m_atomic_port_values [index] = value;
    }

    float get_control_port_value (size_t index) override {
      if (index >= m_port_values.size ()) {
        throw std::runtime_error ("horst: plugin_unit: index out of bounds");
      }
      return m_atomic_port_values [index];
    }

    void set_midi_binding (size_t index, const midi_binding &binding) override {
      if (index >= m_port_values.size ()) {
        throw std::runtime_error ("horst: plugin_unit: index out of bounds");
      }
      m_atomic_midi_bindings[index] = binding;
    }

    midi_binding get_midi_binding (size_t index) override {
      if (index >= m_port_values.size ()) {
        throw std::runtime_error ("horst: plugin_unit: index out of bounds");
      }
      return m_atomic_midi_bindings[index];
    }

    virtual port_properties get_port_properties (int index) override {
      return m_plugin->m_port_properties[index];
    }
  };

  struct internal_plugin_unit : public unit {
    jack_client_ptr m_jack_client;
    jack_intclient_t m_jack_intclient;

    internal_plugin_unit (jack_client_ptr jack_client, jack_intclient_t jack_intclient) :
      m_jack_client (jack_client),
      m_jack_intclient (jack_intclient) {
      DBG_EXIT
    }

    ~internal_plugin_unit () {
      DBG_ENTER
      jack_internal_client_unload (m_jack_client->m, m_jack_intclient);
      DBG_EXIT
    }
  };
}
