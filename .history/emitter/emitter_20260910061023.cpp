#include "emitter.h"
#include <iostream>

emitter::emitter(const std::string& ip, const std::string& p, const std::string& dev)
    : ip_destino(ip), puerto(p), dispositivo(dev), 
      pipeline(nullptr), main_loop(nullptr), en_ejecucion(false) {
    
    // Inicia GStreamer de forma segura si no se ha hecho globalmente
    if (!gst_is_initialized()) {
        gst_init(nullptr, nullptr);
    }
}

emitter::~emitter() {
    Detener(); // Garantiza la liberación de /dev/videoX al destruir el objeto
}

bool emitter::Iniciar() {
    if (en_ejecucion) return true;

    // Pipeline estricto: Reloj forzado, negociación YUY2 y modo asíncrono
    std::string pipeline_str = 
        "v4l2src device=" + dispositivo + " do-timestamp=true ! "
        "video/x-raw,format=YUY2,width=640,height=480,framerate=30/1 ! "
        "videoconvert ! "
        "video/x-raw,format=I420 ! "
        "x264enc tune=zerolatency speed-preset=ultrafast ! "
        "rtph264pay pt=96 ! "
        "udpsink host=" + ip_destino + " port=" + puerto + " sync=false async=false";

    GError *error = nullptr;
    pipeline = gst_parse_launch(pipeline_str.c_str(), &error);

    if (error) {
        std::cerr << "[emitter] Error: " << error->message << std::endl;
        g_clear_error(&error);
        return false;
    }

    main_loop = g_main_loop_new(nullptr, FALSE);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    en_ejecucion = true;

    // Desacopla GStreamer: Lanza el bucle en un hilo separado
    hilo_gstreamer = std::thread(&emitter::EjecutarBucle, this);
    
    std::cout << "[emitter] Transmisión H.264 (Reloj Forzado) hacia " << ip_destino << ":" << puerto << std::endl;
    return true;
}

void emitter::EjecutarBucle() {
    // Este código corre en paralelo. Se bloquea aquí, pero no afecta al controlador.
    g_main_loop_run(main_loop);
}

void emitter::Detener() {
    if (!en_ejecucion) return;

    std::cout << "[emitter] Deteniendo hilo y liberando cámara..." << std::endl;
    
    if (main_loop) {
        g_main_loop_quit(main_loop); // Desbloquea g_main_loop_run
    }
    
    if (hilo_gstreamer.joinable()) {
        hilo_gstreamer.join(); // Espera de forma segura a que el hilo muera
    }

    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL); // Suelta el USB
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }

    if (main_loop) {
        g_main_loop_unref(main_loop);
        main_loop = nullptr;
    }

    en_ejecucion = false;
    std::cout << "[emitter] Hardware liberado." << std::endl;
}

bool emitter::EstaCorriendo() const {
    return en_ejecucion;
}