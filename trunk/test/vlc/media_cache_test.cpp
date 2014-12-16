#include "test.h"
#include "vlc/media.cpp"
#include "vlc/media_cache.cpp"
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

    media_cache_test()
        : pm5544_png(resources::pm5544_png, "png"),
          png_test(this, "vlc::media::png", &media_cache_test::png)
    {
    }

    struct test png_test;
    void png()
    {
        class instance instance;
        auto media = media::from_file(instance, pm5544_png);
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
} media_cache_test;

} // End of namespace
