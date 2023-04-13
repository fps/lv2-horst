#include <jack/jack.h>
#include <iostream>
#include <unistd.h>

int main(int argc, char *argv[]) {
  jack_client_t *client = jack_client_open ("cpu", JackNullOption, 0);
  while (true) {
    std::cout << jack_cpu_load (client) << "\n";
    usleep (1000000);
  }
}
