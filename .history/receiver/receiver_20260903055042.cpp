#include "receiver.h"
#include <iostream>

receiver::receiver(const std::string& puerto)
    : puerto_escucha(puerto), pipeline(nullptr), main_loop(nullptr), en_ejecucion(false) {
    if (!gst_is_initialized()) {
        gst_init(nullptr, nullptr);
    }
}

receiver::~receiver() {
    Detener();
}

bool receiver::Iniciar() {
    if (en_ejecucion) return true;

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

    // --- NUEVO: Enlazar el vigilante al bus del pipeline ---
    GstBus* bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, EscucharBus, this);
    gst_object_unref(bus);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    en_ejecucion = true;

    hilo_gstreamer = std::thread(&receiver::EjecutarBucle, this);
    
    std::cout << "[receiver] Escuchando video en el puerto " << puerto_escucha << std::endl;
    return true;
}

void receiver::EjecutarBucle() {
    g_main_loop_run(main_loop);
    en_ejecucion = false; // Se marca como falso en el instante que el bucle cae
}

// --- NUEVO: Intercepción de mensajes del Bus ---
gboolean receiver::EscucharBus(GstBus* bus, GstMessage* msg, gpointer data) {
    receiver* receptor = static_cast<receiver*>(data);
    
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR:
        case GST_MESSAGE_EOS:
            // Si el visor de video se cierra a la fuerza, entra por aquí
            if (g_main_loop_is_running(receptor->main_loop)) {
                std::cout << "\n[receiver] Ventana cerrada o flujo cortado. Deteniendo..." << std::endl;
                g_main_loop_quit(receptor->main_loop);
            }
            break;
        default:
            break;
    }
    return TRUE;
}

void receiver::Detener() {
    if (!pipeline) return; // Evita la doble liberación si el destructor entra luego de Ctrl+C

    if (main_loop && g_main_loop_is_running(main_loop)) {
        g_main_loop_quit(main_loop);
    }
    
    if (hilo_gstreamer.joinable()) {
        hilo_gstreamer.join();
    }

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(main_loop);
    
    pipeline = nullptr;
    main_loop = nullptr;
    en_ejecucion = false;
}

bool receiver::EstaCorriendo() const { return en_ejecucion; }