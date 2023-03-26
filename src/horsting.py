import horst

from horst import midi_binding
from horst import port_properties
from horst import connections
from horst import connection

import weakref
import subprocess

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
    return self.unit().unit.get_control_port_value(self.p.index)

  def set_value(self, v):
    self.unit().unit.set_control_port_value(self.p.index, v)

  value = property(get_value, set_value)

  def bind_midi(self, *args):
    self.unit().bind_midi(self.p.index, *args)

  def unbind_midi(self, *args):
    self.unit().unbind_midi(self.p.index, *args)
  

class unit:
  def __init__(self, unit, jack_client_name, expose_control_ports):
    self.unit = unit
    self.jack_client_name = jack_client_name
    self.port_indices = {}
    self.port_properties = {}
    self.ports = ports()
    for index in range (self.unit.get_number_of_ports ()):
      p = props (self, index)
      p.jack_name = self.jack_client_name + ":" + p.name
      setattr(self, '_' + p.name, p)
      self.port_properties[index] = p
      self.port_properties[p.name] = p
      self.ports.props.append(p)      

  def __getattr__(self, name):
    return getattr(self.unit, name)

  def bind_midi(self, port_index, channel, cc, factor = 1.0, offset = 0.0):
    b = horst.midi_binding(True, channel, cc, factor, offset)
    self.unit.set_midi_binding(port_index, b)

  def unbind_midi(self, port_index):
    b = horst.midi_binding(False, 0, 0, 0, 0)
    self.unit.set_midi_binding(port_index, b)

class lv2(unit):
  def __init__(self, uri, jack_client_name = "", expose_control_ports = False):
    if uri in lv2.blacklisted_uris:
      raise RuntimeError("blacklisted uri: " + uri)
    jack_client_name = uri if jack_client_name == "" else jack_client_name
    unit.__init__ (self, h.lv2 (uri, jack_client_name, expose_control_ports), jack_client_name, expose_control_ports)

  uris = subprocess.check_output(['lv2ls']).decode('utf-8').split('\n')
  blacklisted_uris = [
    'http://github.com/blablack/ams-lv2/fftvocoder'
  ]

def connect(connections):
  if type(connections) is horst.connections:
    h.connect(connections)
    return
  # h.connect(connections)
  cs = horst.connections() 
  for connection in connections:
    if type(connection) is horst.connection:
      cs.add(connection)
      continue
 
    source = connection[0]
    sink = connection[1]
    if type(source) is props:
      source = source.jack_name
    if type(sink) is props:
      sink = sink.jack_name
    cs.add(source, sink)
  h.connect(cs)
    
