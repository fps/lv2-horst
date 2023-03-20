import horst
import time 

uris = [
  "http://calf.sourceforge.net/plugins/Gate",
  "http://calf.sourceforge.net/plugins/Compressor",
  "http://fps.io/plugins/clipping.tanh",
  "http://calf.sourceforge.net/plugins/EnvelopeFilter",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://guitarix.sourceforge.net/plugins/gxts9#ts9sim",
  "http://moddevices.com/plugins/mod-devel/BigMuffPi",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://guitarix.sourceforge.net/plugins/gx_cabinet#CABINET",
  "http://drobilla.net/plugins/mda/Leslie",
  "http://calf.sourceforge.net/plugins/MultiChorus",
  "http://calf.sourceforge.net/plugins/Phaser",
  "http://calf.sourceforge.net/plugins/Reverb",
  "http://calf.sourceforge.net/plugins/Reverb",
  "http://drobilla.net/plugins/mda/DubDelay",
  "https://ca9.eu/lv2/bolliedelay",
  "http://calf.sourceforge.net/plugins/Saturator",
  "http://calf.sourceforge.net/plugins/Limiter",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/clipping.tanh",
  "http://fps.io/plugins/relative_dynamics",
  "http://guitarix.sourceforge.net/plugins/gx_cabinet#CABINET",
  "http://calf.sourceforge.net/plugins/HaasEnhancer",
  "http://calf.sourceforge.net/plugins/Reverb",
  "http://plugin.org.uk/swh-plugins/amp"
]

h = horst.horst()

index = 0
for uri in uris:
  try:
    h.insert_lv2_plugin(0, uri, str(index)+"_"+uri)
  except:
    print ("failed to insert plugin " + uri)
    pass
  index = index + 1

