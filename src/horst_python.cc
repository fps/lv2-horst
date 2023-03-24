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
    bp::init<bool, int, int, bp::optional<float, float>>())
  ;

  bp::class_<horst::unit_wrapper> ("unit_wrapper", bp::no_init);

  bp::class_<horst::horst_jack>("horst")
    .def ("insert_ladspa_plugin", &horst::horst_jack::insert_ladspa_plugin)
    .def ("lv2_unit", &horst::horst_jack::create_lv2_unit)
    .def ("lv2_internal_unit", &horst::horst_jack::create_lv2_internal_unit)
    .def ("set_control_port_value", &horst::horst_jack::set_control_port_value)
    .def ("get_control_port_value", &horst::horst_jack::get_control_port_value)
    .def ("get_control_port_index", &horst::horst_jack::get_control_port_index)
    .def ("set_midi_binding", &horst::horst_jack::set_midi_binding)
    .def ("get_midi_binding", &horst::horst_jack::get_midi_binding)
    .def ("connect", &horst::horst_jack::connect)
  ;
}
