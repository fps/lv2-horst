import horst

from horst import midi_binding
from horst import port_properties
from horst import connections
from horst import connection

import weakref
import subprocess
import re
from collections import namedtuple

the_horst = horst.horst()

def string_to_identifier(varStr): return re.sub('\W|^(?=\d)','_', varStr)

class uris_info:
  def __init__(self):
    self.uris = the_horst.lv2_uris()
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

class with_ports:
  pass

class dict_with_attributes:
  def __init__(self):
    self.__d = {}

  def __getitem__(self, name):
    return self.__d[name]

  def __setitem__(self, name, value):
    self.__d[name] = value

  def __len__(self):
    return len(self.__d)

class unit(with_ports):
  def __init__(self, unit, jack_client_name, expose_control_ports):
    self.unit = unit
    self.jack_client_name = jack_client_name

    self.ports = dict_with_attributes()

    self.audio = dict_with_attributes()
    self.audio_in = dict_with_attributes()
    self.audio_out = dict_with_attributes()

    self.side_chain = dict_with_attributes()
    self.side_chain_in = dict_with_attributes()
    self.side_Chain_out = dict_with_attributes()

    self.control = dict_with_attributes()
    self.control_in = dict_with_attributes()
    self.control_out = dict_with_attributes()

    self.cv = dict_with_attributes()
    self.cv_in = dict_with_attributes()
    self.cv_out = dict_with_attributes()

    audio_port_index = 0
    audio_in_port_index = 0
    audio_out_port_index = 0
    
    side_chain_port_index = 0
    side_chain_in_port_index = 0
    side_chain_out_port_index = 0

    for index in range (self.unit.get_number_of_ports ()):
      p = props (self, index)
      p.jack_name = self.jack_client_name + ":" + p.name
      setattr(self, '_' + p.name, p)
      self.ports[index] = p

      if p.is_audio and not p.is_side_chain:
        self.audio[audio_port_index] = p
        setattr(self.audio, '_'+p.name, p)
        audio_port_index += 1

        if p.is_input:
          self.audio_in[audio_in_port_index] = p
          setattr(self.audio_in, '_'+p.name, p)
          audio_in_port_index += 1
          
        if p.is_output:
          self.audio_out[audio_out_port_index] = p
          setattr(self.audio_out, '_'+p.name, p)
          audio_out_port_index += 1

  def __getitem__(self, index):
    return self.ports[index]

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
    unit.__init__ (self, the_horst.lv2 (uri, jack_client_name, expose_control_ports), jack_client_name, expose_control_ports)

  blacklisted_uris = [
    'http://github.com/blablack/ams-lv2/fftvocoder'
  ]

class system_ports(with_ports):
  def __init__(self):
    self.audio_in = [namedtuple('foo', 'jack_name', defaults=["system:playback_" + str(n)])() for n in range(1,256)]
    self.audio_out = [namedtuple('foo', 'jack_name', defaults=["system:capture_" + str(n)])() for n in range(1,256)]

system = system_ports()

def connect2(source, sink):
  print('connect2: ' + str(source) + ' ' + str(sink))
  if isinstance(source, with_ports) and isinstance(sink, with_ports):
    count = min(len(source.audio_out), len(sink.audio_in))
    print(count)
    return [(source.audio_out[n].jack_name, sink.audio_in[n].jack_name) for n in range(count)]
  if isinstance(source, with_ports):
    return [connect2(source.audio_out[n].jack_name, sink) for n in range(len(source.audio_out))]
  if isinstance(sink, with_ports):
    return [connect2(source, sink.audio_in[n].jack_name) for n in range(len(sink.audio_in))]
  if isinstance(sink, props):
    return connect2(source, sink.jack_name)
  if isinstance(source, props):
    return connect2(source.jack_name, sink)
  print('base: ' + str(source) + ' ' + str(sink))
  return [(source, sink)]

def connect1(l):
  r = []
  print('connect1: ' + str(l))
  cs = horst.connections()
  for c in l:
    r = r + connect2(c[0], c[1])
  return r

def connect(*args):
  print('connect: ' + str(args))
  cs = []
  if len(args) == 1:
    cs = connect1(*args)
  if len(args) == 2:
    cs = connect2(*args)
  if len(args) > 2:
    for n in range(1,len(args)):
      cs = cs + connect2(args[n-1], args[n])
  print('final connections: ' + str(cs))

  hcs = horst.connections()
  for c in cs:
    hcs.add(c[0], c[1])
  the_horst.connect(hcs)
