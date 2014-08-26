#include "backend.h"
#include "messageloop.h"
#include <cstdio>
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

  std::string logfilename = "/var/log/lximcbackend.log";
  auto logfile = freopen(logfilename.c_str(), "w", stderr);
  if (logfile == nullptr)
  {
      logfilename = "/tmp/lximcbackend.log";
      logfile = freopen(logfilename.c_str(), "w", stderr);
  }

  if (logfile == nullptr)
      logfilename.clear();

  // Allocate these on heap to keep the stack free.
  const std::unique_ptr<class messageloop> messageloop(new class messageloop());
  const std::unique_ptr<class backend> backend(new class backend(*messageloop, logfilename));

  messageloop->post([&messageloop, &backend]
  {
    if (!backend->initialize())
    {
      std::clog << "[" << backend.get() << "] failed to initialize backend; stopping." << std::endl;
      messageloop->stop(1);
    }
  });

  const int result = messageloop->run();
  fclose(logfile);
  return result;
}
