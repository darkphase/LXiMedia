#include "test.h"
#include <iostream>
#include <vector>

#if defined(__unix__)
# include <clocale>
#endif

struct test * test::current = nullptr;

int main(int /*argc*/, const char */*argv*/[])
{
#if defined(__unix__)
  // Ensure UTF-8 is enabled.
  setlocale(LC_ALL, "");
#endif

  std::vector<struct test *> tests;
  for (struct test *i = test::current; i; i = i->previous)
      tests.push_back(i);

  int result = 0;
  for (auto i = tests.rbegin(); i != tests.rend(); i++)
  {
      std::cout << "start " << (*i)->name << std::endl;

      try
      {
          (*i)->func();
      }
      catch(const std::exception &e)
      {
          std::cout
                  << "error " << e.what() << std::endl
                  << "fail  " << (*i)->name << std::endl;

          result++;
          continue;
      }

      std::cout << "pass  " << (*i)->name << std::endl;
  }

  return result;
}
