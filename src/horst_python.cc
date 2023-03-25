#include <pybind11/pybind11.h>
#include <horst/horst.h>

namespace bp = pybind11;

PYBIND11_MODULE(horst, m)
{
  bp::class_<horst::port_properties>(m, "port_properties")
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

  bp::class_<horst::connection>(m, "connection")
    .def (bp::init<const std::string&, const std::string&> ())
  ;

  void (horst::connections::*add1)(const std::string &, const std::string &) = &horst::connections::add;
  void (horst::connections::*add2)(const horst::connection &) = &horst::connections::add;
  bp::class_<horst::connections>(m, "connections")
    .def (bp::init<> ())
    .def ("add", add1)
    .def ("add", add2)
  ;

  bp::class_<horst::midi_binding>(m, "midi_binding")
    .def (
      bp::init<bool, int, int, float, float>(), 
      bp::arg ("enabled") = false, bp::arg ("channel") = 0, bp::arg ("cc") = 0, bp::arg ("factor") = 1.0f, bp::arg ("offset") = 0.0f
    )
  ;

  bp::class_<horst::unit_wrapper> (m, "unit_wrapper");

  bp::class_<horst::horst, std::shared_ptr<horst::horst>> (m, "horst")
    .def (bp::init<> ())
    //.def ("create", &horst::horst::create)
    .def ("insert_ladspa_plugin", &horst::horst::insert_ladspa_plugin)
    .def ("lv2_unit", &horst::horst::create_lv2_unit)
    .def ("lv2_internal_unit", &horst::horst::create_lv2_internal_unit)
    .def ("set_control_port_value", &horst::horst::set_control_port_value)
    .def ("get_control_port_value", &horst::horst::get_control_port_value)
    .def ("get_port_properties", &horst::horst::get_port_properties)
    .def ("set_midi_binding", &horst::horst::set_midi_binding)
    .def ("get_midi_binding", &horst::horst::get_midi_binding)
    .def ("get_number_of_ports", &horst::horst::get_number_of_ports)
    .def ("connect", &horst::horst::connect)
  ;
}
