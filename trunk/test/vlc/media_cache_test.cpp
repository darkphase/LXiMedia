#include "test.h"
#include "vlc/media.cpp"
#include "vlc/media_cache.cpp"
#include "platform/fstream.h"
#include "platform/messageloop.h"
#include "platform/path.h"
#include <algorithm>

extern const uint8_t pm5544_png[15118];
extern const uint8_t pm5544_mp4[191961];
extern const uint8_t pm5544_srt[88];

static std::string write_file(const std::string &filename, const uint8_t *data, size_t size)
{
    platform::ofstream str(filename, std::ios::binary);
    test_assert(str.is_open());
    str.write(reinterpret_cast<const char *>(data), size);

    return filename;
}

namespace vlc {

static const struct media_cache_test
{
    const std::string filename;

    media_cache_test()
        : filename(platform::temp_file_path("pm5544.")),
          png_test(this, "vlc::media::png", &media_cache_test::png),
          mp4_test(this, "vlc::media::mp4", &media_cache_test::mp4)
    {
    }

    ~media_cache_test()
    {
        ::remove((filename + "png").c_str());
        ::remove((filename + "mp4").c_str());
        ::remove((filename + "srt").c_str());
    }

    struct test png_test;
    void png()
    {
        const std::string file = write_file(filename + "png", pm5544_png, sizeof(pm5544_png));
        ::remove((filename + "srt").c_str());

        class instance instance;
        auto media = media::from_file(instance, file);
        test_assert(static_cast< ::libvlc_media_t *>(media) != nullptr);
        test_assert(!media.mrl().empty());

        class media_cache media_cache;

        static const platform::uuid ref_uuid("6c9a8849-dbd4-5b7e-8f50-0916d1294251");
        test_assert(media_cache.uuid(media.mrl()) == ref_uuid);

        test_assert(media_cache.media_type(media) == media_type::picture);

        const auto media_info = media_cache.media_info(media);
        test_assert(media_info.tracks.size() == 1);
        for (const auto &track : media_info.tracks)
        {
            switch (track.type)
            {
            case track_type::unknown:
            case track_type::text:
            case track_type::audio:
                test_assert(false);
                break;

            case track_type::video:
                test_assert(std::abs(int(track.video.width) - 768) < 16);
                test_assert(std::abs(int(track.video.height) - 576) < 16);
                break;
            }
        }
    }

    struct test mp4_test;
    void mp4()
    {
        const std::string file = write_file(filename + "mp4", pm5544_mp4, sizeof(pm5544_mp4));
        write_file(filename + "srt", pm5544_srt, sizeof(pm5544_srt));

        class instance instance;
        auto media = media::from_file(instance, file);
        test_assert(static_cast< ::libvlc_media_t *>(media) != nullptr);
        test_assert(!media.mrl().empty());

        class media_cache media_cache;

        static const platform::uuid ref_uuid("c34d207f-0b0a-5a40-a119-76792ed46e18");
        test_assert(media_cache.uuid(media.mrl()) == ref_uuid);

        test_assert(media_cache.media_type(media) == media_type::video);

        const auto media_info = media_cache.media_info(media);
        test_assert(media_info.tracks.size() == 3);
        for (const auto &track : media_info.tracks)
        {
            switch (track.type)
            {
            case track_type::unknown:
                test_assert(false);
                break;

            case track_type::text:
                break;

            case track_type::audio:
                test_assert(track.audio.sample_rate == 44100);
                test_assert(track.audio.channels == 2);
                break;

            case track_type::video:
                test_assert(std::abs(int(track.video.width) - 768) < 16);
                test_assert(std::abs(int(track.video.height) - 576) < 16);
                break;
            }
        }

        test_assert(std::abs(media_info.duration.count() - 10000) < 100);
        test_assert(media_info.chapter_count == 0);
    }
} media_cache_test;

} // End of namespace
