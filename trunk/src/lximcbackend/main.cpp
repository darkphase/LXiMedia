#include "backend.h"
#include "messageloop.h"
#include <iostream>
#include <memory>

#if defined(__unix__)
# include <clocale>
#endif

int main(int /*argc*/, const char */*argv*/[])
{
#if defined(__unix__)
  // Ensure UTF-8 is enabled.
  setlocale(LC_ALL, "");
#endif

  // Allocate these on heap to keep stack free.
  const std::unique_ptr<class messageloop> messageloop(new class messageloop());
  const std::unique_ptr<class backend> backend(new class backend(*messageloop));

  messageloop->post([&messageloop, &backend]
  {
    if (!backend->initialize())
    {
      std::clog << "[" << backend.get() << "] failed to initialize backend; stopping." << std::endl;
      messageloop->stop(1);
    }
  });

  return messageloop->run();
}
