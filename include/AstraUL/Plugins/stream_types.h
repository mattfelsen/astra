#ifndef STREAM_TYPES_H
#define STREAM_TYPES_H

#include <AstraUL/streams/depth_types.h>
#include <AstraUL/streams/color_types.h>
#include <AstraUL/streams/hand_types.h>
#include <AstraUL/streams/image_types.h>

// https://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html
// http://stackoverflow.com/questions/3350852/how-to-correctly-fix-zero-sized-array-in-struct-union-warning-c4200-without
#if ! defined(__ANDROID__) && (defined(__GNUC__) || defined(__clang__))
#define PACK_STRUCT __attribute__((packed))
#else
#define PACK_STRUCT
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

struct _astra_imageframe {
    union {
        astra_frame_t* frame;
        uint64_t pad0;
    };

    astra_image_metadata_t metadata;
    union {
        void* data;
        uint64_t pad1;
    };

} PACK_STRUCT;

struct _astra_handframe {
    astra_frame_t* frame;
    size_t handCount;
    astra_handpoint_t* handpoints;
} PACK_STRUCT;

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4200 )
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc99-extensions"
#endif

typedef struct _astra_imageframe_wrapper {
    _astra_imageframe frame;
    char frame_data[];
} PACK_STRUCT astra_imageframe_wrapper_t;

typedef struct _astra_handframe_wrapper {
    _astra_handframe frame;
    char frame_data[];
} PACK_STRUCT astra_handframe_wrapper_t;

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif /* STREAM_TYPES_H */
