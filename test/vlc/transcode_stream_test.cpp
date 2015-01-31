#include "test.h"
#include "vlc/transcode_stream.cpp"
#include "vlc/media_cache.h"
#include "platform/fstream.h"
#include "platform/path.h"
#include "resources/resource_file.h"
#include "resources/resources.h"
#include <algorithm>

namespace vlc {

static const struct transcode_stream_test
{
    const resources::resource_file a440hz_flac;
    const resources::resource_file pm5544_png;
    std::string media_cache_file, out_file;

    transcode_stream_test()
        : a440hz_flac(resources::a440hz_flac, "flac"),
          pm5544_png(resources::pm5544_png, "png"),
          media_cache_file(platform::temp_file_path("ini")),
          transcode_mp2v_ps_test(this, "vlc::transcode_stream::transcode_mp2v_ps", &transcode_stream_test::transcode_mp2v_ps),
          transcode_mp2v_ts_test(this, "vlc::transcode_stream::transcode_mp2v_ts", &transcode_stream_test::transcode_mp2v_ts),
          transcode_h264_ts_test(this, "vlc::transcode_stream::transcode_h264_ts", &transcode_stream_test::transcode_h264_ts)
    {
    }

    ~transcode_stream_test()
    {
        if (!out_file.empty())
            ::remove(out_file.c_str());

        if (!media_cache_file.empty())
            ::remove(media_cache_file.c_str());
    }

    void transcode_base(const std::string &transcode, const char *mux)
    {
        class platform::messageloop messageloop;
        class platform::messageloop_ref messageloop_ref(messageloop);

        class platform::inifile inifile(media_cache_file);
        class media_cache media_cache(messageloop_ref, inifile);

        if (!out_file.empty())
            ::remove(out_file.c_str());

        out_file = platform::temp_file_path(mux);

        // Transcode file.
        {
            class transcode_stream transcode_stream(messageloop_ref);

            const auto a440hz_flac_mrl = platform::mrl_from_path(a440hz_flac);
            transcode_stream.add_option(":input-slave=" + a440hz_flac_mrl);

            const auto pm5544_png_mrl = platform::mrl_from_path(pm5544_png);

            struct vlc::track_ids track_ids;
            transcode_stream.set_track_ids(track_ids);

            test_assert(transcode_stream.open(pm5544_png_mrl, transcode, mux));

            platform::ofstream out(out_file, std::ios::binary);
            test_assert(out.is_open());

            bool finished = false;
            std::thread read_thread([&]
            {
                std::copy(  std::istreambuf_iterator<char>(transcode_stream),
                            std::istreambuf_iterator<char>(),
                            std::ostreambuf_iterator<char>(out));

                finished = true;
            });

            while (!finished) messageloop.process_events(std::chrono::milliseconds(16));
            read_thread.join();
        }

        // Check output file.
        {
            const auto media_info = media_cache.media_info(
                        platform::mrl_from_path(out_file));

            test_assert(media_info.tracks.size() == 2);
            if (instance::compare_version(2, 1) != 0)
            {
                test_assert(
                            (media_info.duration.count() == 0) ||
                            (std::abs(int(media_info.duration.count()) - 10000) < 1000));
            }

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
                    test_assert(std::abs(int(track.video.width) - 768) < 16);
                    test_assert(std::abs(int(track.video.height) - 576) < 16);
                    break;
                }
            }
        }

        ::remove(out_file.c_str());
        out_file.clear();
    }

    struct test transcode_mp2v_ps_test;
    void transcode_mp2v_ps()
    {
        std::ostringstream transcode;
        transcode << "#transcode{"
                  << "vcodec=mp2v,scale=Auto,fps=25,width=768,height=576,";

        // Workaround for ticket https://trac.videolan.org/vlc/ticket/10148
        if (instance::compare_version(2, 1) != 0)
            transcode << "vfilter=canvas{width=768,height=576},soverlay,";

        transcode << "acodec=mpga,samplerate=44100,channels=2"
                  << "}";

        transcode_base(transcode.str(), "ps");
    }

    struct test transcode_mp2v_ts_test;
    void transcode_mp2v_ts()
    {
        std::ostringstream transcode;
        transcode << "#transcode{"
                  << "vcodec=mp2v,scale=Auto,fps=25,width=768,height=576,";

        // Workaround for ticket https://trac.videolan.org/vlc/ticket/10148
        if (instance::compare_version(2, 1) != 0)
            transcode << "vfilter=canvas{width=768,height=576},soverlay,";

        transcode << "acodec=mpga,samplerate=44100,channels=2"
                  << "}";

        transcode_base(transcode.str(), "ts");
    }

    struct test transcode_h264_ts_test;
    void transcode_h264_ts()
    {
        std::ostringstream transcode;
        transcode << "#transcode{"
                  << "vcodec=h264,scale=Auto,fps=25,width=768,height=576,";

        // Workaround for ticket https://trac.videolan.org/vlc/ticket/10148
        if (instance::compare_version(2, 1) != 0)
            transcode << "vfilter=canvas{width=768,height=576},soverlay,";

        transcode << "acodec=a52,samplerate=44100,channels=2"
                  << "}";

        transcode_base(transcode.str(), "ts");
    }
} transcode_stream_test;

} // End of namespace
