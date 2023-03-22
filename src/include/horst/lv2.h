#pragma once

#include <lilv/lilv.h>

namespace horst {

  struct lilv_world {
    LilvWorld *m;

    lilv_world () {
      m = lilv_world_new ();
      if (m == 0) throw std::runtime_error ("horst: lilv_world: Failed to create lilv world");
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
      if (m == 0) throw std::runtime_error ("horst: lilv_uri_node: Failed to create lilv uri node. URI: " + uri);
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
      if (m == 0) throw std::runtime_error ("horst: lilv_plugin_instance: Failed to instantiate plugin");
      lilv_instance_activate (m);
    }

    ~lilv_plugin_instance () {
      lilv_instance_deactivate (m);
      lilv_instance_free (m);
    }
  };

  typedef std::shared_ptr<lilv_plugin_instance> lilv_plugin_instance_ptr;
}
