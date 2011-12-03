/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2011
 * Martin Lambers <marlam@marlam.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string>
#include <list>

#include <gta/gta.hpp>

#include "msg.h"
#include "opt.h"
#include "str.h"
#include "fio.h"
#include "blob.h"

#include "lib.h"

#include "media_object.h"


extern "C" void gtatool_from_ffmpeg_help(void)
{
    msg::req_txt(
            "from-ffmpeg [-l|--list-streams] [-s|--stream=N] <input-file> [<output-file>]\n"
            "\n"
            "Converts video or audio data readable by FFmpeg to GTAs.\n"
            "When -l is given, list the streams available in the input file and quit.\n"
            "Select a stream to convert with -s. The default is to use the first stream.");
}

extern "C" int gtatool_from_ffmpeg(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::flag list_streams("list-streams", 'l', opt::optional);
    options.push_back(&list_streams);
    opt::val<int> stream("stream", 's', opt::optional, 1, std::numeric_limits<int>::max(), 1);
    options.push_back(&stream);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_from_ffmpeg_help();
        return 0;
    }

    media_object input(true);
    try
    {
        input.open(arguments[0], device_request());
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    if (list_streams.value())
    {
        for (int i = 0; i < input.video_streams(); i++)
        {
            msg::req("Stream %d: Video, %s, %g seconds",
                    i + 1,
                    input.video_frame_template(i).format_info().c_str(),
                    input.video_duration(i) / 1e6f);
        }
        for (int i = 0; i < input.audio_streams(); i++)
        {
            msg::req("Stream %d: Audio, %s, %g seconds",
                    i + 1 + input.video_streams(),
                    input.audio_blob_template(i).format_info().c_str(),
                    input.audio_duration(i) / 1e6f);
        }
        input.close();
        return 0;
    }

    if (stream.value() > input.video_streams() + input.audio_streams())
    {
        msg::err("%s contains no stream %d", arguments[0].c_str(), stream.value());
        return 1;
    }

    try
    {
        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, arguments[0]), arguments.size() == 2 ? arguments[1] : "");
        if (stream.value() - 1 < input.video_streams())
        {
            int s = stream.value() - 1;
            input.video_stream_set_active(s, true);
            input.start_video_frame_read(s);
            video_frame frame;
            while ((frame = input.finish_video_frame_read(s)).is_valid())
            {
                input.start_video_frame_read(s);
                gta::header hdr;
                std::string name;
                hdr.global_taglist().set("X-MILLISECONDS", str::from(frame.presentation_time / 1e3f).c_str());
                hdr.set_dimensions(frame.raw_width, frame.raw_height);
                hdr.set_components(gta::uint8, gta::uint8, gta::uint8);
                hdr.component_taglist(0).set("INTERPRETATION", "SRGB/RED");
                hdr.component_taglist(1).set("INTERPRETATION", "SRGB/GREEN");
                hdr.component_taglist(2).set("INTERPRETATION", "SRGB/BLUE");
                array_loop.write(hdr, name);
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, gta::header(), hdr);
                const uint32_t *src = static_cast<const uint32_t *>(frame.data[0][0]);
                uint8_t rgb[3];
                for (int y = 0; y < frame.raw_height; y++)
                {
                    for (int x = 0; x < frame.raw_width; x++)
                    {
                        uint32_t bgra = src[y * frame.line_size[0][0] / sizeof(uint32_t) + x];
                        rgb[0] = (bgra >> 16) & 0xff;
                        rgb[1] = (bgra >> 8) & 0xff;
                        rgb[2] = bgra & 0xff;
                        element_loop.write(rgb);
                    }
                }
            }
        }
        else
        {
            int s = stream.value() - input.video_streams() - 1;
            input.audio_stream_set_active(s, true);
            audio_blob ablob;
            gta::header hdr;
            std::string name;
            element_loop_t element_loop;

            hdr.set_dimensions(1);      // number of samples; will be corrected later
            std::vector<gta::type> types;
            switch (input.audio_blob_template(s).sample_format)
            {
            case audio_blob::u8:
                types.resize(input.audio_blob_template(s).channels, gta::uint8);
                break;
            case audio_blob::s16:
                types.resize(input.audio_blob_template(s).channels, gta::int16);
                break;
            case audio_blob::f32:
                types.resize(input.audio_blob_template(s).channels, gta::float32);
                break;
            case audio_blob::d64:
                types.resize(input.audio_blob_template(s).channels, gta::float64);
                break;
            }
            hdr.set_components(types.size(), &(types[0]));

            /* First, read all audio and store it in a temporary file, since we do not
             * know the exact number of audio samples in the stream; (rate * duration)
             * is just an estimate. */
            uintmax_t rate = input.audio_blob_template(s).rate;
            uintmax_t samples_estimate = rate * input.audio_duration(s) / 1000000;
            uintmax_t samples = 0;
            uintmax_t n = std::min(samples_estimate, rate);
            if (n < rate)
            {
                // read the last second worth of samples one at a time so that we do not miss any.
                n = 1;
            }
            input.start_audio_blob_read(s, n * hdr.element_size());
            FILE *tmpf = fio::tempfile();
            while (n > 0)
            {
                ablob = input.finish_audio_blob_read(s);
                if (!ablob.is_valid())
                {
                    break;      // end of stream
                }
                samples_estimate = (samples_estimate >= n) ? samples_estimate - n : 0;
                uintmax_t n_bak = n;
                n = std::min(samples_estimate, rate);
                if (n < rate)
                {
                    // read the last second worth of samples one at a time so that we do not miss any.
                    n = 1;
                }
                input.start_audio_blob_read(s, n * hdr.element_size());
                fio::write(ablob.data, hdr.element_size(), n_bak, tmpf);
                samples += n_bak;
            }
            fio::flush(tmpf);

            /* Now we know the exact number of samples. Write the complete data to the GTA. */
            hdr.set_dimensions(samples);
            hdr.dimension_taglist(0).set("INTERPRETATION", "T");
            hdr.dimension_taglist(0).set("X-SAMPLE-RATE",
                    str::from(input.audio_blob_template(s).rate).c_str());
            hdr.dimension_taglist(0).set("SAMPLE-DISTANCE",
                    (str::from(1.0 / input.audio_blob_template(s).rate) + " s").c_str());
            array_loop.write(hdr, name);
            fio::rewind(tmpf);
            array_loop.start_element_loop(element_loop, gta::header(), hdr);
            blob buf(10000 * hdr.element_size());
            while (samples > 0)
            {
                n = std::min(samples, static_cast<uintmax_t>(10000));
                fio::read(buf.ptr(), hdr.element_size(), n, tmpf);
                element_loop.write(buf.ptr(), n);
                samples -= n;
            }
            fio::close(tmpf);
        }
        array_loop.finish();
        input.close();
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
