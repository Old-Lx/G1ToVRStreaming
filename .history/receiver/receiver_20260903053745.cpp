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

    // Pipeline de decodificación. autovideosink abrirá una ventana en Debian.
    // sync=false es crucial para evitar buffering y lograr "zerolatency".
    std::string pipeline_str = 
        "udpsrc port=" + puerto_escucha + " ! "
        "application/x-rtp, encoding-name=H264, payload=96 ! "
        "rtph264depay ! "
        "avdec_h264 ! "
        "videoconvert ! "
        "autovideosink sync=false";

    GError *error = nullptr;
    pipeline = gst_parse_launch(pipeline_str.c_str(), &error);

    if (error) {
        std::cerr << "[receiver] Error: " << error->message << std::endl;
        g_clear_error(&error);
        return false;
    }

    main_loop = g_main_loop_new(nullptr, FALSE);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    en_ejecucion = true;

    hilo_gstreamer = std::thread(&receiver::EjecutarBucle, this);
    
    std::cout << "[receiver] Escuchando video en el puerto UDP " << puerto_escucha << "..." << std::endl;
    return true;
}

void receiver::EjecutarBucle() {
    g_main_loop_run(main_loop);
}

void receiver::Detener() {
    if (!en_ejecucion) return;
    std::cout << "[receiver] Cerrando visor de video..." << std::endl;
    
    if (main_loop) g_main_loop_quit(main_loop);
    if (hilo_gstreamer.joinable()) hilo_gstreamer.join();

    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }
    if (main_loop) {
        g_main_loop_unref(main_loop);
        main_loop = nullptr;
    }
    en_ejecucion = false;
}

bool receiver::EstaCorriendo() const { return en_ejecucion; }