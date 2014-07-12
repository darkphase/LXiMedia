#include "backend.h"
#include "messageloop.h"
#include <iostream>
#include <memory>

int main(int /*argc*/, const char */*argv*/[])
{
  // Allocate these on heap to keep stack free.
  const std::unique_ptr<class messageloop> messageloop(new class messageloop());
  const std::unique_ptr<class backend> backend(new class backend(*messageloop));

  messageloop->post([&messageloop, &backend]
  {
    if (!backend->initialize())
    {
      std::clog << "Failed to initialize backend; stopping." << std::endl;
      messageloop->stop(1);
    }
  });

  return messageloop->run();
}
