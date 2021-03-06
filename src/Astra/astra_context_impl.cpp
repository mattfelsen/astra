#include "astra_context_impl.hpp"
#include <Astra/astra_capi.h>
#include <Astra/Plugins/plugin_capi.h>
#include <AstraAPI.h>
#include "astra_stream_reader.hpp"
#include "astra_stream_connection.hpp"
#include "astra_streamset_connection.hpp"
#include "astra_logging.hpp"
#include "astra_environment.hpp"
#include "astra_private.h"
#include "astra_configuration.hpp"
#include "astra_filesystem.hpp"
#include "astra_cxx_compatibility.hpp"

INITIALIZE_LOGGING

namespace astra {

    astra_status_t context_impl::initialize()
    {
        if (m_initialized)
            return ASTRA_STATUS_SUCCESS;

#if __ANDROID__
        std::string appPath = environment::application_path();
        std::string logPath = filesystem::combine_paths(appPath, "astra.log");
        std::string configPath = filesystem::combine_paths(appPath, "astra.toml");
#else
        std::string logPath = "astra.log";
        std::string configPath = filesystem::combine_paths(environment::lib_path(), "astra.toml");
#endif

        std::unique_ptr<configuration> config(configuration::load_from_file(configPath.c_str()));
        initialize_logging(logPath.c_str(), config->severityLevel());

        LOG_WARN("context", "Hold on to yer butts");
        LOG_INFO("context", "configuration path: %s", configPath.c_str());
        LOG_INFO("context", "log file path: %s", logPath.c_str());

        pluginManager_ = std::make_unique<plugin_manager>(m_setCatalog);

#if !__ANDROID__
        std::string pluginsPath = filesystem::combine_paths(environment::lib_path(),
                                                            filesystem::append_path_separator(config->pluginsPath()));

        LOG_INFO("context", "plugin path: %s", pluginsPath.c_str());

        pluginManager_->load_plugins(pluginsPath);
#else
        pluginManager_->load_plugin("libopenni_sensor.so");
        pluginManager_->load_plugin("liborbbec_hand.so");
        pluginManager_->load_plugin("liborbbec_xs.so");
#endif

        if (pluginManager_->plugin_count() == 0)
        {
            LOG_WARN("context", "Astra found no plugins. Is there a Plugins folder? Is the working directory correct?");
        }

        m_initialized = true;

        return ASTRA_STATUS_SUCCESS;
    }

    astra_status_t context_impl::terminate()
    {
        if (!m_initialized)
            return ASTRA_STATUS_UNINITIALIZED;

        pluginManager_.reset();
        m_setCatalog.clear();

        m_initialized = false;

        LOG_INFO("context", "Astra terminated.");

        return ASTRA_STATUS_SUCCESS;
    }

    astra_status_t context_impl::streamset_open(const char* uri, astra_streamsetconnection_t& streamSet)
    {
        LOG_INFO("context", "client opening streamset: %s", uri);

        streamset_connection& conn = m_setCatalog.open_set_connection(uri);
        streamSet = conn.get_handle();

        return ASTRA_STATUS_SUCCESS;
    }

    astra_status_t context_impl::streamset_close(astra_streamsetconnection_t& streamSet)
    {
        if (!m_initialized)
        {
            streamSet = nullptr;
            return ASTRA_STATUS_SUCCESS;
        }

        streamset_connection* actualConnection = streamset_connection::get_ptr(streamSet);

        if (actualConnection)
        {
            m_setCatalog.close_set_connection(actualConnection);
        }
        else
        {
            LOG_WARN("context", "attempt to close a non-existent stream set");
        }

        streamSet = nullptr;

        return actualConnection != nullptr ? ASTRA_STATUS_SUCCESS : ASTRA_STATUS_INVALID_PARAMETER;
    }

    // char* context_impl::get_status_string(astra_status_t status)
    // {
    //     //TODO
    //     return nullptr;
    // }

    astra_status_t context_impl::reader_create(astra_streamsetconnection_t streamSet,
                                                   astra_reader_t& reader)
    {
        assert(streamSet != nullptr);

        streamset_connection* actualConnection = streamset_connection::get_ptr(streamSet);

        if (actualConnection)
        {
            stream_reader* actualReader = actualConnection->create_reader();
            m_activeReaders.push_back(actualReader);

            reader = actualReader->get_handle();

            return ASTRA_STATUS_SUCCESS;
        }
        else
        {
            LOG_WARN("context", "attempt to create reader from non-existent stream set");
            return ASTRA_STATUS_INVALID_PARAMETER;
        }
    }

