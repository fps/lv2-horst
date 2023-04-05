#pragma once

#include <vector>
#include <list>
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <atomic>

#include <jack/jack.h>
#include <jack/intclient.h>
#include <jack/midiport.h>
#include <jack/control.h>

#include <lilv/lilv.h>
#include <lv2/options/options.h>
#include <lv2/buf-size/buf-size.h>
#include <lv2/atom/atom.h>
#include <lv2/worker/worker.h>

#include <dlfcn.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#ifdef HORST_DEBUG
#define DBG(x) { std::cerr << "  " << __FILE__ << ":" << __LINE__ << ":" << __PRETTY_FUNCTION__ << ": " << x << std::endl << std::flush; }
//#define DBG_JACK(x) { jack_info ("%s:%s:%s: %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, x); }
#define DBG_JACK DBG
#define DBG_ITEM(x) { std::cerr << x ; }
#else
#define DBG(x) { }
#define DBG_JACK(x) { }
#define DBG_ITEM(x) { }
#endif

#define DBG_ENTER DBG("<- enter...")
#define DBG_EXIT DBG("-> done.")
#define DBG_ENTER_EXIT DBG("<- enter ... -> done")

#include <horst/unit.h>

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

  struct horst {
    lilv_world_ptr m_lilv_world;
    lilv_plugins_ptr m_lilv_plugins;
    std::string m_horst_dli_fname;
    std::string m_jack_dli_fname;
    jack_client_ptr m_jack_client;

    horst () :
      m_lilv_world (new lilv_world),
      m_lilv_plugins (new lilv_plugins (m_lilv_world)),
      m_jack_client (new jack_client ("horst-loader", JackNullOption))
    {
      DBG_ENTER

      Dl_info dl_info;

      if (dladdr ((const void*)dli_fname_test, &dl_info))
      {
        m_horst_dli_fname = dl_info.dli_fname;
        DBG("horst_dli_fname: " << m_horst_dli_fname)
        // std::cout << "horst: horst_dli_fname: " << m_horst_dli_fname << "\n";
      } else {
        throw std::runtime_error ("horst: horst: Failed to find horst dli_fname");
      }

      if (dladdr ((const void *)jack_set_process_callback, &dl_info))
      {
        m_jack_dli_fname = dl_info.dli_fname;
        DBG("jack_dli_fname: " << m_jack_dli_fname)
        // std::cout << "horst: jack_dli_fname: " << m_jack_dli_fname << "\n";
      } else {
        throw std::runtime_error ("horst: horst: Failed to find jack dli_fname");
      }

      DBG_EXIT
    }

    std::string get_internal_client_load_name (const std::string &dli_fname)
    {
      size_t number_of_path_components = 0;
      for (size_t index = 0; index < m_jack_dli_fname.size(); ++index)
      {
        if (m_jack_dli_fname[index] == '/') ++number_of_path_components;
      }

      std::string prefix = "";
      for (size_t index = 0; index < number_of_path_components+1; ++index)
      {
        prefix = prefix + "../";
      }
      
      std::string load_name = prefix + dli_fname.substr (0, dli_fname.size () - 3) + "_internal";
      // std::cout << "horst: load name: " << load_name << "\n";
      return load_name;
    }

    ~horst ()
    {
      DBG_EXIT
    }

    std::vector<std::string> lv2_uris ()
    {
      std::vector<std::string> uris;

      LILV_FOREACH (plugins, i, m_lilv_plugins->m)
      {
        const LilvPlugin* p = lilv_plugins_get(m_lilv_plugins->m, i);
        uris.push_back(lilv_node_as_uri(lilv_plugin_get_uri(p)));
      }

      return uris;
    }

    unit_ptr lv2 (const std::string &uri, const std::string &jack_client_name, bool expose_control_ports)
    {
      plugin_ptr p (new lv2_plugin (m_lilv_world, m_lilv_plugins, uri));
      jack_client_ptr j (new jack_client ((jack_client_name != "") ? jack_client_name : p->get_name (), JackNullOption));
      return unit_ptr (new plugin_unit (p, j, expose_control_ports));
    }

    unit_ptr ladspa (std::string library_file_name, std::string plugin_label)
    {
      return unit_ptr ();  
    }

    void connect (const connections& cs)
    {
      for (size_t index = 0; index < cs.m.size(); ++index)
      {
        jack_connect (m_jack_client->m, cs.m[index].m_from.c_str (), cs.m[index].m_to.c_str ());
      }
    }

    void disconnect (const connections& cs)
    {
      for (size_t index = 0; index < cs.m.size(); ++index)
      {
        jack_disconnect (m_jack_client->m, cs.m[index].m_from.c_str (), cs.m[index].m_to.c_str ());
      }
    }
  };

  typedef std::shared_ptr<horst> horst_ptr;
}


