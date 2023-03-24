import horst

h = horst.horst()

class ports:
    def __init__(self):
        self.props = []
    def __getitem__(self, index):
        return self.props[index]

class lv2:
    def __init__(self, uri, jack_client_name, expose_control_ports):
        self.unit = h.lv2_unit (uri, jack_client_name, expose_control_ports)
        self.jack_client_name = jack_client_name
        self.port_indices = {}
        self.port_properties = {}
        self.ports = ports()
        for index in range (h.get_number_of_ports (self.unit)):
            props = h.get_port_properties (self.unit, index)
            props.index = index
            props.jack_name = self.jack_client_name + ":" + props.name
            props.__class__.value = property (lambda s: h.get_control_port_value(self.unit, index), lambda s, v: h.set_control_port_value(self.unit, index, v))
            setattr(self, 'port_' + props.name, props)
            self.port_properties[index] = props
            self.ports.props.append(props)