    astra_status_t context_impl::reader_destroy(astra_reader_t& reader)
    {
        assert(reader != nullptr);

        stream_reader* actualReader = stream_reader::get_ptr(reader);

        if (actualReader)
        {
            streamset_connection& connection = actualReader->get_connection();
            connection.destroy_reader(actualReader);
        }
        else
        {
            LOG_WARN("context", "attempt to destroy a non-existent reader: %p", reader);
        }

        reader = nullptr;

        return ASTRA_STATUS_SUCCESS;
    }

    astra_status_t context_impl::reader_get_stream(astra_reader_t reader,
                                                       astra_stream_type_t type,
                                                       astra_stream_subtype_t subtype,
                                                       astra_streamconnection_t& connection)
    {
        assert(reader != nullptr);

        stream_reader* actualReader = stream_reader::get_ptr(reader);

        if (actualReader)
        {
            astra_stream_desc_t desc;
            desc.type = type;
            desc.subtype = subtype;

            connection = actualReader->get_stream(desc)->get_handle();
        }
        else
        {
            LOG_WARN("context", "get_stream called on non-existent reader");
            connection = nullptr;
        }

        return ASTRA_STATUS_SUCCESS;
    }

    astra_status_t context_impl::stream_get_description(astra_streamconnection_t connection,
                                                            astra_stream_desc_t* description)
    {
        stream_connection* actualConnection = stream_connection::get_ptr(connection);

        if (actualConnection)
        {
            *description = actualConnection->get_description();
        }
        else
        {
            LOG_WARN("context", "get_description called on non-existent stream");
        }

        return ASTRA_STATUS_SUCCESS;
    }

    astra_status_t context_impl::stream_start(astra_streamconnection_t connection)
    {
        assert(connection != nullptr);
        assert(connection->handle != nullptr);

        stream_connection* actualConnection = stream_connection::get_ptr(connection);

        if (actualConnection)
        {
            actualConnection->start();
        }
        else
        {
            LOG_WARN("context", "start called on non-existent stream");
        }

        return ASTRA_STATUS_SUCCESS;
    }

    astra_status_t context_impl::stream_stop(astra_streamconnection_t connection)
    {
        assert(connection != nullptr);
        assert(connection->handle != nullptr);

        stream_connection* actualConnection = stream_connection::get_ptr(connection);

        if (actualConnection)
        {
            actualConnection->stop();
        }
        else
        {
            LOG_WARN("context", "stop called on non-existent stream");
        }

        return ASTRA_STATUS_SUCCESS;
    }

    astra_status_t context_impl::reader_open_frame(astra_reader_t reader,
                                                       int timeoutMillis,
                                                       astra_reader_frame_t& frame)
    {
        if (reader == nullptr)
        {
            LOG_WARN("context", "reader_open_frame called with null reader");
            assert(reader != nullptr);
            return ASTRA_STATUS_INVALID_OPERATION;
        }

        stream_reader* actualReader = stream_reader::get_ptr(reader);

        if (actualReader)
        {
            return actualReader->lock(timeoutMillis, frame);
        }
        else
        {
            LOG_WARN("context", "open_frame called on non-existent reader");
            return ASTRA_STATUS_INVALID_PARAMETER;
        }
    }

    astra_status_t context_impl::reader_close_frame(astra_reader_frame_t& frame)
    {
        if (frame == nullptr)
        {
            LOG_WARN("context", "reader_close_frame called with null frame");
            assert(frame != nullptr);
            return ASTRA_STATUS_INVALID_OPERATION;
        }

        stream_reader* actualReader = stream_reader::from_frame(frame);

        if (!actualReader)
        {
            LOG_WARN("context", "reader_close_frame couldn't retrieve stream_reader from frame");
            assert(actualReader != nullptr);
            return ASTRA_STATUS_INTERNAL_ERROR;
        }

        return actualReader->unlock(frame);
    }

    astra_status_t context_impl::reader_register_frame_ready_callback(astra_reader_t reader,
                                                                          astra_frame_ready_callback_t callback,
                                                                          void* clientTag,
                                                                          astra_reader_callback_id_t& callbackId)
    {
        assert(reader != nullptr);
        callbackId = nullptr;

        stream_reader* actualReader = stream_reader::get_ptr(reader);

        if (actualReader)
        {
            CallbackId cbId = actualReader->register_frame_ready_callback(callback, clientTag);

            astra_reader_callback_id_t cb = new _astra_reader_callback_id;
            callbackId = cb;

            cb->reader = reader;
            cb->callbackId = cbId;

            return ASTRA_STATUS_SUCCESS;
        }
        else
        {
            LOG_WARN("context", "register_frame_ready_callback called on non-existent reader");
            return ASTRA_STATUS_INVALID_PARAMETER;
        }
    }

