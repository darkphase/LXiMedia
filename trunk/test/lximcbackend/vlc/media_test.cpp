#include "test.h"
#include "lximcbackend/vlc/media.cpp"
#include <algorithm>
#include <fstream>

extern const uint8_t pm5544_png[15118];
extern const uint8_t pm5544_mp4[191961];
extern const uint8_t pm5544_srt[88];

static std::string filename(const char *);
static std::string write_file(const char *, const uint8_t *, size_t);

namespace vlc {

static const struct media_test
{
    media_test()
        : png_test("vlc::media::png", &media_test::png),
          mp4_test("vlc::media::mp4", &media_test::mp4)
    {
    }

    ~media_test()
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
        test_assert(media.mrl() == ("file://" + file));

        const auto tracks = media.tracks();
        test_assert(tracks.size() == 1);
        for (const auto &track : tracks)
        {
            switch (track.type)
            {
            case media::track_type::unknown:
            case media::track_type::text:
            case media::track_type::audio:
                test_assert(false);
                break;

            case media::track_type::video:
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
        test_assert(media.mrl() == ("file://" + file));

        const auto tracks = media.tracks();
        test_assert(tracks.size() == 3);
        for (const auto &track : tracks)
        {
            switch (track.type)
            {
            case media::track_type::unknown:
                test_assert(false);
                break;

            case media::track_type::text:
                break;

            case media::track_type::audio:
                test_assert(track.audio.sample_rate == 44100);
                test_assert(track.audio.channels == 2);
                break;

            case media::track_type::video:
                test_assert(std::abs(track.video.width - 768) < 16);
                test_assert(std::abs(track.video.height - 576) < 16);
                break;
            }
        }

        test_assert(std::abs(media.duration().count() - 10000) < 100);
        test_assert(media.chapter_count() == 0);
    }
} media_test;

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

static std::string filename(const char *file)
{
    const char * const temp = getenv("TEMP");
    if (temp)
        return std::string(temp) + '\\'  + std::to_string(_getpid()) + '.' + file;

    throw std::runtime_error("failed to get TEMP directory");
}
#endif

static std::string write_file(const char *file, const uint8_t *data, size_t size)
{
    const std::string filename = ::filename(file);

    std::ofstream str(filename, std::ios::binary);
    test_assert(str.is_open());
    str.write(reinterpret_cast<const char *>(data), size);

    return filename;
}
