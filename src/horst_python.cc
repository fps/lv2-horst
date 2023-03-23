#include <horst/horst.h>
#include <boost/python.hpp>

namespace bp = boost::python;
BOOST_PYTHON_MODULE(horst)
{
  bp::class_<horst::connection>("connection", 
    bp::init<const std::string&, const std::string&> ())
  ;

  void (horst::connections::*add1)(const std::string &, const std::string &) = &horst::connections::add;
  void (horst::connections::*add2)(const horst::connection &) = &horst::connections::add;
  bp::class_<horst::connections>("connections")
    .def ("add", add1)
    .def ("add", add2)
  ;

  bp::class_<horst::midi_binding>("midi_binding", 
    bp::init<int, bp::optional<int, float, float>> ())
  ;

  bp::class_<horst::horst_jack>("horst")
    .def ("insert_ladspa_plugin", &horst::horst_jack::insert_ladspa_plugin)
    .def ("insert_lv2_plugin", &horst::horst_jack::insert_lv2_plugin)
    .def ("insert_lv2_plugin_internal", &horst::horst_jack::insert_lv2_plugin)
    .def ("remove_plugin", &horst::horst_jack::remove_plugin)
    .def ("set_control_port_value", &horst::horst_jack::set_control_port_value)
    .def ("get_control_port_value", &horst::horst_jack::get_control_port_value)
    .def ("number_of_plugins", &horst::horst_jack::number_of_plugins)
    .def ("connect", &horst::horst_jack::connect)
  ;
}