    astra_status_t context_impl::reader_unregister_frame_ready_callback(astra_reader_callback_id_t& callbackId)
    {
        if (!m_initialized)
        {
            delete callbackId;
            callbackId = nullptr;
            return ASTRA_STATUS_SUCCESS;
        }

        astra_reader_callback_id_t cb = callbackId;
        assert(cb != nullptr);
        assert(cb->reader != nullptr);

        CallbackId cbId = cb->callbackId;
        stream_reader* actualReader = stream_reader::get_ptr(cb->reader);

        if (actualReader)
        {
            actualReader->unregister_frame_ready_callback(cbId);
        }
        else
        {
            LOG_WARN("context", "unregister_frame_ready_callback for non-existent reader: %p", cb->reader);
        }

        delete cb;
        callbackId = nullptr;

        return actualReader != nullptr ? ASTRA_STATUS_SUCCESS : ASTRA_STATUS_INVALID_PARAMETER;
    }

    astra_status_t context_impl::reader_get_frame(astra_reader_frame_t frame,
                                                      astra_stream_type_t type,
                                                      astra_stream_subtype_t subtype,
                                                      astra_frame_t*& subFrame)
    {
        assert(frame != nullptr);

        stream_reader* actualReader = stream_reader::from_frame(frame);

        if (actualReader)
        {
            astra_stream_desc_t desc;
            desc.type = type;
            desc.subtype = subtype;

            subFrame = actualReader->get_subframe(desc);

            return ASTRA_STATUS_SUCCESS;
        }
        else
        {
            LOG_WARN("context", "get_frame called on non-existent reader/frame combo");
            return ASTRA_STATUS_INVALID_PARAMETER;
        }
    }

    astra_status_t context_impl::temp_update()
    {
        pluginManager_->update();

        return ASTRA_STATUS_SUCCESS;
    }

    astra_status_t context_impl::stream_set_parameter(astra_streamconnection_t connection,
                                                          astra_parameter_id parameterId,
                                                          size_t inByteLength,
                                                          astra_parameter_data_t inData)
    {
        assert(connection != nullptr);
        assert(connection->handle != nullptr);

        stream_connection* actualConnection = stream_connection::get_ptr(connection);

        if (actualConnection)
        {
            actualConnection->set_parameter(parameterId, inByteLength, inData);
            return ASTRA_STATUS_SUCCESS;
        }
        else
        {
            LOG_WARN("context", "set_parameter called on non-existent stream");
            return ASTRA_STATUS_INVALID_PARAMETER;
        }
    }

    astra_status_t context_impl::stream_get_parameter(astra_streamconnection_t connection,
                                                          astra_parameter_id parameterId,
                                                          size_t& resultByteLength,
                                                          astra_result_token_t& token)
    {
        assert(connection != nullptr);
        assert(connection->handle != nullptr);

        stream_connection* actualConnection = stream_connection::get_ptr(connection);

        if (actualConnection)
        {
            actualConnection->get_parameter(parameterId, resultByteLength, token);
            return ASTRA_STATUS_SUCCESS;
        }
        else
        {
            LOG_WARN("context", "get_parameter called on non-existent stream");
            return ASTRA_STATUS_INVALID_PARAMETER;
        }
    }

    astra_status_t context_impl::stream_get_result(astra_streamconnection_t connection,
                                                       astra_result_token_t token,
                                                       size_t dataByteLength,
                                                       astra_parameter_data_t dataDestination)
    {
        assert(connection != nullptr);
        assert(connection->handle != nullptr);

        stream_connection* actualConnection = stream_connection::get_ptr(connection);
        if (actualConnection)
        {
            return actualConnection->get_result(token, dataByteLength, dataDestination);
        }
        else
        {
            LOG_WARN("context", "get_result called on non-existent stream");
            return ASTRA_STATUS_INVALID_PARAMETER;
        }
    }

    astra_status_t context_impl::stream_invoke(astra_streamconnection_t connection,
                                                   astra_command_id commandId,
                                                   size_t inByteLength,
                                                   astra_parameter_data_t inData,
                                                   size_t& resultByteLength,
                                                   astra_result_token_t& token)
    {
        assert(connection != nullptr);
        assert(connection->handle != nullptr);

        stream_connection* actualConnection = stream_connection::get_ptr(connection);

        if (actualConnection)
        {
            actualConnection->invoke(commandId, inByteLength, inData, resultByteLength, token);
            return ASTRA_STATUS_SUCCESS;
        }
        else
        {
            LOG_WARN("context", "invoke called on non-existent stream");
            return ASTRA_STATUS_INVALID_PARAMETER;
        }
    }

    astra_status_t context_impl::notify_host_event(astra_event_id id, const void* data, size_t dataSize)
    {
        pluginManager_->notify_host_event(id, data, dataSize);
        return ASTRA_STATUS_SUCCESS;
    }
}
