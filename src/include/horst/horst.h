#pragma once

#include <vector>
#include <list>
#include <string>
#include <functional>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <iostream>

#include <jack/jack.h>
#include <jack/intclient.h>

#include <horst/unit.h>

#include <dlfcn.h>


namespace horst {

  extern "C" {
    void dli_fname_test () {

    }
  }

  struct connection {
    const std::string m_from;
    const std::string m_to;
    connection (const std::string &from, const std::string &to) :
      m_from (from), m_to (to) {

    }
  };

  struct connections {
    std::vector<connection> m;
    void add (const connection &c) {
      m.push_back (c);
    }

    void add (const std::string &from, const std::string &to) {
      m.push_back (connection (from, to));
    }
  };

  struct horst_jack {
    lilv_world m_lilv_world;
    lilv_plugins m_lilv_plugins;
    std::list<unit_ptr> m_units;
    std::string m_horst_dli_fname;
    std::string m_jack_dli_fname;
    jack_client_ptr m_jack_client;

    horst_jack () :
      m_lilv_plugins (m_lilv_world) {
      Dl_info dl_info;
      if (dladdr ((const void*)dli_fname_test, &dl_info)) {
        m_horst_dli_fname = dl_info.dli_fname;
        // std::cout << "horst: horst_dli_fname: " << m_horst_dli_fname << "\n";
      } else {
        throw std::runtime_error ("horst: horst_jack: Failed to find horst dli_fname");
      }
      if (dladdr ((const void *)jack_set_process_callback, &dl_info)) {
        m_jack_dli_fname = dl_info.dli_fname;
        // std::cout << "horst: jack_dli_fname: " << m_jack_dli_fname << "\n";
      } else {
        throw std::runtime_error ("horst: horst_jack: Failed to find jack dli_fname");
      }

      m_jack_client = jack_client_ptr (new jack_client ("horst-loader", JackNullOption));
    }

    std::string get_internal_client_load_name (const std::string &dli_fname) {
      size_t number_of_path_components = 0;
      for (size_t index = 0; index < m_jack_dli_fname.size(); ++index) {
        if (m_jack_dli_fname[index] == '/') ++number_of_path_components;
      }

      std::string prefix = "";
      for (size_t index = 0; index < number_of_path_components+1; ++index) {
        prefix = prefix + "../";
      }
      
      std::string load_name = prefix + dli_fname.substr (0, dli_fname.size () - 3) + "_internal";
      // std::cout << "horst: load name: " << load_name << "\n";
      return load_name;
    }

    ~horst_jack () {

    }

    void insert_lv2_plugin (int plugin_index, const std::string &uri, const std::string &jack_client_name) {
      auto it = m_units.begin ();
      for (int index = 0; index < plugin_index; ++index) ++it; 
      m_units.insert(it, unit_ptr (new plugin_unit (plugin_ptr (new lv2_plugin (m_lilv_world, m_lilv_plugins, uri)), jack_client_name, 0)));
    }

    void insert_lv2_plugin_internal (int plugin_index, const std::string &uri, const std::string &jack_client_name) {
      auto it = m_units.begin ();
      for (int index = 0; index < plugin_index; ++index) ++it; 

      jack_status_t jack_status;
      // jack_client_t *jack_client = jack_client_open ("horst-loader", JackNullOption, 0);

      jack_intclient_t jack_intclient = jack_internal_client_load (
        m_jack_client->m, jack_client_name.c_str (), 
        (jack_options_t)(JackLoadInit | JackLoadName), &jack_status, 
        get_internal_client_load_name (m_horst_dli_fname).c_str (), 
        ("lv2 " + uri).c_str ());

      if (jack_intclient == 0) {
        std::cout << jack_status << "\n";
        // jack_client_close (jack_client);
        throw std::runtime_error ("horst: horst_jack: Failed to create internal client. Name: " + jack_client_name);
      }
      // jack_client_close (jack_client);
      m_units.insert(it, unit_ptr (new internal_plugin_unit (m_jack_client, jack_intclient)));
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

    void connect (const connections& cs) {
      for (size_t index = 0; index < cs.m.size(); ++index) {
        jack_connect (m_jack_client->m, cs.m[index].m_from.c_str (), cs.m[index].m_to.c_str ());
      }
    }
  };
}


