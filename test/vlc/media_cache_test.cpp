#include "test.h"
#include "vlc/media.cpp"
#include "vlc/media_cache.cpp"
#include "vlc/subtitles.cpp"
#include "platform/fstream.h"
#include "platform/messageloop.h"
#include "platform/path.h"
#include "resources/resource_file.h"
#include "resources/resources.h"
#include <algorithm>

namespace vlc {

static const struct media_cache_test
{
    const resources::resource_file pm5544_png;
    std::string media_cache_file;

    media_cache_test()
        : pm5544_png(resources::pm5544_png, "png"),
          media_cache_file(platform::temp_file_path("ini")),
          png_test(this, "vlc::media::png", &media_cache_test::png)
    {
    }

    ~media_cache_test()
    {
        if (!media_cache_file.empty())
            ::remove(media_cache_file.c_str());
    }

    struct test png_test;
    void png()
    {
        class platform::messageloop messageloop;
        class platform::messageloop_ref messageloop_ref(messageloop);
        auto mrl = platform::mrl_from_path(pm5544_png);
        test_assert(!mrl.empty());

        class platform::inifile inifile(media_cache_file, false);
        class media_cache media_cache(messageloop_ref, inifile);

        static const platform::uuid ref_uuid("6c9a8849-dbd4-5b7e-8f50-0916d1294251");
        test_assert(media_cache.uuid(mrl) == ref_uuid);

        test_assert(media_cache.media_type(mrl) == media_type::picture);

        const auto media_info = media_cache.media_info(mrl);
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
} media_cache_test;

} // End of namespace
