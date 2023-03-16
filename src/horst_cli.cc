#include <horst/horst.h>
#include <horst/lart/heap.h>
#include <horst/lart/ringbuffer.h>

#include <stdlib.h>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <sstream>

#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>
#include <jack/jack.h>

lart::ringbuffer<std::function<void()>> the_commands(1024);
lart::heap the_heap;

horst::horst_ptr the_horst;

struct options {
  std::string m_jack_client_name;
  int m_buffer_size;
};

options the_options;

void handle_input(const std::string &line) {
  std::stringstream stream(line);

  std::string command;
  stream >> command; 

  if (command == "add-rack") {
    // std::cout << "Add rack\n";
  }

  if (command == "add-ladspa-plugin") {
    // std::cout << "Add LADSPA plugin\n";
  }

  if (command == "add-lv2-plugin") {
    // std::cout << "Add LV2 plugin\n";
  }

  if (command == "dump-state") {
    
  }
}

extern "C" {
  int process(jack_nframes_t nframes, void *arg) {
    return 1;
  }
}

int main(int argc, char *argv[]) {
  try {
    namespace po = boost::program_options;
  
    po::options_description desc ("Options");
    desc.add_options ()
      ("help,h", "produce this help message")
      ("jack-client-name,j", po::value<std::string>(&the_options.m_jack_client_name)->default_value("horst"), "the jack client name to use")
    ;
  
    po::variables_map vm;
    po::store (po::parse_command_line (argc, argv, desc), vm);
    po::notify (vm);
  
    if (vm.count ("help")) {
      std::cout << desc << "\n";
      return EXIT_SUCCESS;
    }
  } catch (std::runtime_error &e) {
      std::cerr << "Error parsing commandline: " << e.what() << "\n";
      return EXIT_FAILURE;
  }

  horst::horst_jack the_horst_jack(the_options.m_jack_client_name);

  bool done = false;
  while (!done) {
    try {
      char *line = readline ("horst> ");
      
      if (line) {
        if (*line) {
          handle_input(line);
          add_history (line);
        }
        free (line);
      } else {
        done = true;
      }
    } catch (std::runtime_error &e) {
      std::cerr << "Error handling input: " << e.what() << "\n";
    }
  }
}
