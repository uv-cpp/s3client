// Copy Markdown file adding {#mainpage} to title line
// to make Doxygen interpret the file as the main page
// for the documentation
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;
int main(int argc, char **argv) {
  ifstream is(argv[1]);
  ofstream os(argv[2]);
  if (argc != 3) {
    cerr << "usage: " << argv[0]
         << " <input Markdown file> <output Markdown file>" << endl;
    exit(EXIT_FAILURE);
  }
  string buf;
  bool found = false;
  while (getline(is, buf)) {
    if (!found) {
      auto i = buf.find_first_not_of(" \t\r\n");
      if (i != string::npos && buf[i] == '#') {
        buf += " {#mainpage}";
      }
      found = true;
    }
    os << buf << endl;
  }
  return EXIT_SUCCESS;
}
