#include "test.h"
#include "lximcbackend/vlc/media.cpp"
#include <fstream>

extern const uint8_t pm5544_png[15118];
extern const uint8_t pm5544_ogv[176003];
static std::string write_file(const char *, const uint8_t *, size_t);

namespace vlc {

static const struct media_test
{
    media_test()
        : png_test("vlc::media::png", &media_test::png),
          ogv_test("vlc::media::ogv", &media_test::ogv)
    {
    }

    struct test png_test;
    static void png()
    {
        const std::string file = write_file("pm5544.png", pm5544_png, sizeof(pm5544_png));

        class instance instance;
        auto media = media::from_file(instance, file);
        test_assert(static_cast< ::libvlc_media_t *>(media) != nullptr);
        test_assert(media.mrl() == ("file://" + file));

        const auto tracks = media.tracks();
        test_assert(tracks.size() == 1);

        const auto track = tracks[0];
        test_assert(track.type == media::track_type::video);
        test_assert(track.video.width == 768);
        test_assert(track.video.height == 576);

        ::remove(file.c_str());
    }

    struct test ogv_test;
    static void ogv()
    {
        const std::string file = write_file("pm5544.ogv", pm5544_ogv, sizeof(pm5544_ogv));

        class instance instance;
        auto media = media::from_file(instance, file);
        test_assert(static_cast< ::libvlc_media_t *>(media) != nullptr);
        test_assert(media.mrl() == ("file://" + file));

        const auto tracks = media.tracks();
        test_assert(tracks.size() == 1);

        const auto track = tracks[0];
        test_assert(track.type == media::track_type::video);
        test_assert(track.video.width == 768);
        test_assert(track.video.height == 576);

        test_assert(media.duration() == std::chrono::milliseconds(10000));
        test_assert(media.chapter_count() == 0);

        ::remove(file.c_str());
    }
} media_test;

} // End of namespace

#if defined(__unix__)
static std::string write_file(const char *file, const uint8_t *data, size_t size)
{
    const std::string filename = std::string("/tmp/") + file;

    std::ofstream str(filename);
    str.write(reinterpret_cast<const char *>(data), size);

    return filename;
}
#endif
