import horst

h = horst.horst()

class lv2:
    def __init__(self, uri, jack_client_name, expose_control_ports):
        self.unit = h.lv2_unit (uri, jack_client_name, expose_control_ports)
        self.port_indices = {}
        self.port_properties = {}
        for index in range (h.get_number_of_ports (self.unit)):
            props = h.get_port_properties (self.unit, index)
            self.port_properties[index] = props
            self.port_properties[props.name] = props
            self.port_indices[props.name] = index

    def set_control_port_value (self, index, value):
        h.set_control_port_value (p, index, value)
