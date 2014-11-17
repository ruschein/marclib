#include "util.h"
#include <iostream>


char *progname; // Must be set in main() with "progname = argv[0];";


void Error(const std::string &msg) {
  std::cerr << progname << ": " << msg << '\n';
  std::exit(EXIT_FAILURE);
}


void Warning(const std::string &msg) {
  std::cerr << progname << ": " << msg << '\n';
}
