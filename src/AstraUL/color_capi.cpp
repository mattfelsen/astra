#include <Astra/astra_types.h>
#include <AstraUL/astraul_ctypes.h>
#include "generic_stream_api.h"
#include <AstraUL/streams/color_types.h>
#include <AstraUL/Plugins/stream_types.h>
#include <string.h>
#include <cassert>
#include <AstraUL/streams/image_capi.h>
#include <AstraUL/streams/image_parameters.h>
#include <stdbool.h>

ASTRA_BEGIN_DECLS

ASTRA_API_EX astra_status_t astra_reader_get_colorstream(astra_reader_t reader,
                                                                  astra_colorstream_t* colorStream)

{
    return astra_reader_get_stream(reader,
                                      ASTRA_STREAM_COLOR,
                                      DEFAULT_SUBTYPE,
                                      colorStream);
}

ASTRA_API_EX astra_status_t astra_frame_get_colorframe(astra_reader_frame_t readerFrame,
                                                                astra_colorframe_t* colorFrame)
{
    return astra_reader_get_imageframe(readerFrame,
                                          ASTRA_STREAM_COLOR,
                                          DEFAULT_SUBTYPE,
                                          colorFrame);
}

ASTRA_API_EX astra_status_t astra_frame_get_colorframe_with_subtype(astra_reader_frame_t readerFrame,
                                                                             astra_stream_subtype_t subtype,
                                                                             astra_colorframe_t* colorFrame)
{
    return astra_reader_get_imageframe(readerFrame,
                                          ASTRA_STREAM_COLOR,
                                          subtype,
                                          colorFrame);
}

ASTRA_API_EX astra_status_t astra_colorframe_get_frameindex(astra_colorframe_t colorFrame,
                                                                     astra_frame_index_t* index)
{
    return astra_generic_frame_get_frameindex(colorFrame, index);
}

ASTRA_API_EX astra_status_t astra_colorframe_get_data_byte_length(astra_colorframe_t colorFrame,
                                                                           size_t* byteLength)
{
    return astra_imageframe_get_data_byte_length(colorFrame, byteLength);
}

ASTRA_API_EX astra_status_t astra_colorframe_get_data_ptr(astra_colorframe_t colorFrame,
                                                                   uint8_t** data,
                                                                   size_t* byteLength)
{
    void* voidData = nullptr;
    astra_imageframe_get_data_ptr(colorFrame, &voidData, byteLength);
    *data = static_cast<uint8_t*>(voidData);

    return ASTRA_STATUS_SUCCESS;
}

ASTRA_API_EX astra_status_t astra_colorframe_get_data_rgb_ptr(astra_colorframe_t colorFrame,
                                                              astra_rgb_pixel_t** data,
                                                              size_t* byteLength)
{
    void* voidData = nullptr;
    astra_imageframe_get_data_ptr(colorFrame, &voidData, byteLength);
    *data = static_cast<astra_rgb_pixel_t*>(voidData);

    return ASTRA_STATUS_SUCCESS;
}

ASTRA_API_EX astra_status_t astra_colorframe_copy_data(astra_colorframe_t colorFrame,
                                                                uint8_t* data)
{
    return astra_imageframe_copy_data(colorFrame, data);
}

ASTRA_API_EX astra_status_t astra_colorframe_get_metadata(astra_colorframe_t colorFrame,
                                                                   astra_image_metadata_t* metadata)
{
    return astra_imageframe_get_metadata(colorFrame, metadata);
}

ASTRA_END_DECLS
