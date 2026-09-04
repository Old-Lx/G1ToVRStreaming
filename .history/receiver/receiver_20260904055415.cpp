#include "receiver.h"
#include <iostream>

receiver::receiver(const std::string& puerto)
    : puerto_escucha(puerto), pipeline(nullptr), appsink(nullptr), main_loop(nullptr), en_ejecucion(false) {
    if (!gst_is_initialized()) {
        gst_init(nullptr, nullptr);
    }
}

receiver::~receiver() { Detener(); }

bool receiver::Iniciar() {
    if (en_ejecucion) return true;

    // Pipeline modificado: salida cruda en BGRA hacia la memoria (appsink)
    std::string pipeline_str = 
        "udpsrc port=" + puerto_escucha + " ! application/x-rtp, encoding-name=H264, payload=96 ! rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=false";
        /* 
        "udpsrc port=" + puerto_escucha + " ! "
        "application/x-rtp, encoding-name=H264, payload=96 ! "
        "rtph264depay ! avdec_h264 ! videoconvert ! "
        "video/x-raw,format=BGRA ! appsink name=vr_sink emit-signals=true max-buffers=1 drop=true sync=false"; 
        */

    GError *error = nullptr;
    pipeline = gst_parse_launch(pipeline_str.c_str(), &error);

    if (error) {
        std::cerr << "[receiver] Error: " << error->message << std::endl;
        g_clear_error(&error);
        return false;
    }

    // Extraer el appsink y conectar la señal del nuevo frame
    appsink = gst_bin_get_by_name(GST_BIN(pipeline), "vr_sink");
    g_signal_connect(appsink, "new-sample", G_CALLBACK(OnNewSample), this);
    gst_object_unref(appsink);

    main_loop = g_main_loop_new(nullptr, FALSE);
    GstBus* bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, EscucharBus, this);
    gst_object_unref(bus);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    en_ejecucion = true;
    hilo_gstreamer = std::thread(&receiver::EjecutarBucle, this);
    
    std::cout << "[receiver] Escuchando video. Extrayendo frames a memoria RAM..." << std::endl;
    return true;
}

void receiver::EjecutarBucle() {
    g_main_loop_run(main_loop);
    en_ejecucion = false;
}

// Extracción directa de los bytes de video
GstFlowReturn receiver::OnNewSample(GstElement* sink, gpointer data) {
    receiver* receptor = static_cast<receiver*>(data);
    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    
    if (sample) {
        GstBuffer* buffer = gst_sample_get_buffer(sample);
        GstMapInfo map;
        
        if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
            // Bloquear memoria, copiar los bytes y soltar
            std::lock_guard<std::mutex> lock(receptor->mutex_buffer);
            receptor->frame_buffer.assign(map.data, map.data + map.size);
            
            receptor->contador_frames++;
            std::cout << "\r[receiver] Frame " << receptor->contador_frames 
                      << " | Tamaño: " << map.size << " bytes   " << std::flush;
            
            gst_buffer_unmap(buffer, &map);
        }
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }
    return GST_FLOW_ERROR;
}

gboolean receiver::EscucharBus(GstBus* bus, GstMessage* msg, gpointer data) {
    receiver* receptor = static_cast<receiver*>(data);
    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR || GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
        if (g_main_loop_is_running(receptor->main_loop)) {
            g_main_loop_quit(receptor->main_loop);
        }
    }
    return TRUE;
}

void receiver::Detener() {
    if (!pipeline) return;
    if (main_loop && g_main_loop_is_running(main_loop)) g_main_loop_quit(main_loop);
    if (hilo_gstreamer.joinable()) hilo_gstreamer.join();
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(main_loop);
    pipeline = nullptr;
    main_loop = nullptr;
    en_ejecucion = false;
}
bool receiver::EstaCorriendo() const { return en_ejecucion; }