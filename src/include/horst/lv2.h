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
    lilv_world_ptr m_world;

    lilv_plugins (lilv_world_ptr world) :
      m (lilv_world_get_all_plugins (world->m)),
      m_world (world) {
    }
  };

  typedef std::shared_ptr<lilv_plugins> lilv_plugins_ptr;

  struct lilv_uri_node {
    const std::string m_uri;
    LilvNode *m;
    lilv_world_ptr m_world;

    lilv_uri_node (lilv_world_ptr world, const std::string &uri) :
      m_uri (uri),
      m (lilv_new_uri (world->m, uri.c_str ())),
      m_world (world) {
      if (m == 0) throw std::runtime_error ("horst: lilv_uri_node: Failed to create lilv uri node. URI: " + uri);
    }

    ~lilv_uri_node () {
      lilv_node_free (m);
    }
  };

  typedef std::shared_ptr<lilv_uri_node> lilv_uri_node_ptr;

  struct lilv_plugin {
    const LilvPlugin *m;
    lilv_uri_node_ptr m_uri_node;
    lilv_plugins_ptr m_lilv_plugins;

    lilv_plugin (lilv_plugins_ptr plugins, const lilv_uri_node_ptr &node) :
      m (lilv_plugins_get_by_uri (plugins->m, node->m)),
      m_uri_node (node),
      m_lilv_plugins (plugins) {
    }
  };

  typedef std::shared_ptr<lilv_plugin> lilv_plugin_ptr;

  struct lilv_plugin_instance {
    LilvInstance *m;
    lilv_plugin_ptr m_plugin;

    lilv_plugin_instance (lilv_plugin_ptr plugin, double sample_rate) :
      m (lilv_plugin_instantiate (plugin->m, sample_rate, 0)),
      m_plugin (plugin)
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
