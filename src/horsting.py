import horst

from horst import midi_binding
from horst import port_properties
from horst import connections
from horst import connection

import weakref
import subprocess
import re

h = horst.horst()

def string_to_identifier(varStr): return re.sub('\W|^(?=\d)','_', varStr)

class uris_info:
  def __init__(self):
    self.uris = h.lv2_uris()
    self.identifiers = list(map(string_to_identifier, self.uris))
    self.identifiers_to_uris = {}
    for uri in self.uris:
      self.identifiers_to_uris[string_to_identifier(uri)] = uri
  def __dir__(self):
    return list(self.__dict__.keys()) + self.identifiers

  def __getattr__(self, name):
    if self.identifiers_to_uris[name]:
      return self.identifiers_to_uris[name]

uris = uris_info()

class ports:
  def __init__(self):
    self.props = {}
  def __getitem__(self, index):
    return self.props[index]
  def __len__(self):
    return len(self.props)

class props:
  def __init__(self, unit, index):
    self.unit = weakref.ref(unit)
    self.p = unit.get_port_properties(index)
    self.p.index = index

  def __getattr__(self, name):
    return getattr(self.p, name)

  def __dir__(self):
    return list(self.__dict__.keys()) + dir(self.p)

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
    self.ports = ports()
    self.audio_ports = ports()
    self.audio_input_ports = ports()
    self.audio_output_ports = ports()
    self.control_ports = ports()
    self.cv_ports = ports()
    audio_port_index = 0
    audio_input_port_index = 0
    audio_output_port_index = 0
    for index in range (self.unit.get_number_of_ports ()):
      p = props (self, index)
      p.jack_name = self.jack_client_name + ":" + p.name
      setattr(self, '_' + p.name, p)
      self.ports.props[index] = p
      self.ports.props[p.name] = p
      if p.is_audio:
        self.audio_ports.props[audio_port_index] = p
        audio_port_index += 1
        if p.is_input:
          self.audio_input_ports.props[audio_input_port_index] = p
          audio_input_port_index += 1
        if p.is_output:
          self.audio_output_ports.props[audio_output_port_index] = p
          audio_output_port_index += 1

  def __getattr__(self, name):
    return getattr(self.unit, name)

  def __dir__(self):
    return list(self.__dict__.keys()) + dir(self.unit)

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

  blacklisted_uris = [
    'http://github.com/blablack/ams-lv2/fftvocoder'
  ]

def chain(*args):
  connections = []
  for index in range(1, len(args)):
    print(f"connect {args[index-1]} -> {args[index]}")
    connections.append((args[index-1], args[index]))
  connect(connections)
  return args

def connect(connections):
  if type(connections) is horst.connections:
    h.connect(connections)
    return
  cs = horst.connections() 
  for connection in connections:
    if type(connection) is horst.connection:
      cs.add(connection)
      continue
 
    source = connection[0]
    sink = connection[1]
    if type(source) is props and type(sink) is props:
      source = source.jack_name
      sink = sink.jack_name
      cs.add(source, sink)
    if type(source) is lv2 and type(sink) is lv2:
      for n in range(0, min(len(source.audio_output_ports), len(sink.audio_input_ports))):
        print((source.audio_output_ports[n].jack_name, sink.audio_input_ports[n].jack_name))
        cs.add(source.audio_output_ports[n].jack_name, sink.audio_input_ports[n].jack_name)
  h.connect(cs)
    
