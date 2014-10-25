#include "test.h"
#include "vlc/transcode_stream.cpp"
#include "vlc/media_cache.h"
#include "platform/fstream.h"
#include <algorithm>

extern const uint8_t pm5544_mp4[191961];
extern const uint8_t pm5544_srt[88];

static std::string filename(const char *);
static std::string write_file(const char *, const uint8_t *, size_t);

namespace vlc {

static const struct transcode_stream_test
{
    transcode_stream_test()
        : transcode_test("vlc::transcode_stream::transcode", &transcode_stream_test::transcode)
    {
    }

    ~transcode_stream_test()
    {
        ::remove(filename("pm5544.mp4").c_str());
        ::remove(filename("pm5544.srt").c_str());
        ::remove(filename("output.ts").c_str());
    }

    struct test transcode_test;
    static void transcode()
    {
        const std::string infile = write_file("pm5544.mp4", pm5544_mp4, sizeof(pm5544_mp4));
        write_file("pm5544.srt", pm5544_srt, sizeof(pm5544_srt));

        class platform::messageloop messageloop;
        class platform::messageloop_ref messageloop_ref(messageloop);
        class instance instance;
        class media_cache media_cache(messageloop_ref);

        // Transcode file.
        const std::string outfile = filename("output.ts");
        {
            struct transcode_stream::track_ids track_ids;
            class media media = media::from_file(instance, infile);
            for (auto &track : media_cache.tracks(media))
                switch (track.type)
                {
                case media_cache::track_type::unknown: break;
                case media_cache::track_type::audio:  track_ids.audio = track.id; break;
                case media_cache::track_type::video:  track_ids.video = track.id; break;
                case media_cache::track_type::text:   track_ids.text  = track.id; break;
                }

            class transcode_stream transcode_stream(messageloop_ref, instance);
            if (vlc::instance::compare_version(2, 1) == 0)
            {
                // Fallback for VLC 2.1
                static const char transcode_nocroppadd[] =
                        "#transcode{"
                        "vcodec=mp2v,fps=24,soverlay,width=1024,height=576,"
                        "acodec=mpga"
                        "}";

                test_assert(transcode_stream.open(media.mrl(), 0, track_ids, transcode_nocroppadd, "ts"));
            }
            else
            {
                static const char transcode[] =
                        "#transcode{"
                        "vcodec=mp2v,fps=24,soverlay,vfilter=croppadd{paddleft=128,paddright=128},"
                        "acodec=mpga"
                        "}";

                test_assert(transcode_stream.open(media.mrl(), 0, track_ids, transcode, "ts"));
            }

            platform::ofstream out(outfile, std::ios::binary);
            test_assert(out.is_open());
            std::copy(  std::istreambuf_iterator<char>(transcode_stream),
                        std::istreambuf_iterator<char>(),
                        std::ostreambuf_iterator<char>(out));
        }

        // Check output file.
        {
            auto media = media::from_file(instance, outfile);

            const auto tracks = media_cache.tracks(media);
            test_assert(tracks.size() == 2);
            for (const auto &track : tracks)
            {
                switch (track.type)
                {
                case media_cache::track_type::unknown:
                case media_cache::track_type::text:
                    test_assert(false);
                    break;

                case media_cache::track_type::audio:
                    test_assert(track.audio.sample_rate == 44100);
                    test_assert(track.audio.channels == 2);
                    break;

                case media_cache::track_type::video:
                    test_assert(std::abs(track.video.width - 1024) < 16);
                    test_assert(std::abs(track.video.height - 576) < 16);
                    break;
                }
            }
        }
    }
} transcode_stream_test;

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
        return platform::from_windows_path(std::wstring(temp) + L'\\' + std::to_wstring(_getpid()) + L'.' + platform::to_windows_path(file));

    throw std::runtime_error("failed to get TEMP directory");
}
#endif

static std::string write_file(const char *file, const uint8_t *data, size_t size)
{
    const std::string filename = ::filename(file);

    platform::ofstream str(filename, std::ios::binary);
    test_assert(str.is_open());
    str.write(reinterpret_cast<const char *>(data), size);

    return filename;
}
