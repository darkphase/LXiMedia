#include "backend.h"
#include "platform/messageloop.h"
#include <cstdio>
#include <iostream>
#include <memory>

#if defined(__unix__)
# include <clocale>
#endif

static FILE *logfile = nullptr;
static std::string create_logfile();
static void close_logfile()
{
    if (logfile)
        fclose(logfile);

    logfile = nullptr;
}

int main(int /*argc*/, const char */*argv*/[])
{
    // Allocate on heap to keep the stack free.
    const std::unique_ptr<class messageloop> messageloop(new class messageloop());
    const std::string logfile = create_logfile();

    int result = 1;
    {
        std::unique_ptr<class backend> backend;
        ::backend::recreate_backend = [&messageloop, &logfile, &backend]
        {
            backend = nullptr;
            backend.reset(new class backend(*messageloop, logfile));
            if (!backend->initialize())
            {
                std::clog << "[" << backend.get() << "] failed to initialize backend; stopping." << std::endl;
                messageloop->stop(1);
            }
        };

        messageloop->post(::backend::recreate_backend);
        result = messageloop->run();
        ::backend::recreate_backend = nullptr;
    }

    close_logfile();

    return result;
}

#if defined(__unix__)
static std::string create_logfile()
{
    close_logfile();

    std::string filename = "/var/log/lximcbackend.log";
    logfile = freopen(filename.c_str(), "w", stderr);
    if (logfile == nullptr)
    {
        filename = "/tmp/lximcbackend.log";
        logfile = freopen(filename.c_str(), "w", stderr);
    }

    if (logfile == nullptr)
        filename.clear();

    return filename;
}
#elif defined(WIN32)
#include "platform/path.h"

static std::string create_logfile()
{
    close_logfile();

    const wchar_t *temp = _wgetenv(L"TEMP");
    if (temp == nullptr)
        temp = _wgetenv(L"TMP");

    if (temp)
    {
        std::wstring filename = std::wstring(temp) + L"\\lximcbackend.log";
        logfile = _wfreopen(filename.c_str(), L"w", stderr);
        if (logfile == nullptr)
            filename.clear();

        return from_windows_path(filename);
    }

    return std::string();
}
#endif
