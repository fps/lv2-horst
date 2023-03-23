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

  bp::class_<horst::unit_wrapper> ("unit_wrapper", bp::no_init);

  float (horst::horst_jack::*get_cpv1)(horst::unit_wrapper, int) = &horst::horst_jack::get_control_port_value;
  float (horst::horst_jack::*get_cpv2)(horst::unit_wrapper, const std::string &) = &horst::horst_jack::get_control_port_value;
  void (horst::horst_jack::*set_cpv1)(horst::unit_wrapper, int, float) = &horst::horst_jack::set_control_port_value;
  void (horst::horst_jack::*set_cpv2)(horst::unit_wrapper, const std::string &, float) = &horst::horst_jack::set_control_port_value;
  bp::class_<horst::horst_jack>("horst")
    .def ("insert_ladspa_plugin", &horst::horst_jack::insert_ladspa_plugin)
    .def ("lv2_unit", &horst::horst_jack::create_lv2_unit)
    .def ("lv2_internal_unit", &horst::horst_jack::create_lv2_internal_unit)
    .def ("set_control_port_value", set_cpv1)
    .def ("set_control_port_value", set_cpv2)
    .def ("get_control_port_value", get_cpv1)
    .def ("get_control_port_value", get_cpv2)
    .def ("connect", &horst::horst_jack::connect)
  ;
}
