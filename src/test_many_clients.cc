#include <jack/jack.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <unistd.h>
#include <iostream>

extern "C" {
  int process (jack_nframes_t nframes, void *arg) {
    return 0;
  }
}

const size_t N = 100;

int main () {
  std::vector<jack_client_t *> clients(N);
  std::vector<std::string> client_names(N);
  for (size_t index = 0; index < N; ++index) {
    clients[index] = jack_client_open("test", JackNullOption, 0);
    if (clients[index] == 0) throw std::runtime_error ("failed to open client");
    client_names[index] = jack_get_client_name (clients[index]);
    int ret;
    ret = jack_set_process_callback (clients[index], process, 0);
    if (ret != 0) throw std::runtime_error ("failed to set process callback");
    ret = jack_activate (clients[index]);
    if (ret != 0) throw std::runtime_error ("failed to activate client");
  }

  jack_connect(clients[0], "system:capture_1", (client_names[0] + ":in").c_str());
  jack_connect(clients[0], (client_names[N-1] + ":out").c_str(), "system:playback_1");

  for (size_t index = 1; index < N; ++index) {
    jack_connect(clients[0], (client_names[index-1] + ":out").c_str(), (client_names[index] + ":in").c_str());
  }

  std::cout << "EOF to exit\n";
  int n;
  std::cin >> n;

  for (size_t index = 0; index < N; ++index) {
    jack_deactivate(clients[index]);
    jack_client_close(clients[index]);
  }
}
