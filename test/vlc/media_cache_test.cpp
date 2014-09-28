#include "test.h"
#include "vlc/media.cpp"
#include "vlc/media_cache.cpp"
#include "platform/fstream.h"
#include "platform/messageloop.h"
#include <algorithm>

extern const uint8_t pm5544_png[15118];
extern const uint8_t pm5544_mp4[191961];
extern const uint8_t pm5544_srt[88];

static std::string filename(const char *);
static std::string write_file(const char *, const uint8_t *, size_t);

namespace vlc {

static const struct media_cache_test
{
    media_cache_test()
        : png_test("vlc::media::png", &media_cache_test::png),
          mp4_test("vlc::media::mp4", &media_cache_test::mp4)
    {
    }

    ~media_cache_test()
    {
        ::remove(filename("pm5544.png").c_str());
        ::remove(filename("pm5544.mp4").c_str());
        ::remove(filename("pm5544.srt").c_str());
    }

    struct test png_test;
    static void png()
    {
        const std::string file = write_file("pm5544.png", pm5544_png, sizeof(pm5544_png));
        ::remove(filename("pm5544.srt").c_str());

        class instance instance;
        auto media = media::from_file(instance, file);
        test_assert(static_cast< ::libvlc_media_t *>(media) != nullptr);
        test_assert(!media.mrl().empty());

        class messageloop messageloop;
        class media_cache media_cache(messageloop);
        const auto tracks = media_cache.tracks(media);
        test_assert(tracks.size() == 1);
        for (const auto &track : tracks)
        {
            switch (track.type)
            {
            case media_cache::track_type::unknown:
            case media_cache::track_type::text:
            case media_cache::track_type::audio:
                test_assert(false);
                break;

            case media_cache::track_type::video:
                test_assert(std::abs(track.video.width - 768) < 16);
                test_assert(std::abs(track.video.height - 576) < 16);
                break;
            }
        }
    }

    struct test mp4_test;
    static void mp4()
    {
        const std::string file = write_file("pm5544.mp4", pm5544_mp4, sizeof(pm5544_mp4));
        write_file("pm5544.srt", pm5544_srt, sizeof(pm5544_srt));

        class instance instance;
        auto media = media::from_file(instance, file);
        test_assert(static_cast< ::libvlc_media_t *>(media) != nullptr);
        test_assert(!media.mrl().empty());

        class messageloop messageloop;
        class media_cache media_cache(messageloop);
        const auto tracks = media_cache.tracks(media);
        test_assert(tracks.size() == 3);
        for (const auto &track : tracks)
        {
            switch (track.type)
            {
            case media_cache::track_type::unknown:
                test_assert(false);
                break;

            case media_cache::track_type::text:
                break;

            case media_cache::track_type::audio:
                test_assert(track.audio.sample_rate == 44100);
                test_assert(track.audio.channels == 2);
                break;

            case media_cache::track_type::video:
                test_assert(std::abs(track.video.width - 768) < 16);
                test_assert(std::abs(track.video.height - 576) < 16);
                break;
            }
        }

        test_assert(std::abs(media_cache.duration(media).count() - 10000) < 100);
        test_assert(media_cache.chapter_count(media) == 0);
    }
} media_cache_test;

} // End of namespace

#if defined(__unix__)
#include <unistd.h>

static std::string filename(const char *file)
{
    return std::string("/tmp/") + std::to_string(getpid()) + '.' + file;
}
#elif defined(WIN32)
#include <cstdlib>
#include <process.h>
#include "platform/path.h"

static std::string filename(const char *file)
{
    const wchar_t * const temp = _wgetenv(L"TEMP");
    if (temp)
        return from_windows_path(std::wstring(temp) + L'\\' + std::to_wstring(_getpid()) + L'.' + to_windows_path(file));

    throw std::runtime_error("failed to get TEMP directory");
}
#endif

static std::string write_file(const char *file, const uint8_t *data, size_t size)
{
    const std::string filename = ::filename(file);

    ofstream str(filename, std::ios::binary);
    test_assert(str.is_open());
    str.write(reinterpret_cast<const char *>(data), size);

    return filename;
}
