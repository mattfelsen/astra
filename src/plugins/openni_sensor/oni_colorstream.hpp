#ifndef ONI_COLORSTREAM_H
#define ONI_COLORSTREAM_H

#include "oni_devicestream.hpp"
#include <Astra/Plugins/plugin_capi.h>
#include <AstraUL/astraul_ctypes.h>
#include <AstraUL/Plugins/stream_types.h>

namespace orbbec { namespace ni {

    class colorstream : public devicestream<astra_imageframe_wrapper_t>
    {
    public:
        colorstream(astra::PluginServiceProxy& pluginService,
                    astra_streamset_t streamSet,
                    openni::Device& oniDevice,
                    stream_listener& listener);

        colorstream(const colorstream&) = delete;
        colorstream& operator=(const colorstream&) = delete;
    };
}}

#endif /* ONI_COLORSTREAM_H */
