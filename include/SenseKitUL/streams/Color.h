#ifndef COLOR_H
#define COLOR_H

#include <SenseKit/SenseKit.h>
#include <SenseKitUL/StreamTypes.h>
#include <SenseKitUL/streams/color_capi.h>
#include <SenseKitUL/streams/Image.h>

namespace sensekit {

    class ColorStream : public DataStream
    {
    public:
        ColorStream()
        {}

        explicit ColorStream(sensekit_streamconnection_t connection)
            : DataStream(connection)
        {
            m_colorStream = reinterpret_cast<sensekit_colorstream_t>(connection);
        }

        static const sensekit_stream_type_t id = SENSEKIT_STREAM_COLOR;

        float get_horizontalFieldOfView()
        {
            float hFov;
            sensekit_color_stream_get_hfov(m_colorStream, &hFov);

            return hFov;
        }

        float get_verticalFieldOfView()
        {
            float vFov;
            sensekit_color_stream_get_vfov(m_colorStream, &vFov);

            return vFov;
        }

    private:
        sensekit_colorstream_t m_colorStream;
    };

    class ColorFrame : public ImageFrame<RGBPixel>
    {
    public:
        ColorFrame(sensekit_reader_frame_t readerFrame, sensekit_stream_subtype_t subtype)
            : ImageFrame(readerFrame, SENSEKIT_STREAM_COLOR, subtype)
        { }
    };

    class ColorFrameRaw : public ImageFrame<uint8_t>
    {
    public:
        ColorFrameRaw(sensekit_reader_frame_t readerFrame, sensekit_stream_subtype_t subtype)
            : ImageFrame(readerFrame, SENSEKIT_STREAM_COLOR, subtype)
        { }
    };
}

#endif // COLOR_H