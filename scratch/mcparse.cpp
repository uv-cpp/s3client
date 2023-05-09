#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <streambuf>
#include <string>

using namespace std;

int main(int, char **) {
  // string i((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());
  // cout << i << endl;
  ifstream is("/Users/ugovaretto/.mc/config.json");
  if (!is) {
    cerr << "Cannot open file" << endl;
    exit(1);
  }
  string txt((istreambuf_iterator<char>(is)), istreambuf_iterator<char>());
  // cout << txt << endl;
  const string profile = "s3test";
  regex c(string("\"") + profile + "\"\\s*:\\s*\\{\\s*" + "([^\\}]+)\\s*\\}");
  smatch sm;
  regex_search(txt, sm, c);
  string entry = sm[1];
  cout << entry << endl;
  string es = string("\"(\\w+)\"\\s*:\\s*\"([^\\s,]+)\"\\s*,?\\s*");
  regex c2(es);
  map<string, string> cred;
  string::const_iterator searchStart(entry.cbegin());
  while (regex_search(searchStart, entry.cend(), sm, c2)) {
    if (sm.size() > 1) {
      cred[sm[1]] = sm[2];
    }
    searchStart = sm.suffix().first;
  }
  for (auto kv : cred) {
    cout << kv.first << ": " << kv.second << endl;
  }

  return 0;
}
