#include <horst/horst.h>
#include <boost/python.hpp>

namespace bp = boost::python;
BOOST_PYTHON_MODULE(horst)
{
  bp::class_<horst::lilv_world>("lilv_world");
  bp::class_<horst::connection>("connection", bp::init<const std::string&, const std::string&> ());
  bp::class_<horst::connections>("connections")
    .def ("add", &horst::connections::add);
  bp::class_<horst::horst_jack>("horst")
    .def ("insert_ladspa_plugin", &horst::horst_jack::insert_ladspa_plugin)
    .def ("insert_lv2_plugin", &horst::horst_jack::insert_lv2_plugin)
    .def ("insert_lv2_plugin_internal", &horst::horst_jack::insert_lv2_plugin)
    .def ("remove_plugin", &horst::horst_jack::remove_plugin)
    .def ("set_plugin_parameter", &horst::horst_jack::set_plugin_parameter)
    .def ("number_of_plugins", &horst::horst_jack::number_of_plugins)
    .def ("connect", &horst::horst_jack::connect)
  ;
}
