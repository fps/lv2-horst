#include <horst/horst.h>
#include <boost/python.hpp>

namespace bp = boost::python;
BOOST_PYTHON_MODULE(horst)
{
  bp::class_<horst::lilv_world>("lilv_world");
  bp::class_<horst::plugin_base_wrapper>("plugin");
  bp::class_<horst::horst_jack>("horst")
    .def ("insert_ladspa_plugin", &horst::horst_jack::insert_ladspa_plugin)
    .def ("insert_lv2_plugin", &horst::horst_jack::insert_lv2_plugin)
    .def ("insert_lv2_plugin_internal", &horst::horst_jack::insert_lv2_plugin)
    .def ("remove_plugin", &horst::horst_jack::remove_plugin)
    .def ("set_plugin_parameter", &horst::horst_jack::set_plugin_parameter)
    .def ("number_of_plugins", &horst::horst_jack::number_of_plugins)
  ;
}
