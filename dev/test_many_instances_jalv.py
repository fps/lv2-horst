import subprocess
import time 

N = 56

j = [subprocess.Popen(["jalv", "-n", str(n), "http://fps.io/plugins/state-variable-filter-v2"], stdin=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=0) for n in range(N)]

time.sleep(5)
print("connecting")

subprocess.check_call(["jack_connect", "system:capture_1", "0:in"])
subprocess.check_call(["jack_connect", str(N-1) + ":out", "system:playback_1"])

for n in range(1, N):
  subprocess.check_call(["jack_connect", str(n-1)+":out", str(n)+":in"])

print("running")
time.sleep(5)

print("closing stdin")
map(lambda x: x.stdin.flush(), j)
map(lambda x: x.stdin.close(), j)

time.sleep(5)
print("terminating")
map(lambda x: x.terminate(), j)

time.sleep(5)

print("killing")
map(lambda x: x.kill(9), j)

print("waiting")
map(lambda x: x.wait(), j)
