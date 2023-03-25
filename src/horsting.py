import horst

from horst import midi_binding
from horst import port_properties
from horst import connections
from horst import connection

import weakref

h = horst.horst()

class ports:
    def __init__(self):
        self.props = []
    def __getitem__(self, index):
        return self.props[index]

class props:
  def __init__(self, unit, index):
    self.unit = weakref.ref(unit)
    self.p = unit.get_port_properties(index)
    self.p.index = index

  def __getattr__(self, name):
    return getattr(self.p, name)

  def get_value(self):
    return self.unit().get_control_port_value(self.p.index)

  def set_value(self, v):
    self.unit().set_control_port_value(self.p.index, v)

  value = property(get_value, set_value)
  

class unit:
    def __init__(self, unit, jack_client_name, expose_control_ports):
        self.unit = unit
        self.jack_client_name = jack_client_name
        self.port_indices = {}
        self.port_properties = {}
        self.ports = ports()
        for index in range (self.unit.get_number_of_ports ()):
            p = props (self.unit, index)
            p.jack_name = self.jack_client_name + ":" + p.name
            setattr(self, 'port_' + p.name, p)
            self.port_properties[index] = p
            self.port_properties[p.name] = p
            self.ports.props.append(p)      

    def __getattr__(self, name):
      return getattr(self.unit, name)

class lv2(unit):
    def __init__(self, uri, jack_client_name, expose_control_ports):
        unit.__init__ (self, h.lv2 (uri, jack_client_name, expose_control_ports), jack_client_name, expose_control_ports)

def connect(connections):
  h.connect(connections)
