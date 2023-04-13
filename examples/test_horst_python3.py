import horsting as h
import time 

uris = [
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
  "http://fps.io/plugins/state-variable-filter-v2",
]

units = []

index = 0
for uri in uris:
  print("Adding: " + uri)
  units.append(h.lv2(uri))
  # time.sleep(0.1)
  index += 1
h.connect(h.system, *units, h.system)
