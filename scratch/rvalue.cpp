#include <iostream>

using namespace std;

struct S {
  int &&r;
  // operator const int &() { return r; }
  // operator int &() { return r; }
  operator int &&() { return std::move(r); }
  S(int &&p) : r(std::move(p)) {}
};

int I = 10;
S foo(int i) { return std::move(I); }

int main(int, char **) {
  cout << foo(10) << endl;
  return 0;
}
