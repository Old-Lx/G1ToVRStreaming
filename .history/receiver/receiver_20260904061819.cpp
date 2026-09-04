#include "receiver.h"
#include <iostream>

receiver::receiver(const std::string& puerto)
    : puerto_escucha(puerto), pipeline(nullptr), main_loop(nullptr), en_ejecucion(false) {
    if (!gst_is_initialized()) {
        gst_init(nullptr, nullptr);
    }
}

receiver::~receiver() { Detener(); }

bool receiver::Iniciar() {
    if (en_ejecucion) return true;

    // Pipeline estrictamente visual (sin appsink)
    std::string pipeline_str = 
        "udpsrc port=" + puerto_escucha + " ! "
        "application/x-rtp, encoding-name=H264, payload=96 ! "
        "rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=false";

    GError *error = nullptr;
    pipeline = gst_parse_launch(pipeline_str.c_str(), &error);

    if (error) {
        std::cerr << "[receiver] Error: " << error->message << std::endl;
        g_clear_error(&error);
        return false;
    }

    main_loop = g_main_loop_new(nullptr, FALSE);
    
    // Mantener el vigilante por si cierras la ventana manualmente
    GstBus* bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, EscucharBus, this);
    gst_object_unref(bus);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    en_ejecucion = true;
    hilo_gstreamer = std::thread(&receiver::EjecutarBucle, this);
    
    std::cout << "[receiver] Abriendo ventana de video de la RealSense..." << std::endl;
    return true;
}

void receiver::EjecutarBucle() {
    g_main_loop_run(main_loop);
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

// Puedes dejar OnNewSample vacío aquí si tu receiver.h aún lo declara, 
// para evitar errores de compilación, aunque ya no se usa en el pipeline visual.
GstFlowReturn receiver::OnNewSample(GstElement* sink, gpointer data) {
    return GST_FLOW_OK; 
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