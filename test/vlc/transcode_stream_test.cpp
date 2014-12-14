#include "test.h"
#include "vlc/transcode_stream.cpp"
#include "vlc/media_cache.h"
#include "platform/fstream.h"
#include "platform/path.h"
#include <algorithm>

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

static const struct transcode_stream_test
{
    const std::string filename;

    transcode_stream_test()
        : filename(platform::temp_file_path("pm5544.")),
          transcode_test(this, "vlc::transcode_stream::transcode", &transcode_stream_test::transcode)
    {
    }

    ~transcode_stream_test()
    {
        ::remove((filename + "mp4").c_str());
        ::remove((filename + "srt").c_str());
        ::remove((filename + "ts" ).c_str());
    }

    struct test transcode_test;
    void transcode()
    {
        // VLC 2.2.0-pre2 crashes in this test.
        if (instance::compare_version(2, 2, 0) == 0)
            return;

        const std::string infile = write_file(filename + "mp4", pm5544_mp4, sizeof(pm5544_mp4));
        write_file(filename + "srt", pm5544_srt, sizeof(pm5544_srt));

        class instance instance;
        class media_cache media_cache;

        // Transcode file.
        const std::string outfile = filename + "ps";
        {
            struct transcode_stream::track_ids track_ids;
            class media media = media::from_file(instance, infile);
            for (auto &track : media_cache.media_info(media).tracks)
                switch (track.type)
                {
                case track_type::unknown:   break;
                case track_type::audio:     track_ids.audio = track.id; break;
                case track_type::video:     track_ids.video = track.id; break;
                case track_type::text:      track_ids.text  = track.id; break;
                }

            class platform::messageloop messageloop;
            class platform::messageloop_ref messageloop_ref(messageloop);
            class transcode_stream transcode_stream(messageloop_ref, instance);

            static const char transcode_nocroppadd[] =
                        "#transcode{"
                        "vcodec=mp2v,fps=25,soverlay,canvas{width=768,height=576},"
                        "acodec=mpga"
                        "}";

            test_assert(transcode_stream.open(media.mrl(), 0, track_ids, transcode_nocroppadd, "ps"));

            platform::ofstream out(outfile, std::ios::binary);
            test_assert(out.is_open());
            std::copy(  std::istreambuf_iterator<char>(transcode_stream),
                        std::istreambuf_iterator<char>(),
                        std::ostreambuf_iterator<char>(out));
        }

        // Check output file.
        {
            auto media = media::from_file(instance, outfile);

            const auto media_info = media_cache.media_info(media);
            test_assert(media_info.tracks.size() == 2);
            for (const auto &track : media_info.tracks)
            {
                switch (track.type)
                {
                case track_type::unknown:
                case track_type::text:
                    test_assert(false);
                    break;

                case track_type::audio:
                    test_assert(track.audio.sample_rate == 44100);
                    test_assert(track.audio.channels == 2);
                    break;

                case track_type::video:
                    test_assert(std::abs(int(track.video.width) - 1024) < 16);
                    test_assert(std::abs(int(track.video.height) - 576) < 16);
                    break;
                }
            }
        }
    }
} transcode_stream_test;

} // End of namespace
