#include <horst/horst.h>
#include <boost/python.hpp>

namespace bp = boost::python;

BOOST_PYTHON_MODULE(horst)
{
  bp::class_<horst::port_properties>("port_properties", bp::no_init)
    .def_readonly ("is_audio", &horst::port_properties::m_is_audio)
    .def_readonly ("is_control", &horst::port_properties::m_is_control)
    .def_readonly ("is_input", &horst::port_properties::m_is_input)
    .def_readonly ("is_output", &horst::port_properties::m_is_output)
    .def_readonly ("minimum_value", &horst::port_properties::m_minimum_value)
    .def_readonly ("default_value", &horst::port_properties::m_default_value)
    .def_readonly ("maximum_value", &horst::port_properties::m_maximum_value)
    .def_readonly ("is_logarithmic", &horst::port_properties::m_is_logarithmic)
    .def_readonly ("name", &horst::port_properties::m_name)
  ;

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
    .def ("get_port_properties", &horst::horst_jack::get_port_properties)
    .def ("set_midi_binding", &horst::horst_jack::set_midi_binding)
    .def ("get_midi_binding", &horst::horst_jack::get_midi_binding)
    .def ("get_number_of_ports", &horst::horst_jack::get_number_of_ports)
    .def ("connect", &horst::horst_jack::connect)
  ;
}
