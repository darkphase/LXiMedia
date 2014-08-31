#include "test.h"
#include <clocale>
#include <iostream>
#include <stdexcept>
#include <vector>

struct test * test::current = nullptr;

void test::assert_fail(const char *condition, const char *file, unsigned int line, const char *)
{
    throw std::runtime_error(std::string(file) + ':' + std::to_string(line) + ": " + condition);
}

int main(int /*argc*/, const char */*argv*/[])
{
    setlocale(LC_ALL, "");

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
            std::cerr << e.what() << std::endl;
            std::cout << "fail  " << (*i)->name << std::endl;
            result++;
            continue;
        }

        std::cout << "pass  " << (*i)->name << std::endl;
    }

    return result;
}
