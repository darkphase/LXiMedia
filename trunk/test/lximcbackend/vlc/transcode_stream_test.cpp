#include "test.h"
#include "lximcbackend/vlc/transcode_stream.cpp"
#include <algorithm>
#include <fstream>

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

        class messageloop messageloop;
        class instance instance;

        // Transcode file.
        const std::string outfile = filename("output.ts");
        {
            class transcode_stream transcode_stream(messageloop, instance);

            static const char transcode[] =
                    "#transcode{"
                    "vcodec=mp2v,soverlay,vfilter=croppadd{paddleft=128,paddright=128},"
                    "acodec=mpga"
                    "}";
            test_assert(transcode_stream.open("file://" + infile, 0, transcode, "ts"));

            std::ofstream out(outfile, std::ios::binary);
            test_assert(out.is_open());
            std::copy(  std::istreambuf_iterator<char>(transcode_stream),
                        std::istreambuf_iterator<char>(),
                        std::ostreambuf_iterator<char>(out));
        }

        // Check output file.
        {
            auto media = media::from_file(instance, outfile);

            const auto tracks = media.tracks();
            test_assert(tracks.size() == 2);
            for (const auto &track : tracks)
            {
                switch (track.type)
                {
                case media::track_type::unknown:
                case media::track_type::text:
                    test_assert(false);
                    break;

                case media::track_type::audio:
                    test_assert(track.audio.sample_rate == 44100);
                    test_assert(track.audio.channels == 2);
                    break;

                case media::track_type::video:
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

static std::string write_file(const char *file, const uint8_t *data, size_t size)
{
    const std::string filename = ::filename(file);

    std::ofstream str(filename, std::ios::binary);
    test_assert(str.is_open());
    str.write(reinterpret_cast<const char *>(data), size);

    return filename;
}
#endif
