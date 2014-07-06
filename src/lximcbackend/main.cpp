#include "backend.h"
#include "messageloop.h"
#include <iostream>

int main(int /*argc*/, const char */*argv*/[])
{
  using namespace lximediacenter;

  class messageloop messageloop;
  class backend backend(messageloop);

  messageloop.post([&messageloop, &backend]
  {
    if (!backend.initialize())
    {
      std::clog << "Failed to initialize backend; stopping." << std::endl;
      messageloop.stop(1);
    }
  });

  return messageloop.run();
}
