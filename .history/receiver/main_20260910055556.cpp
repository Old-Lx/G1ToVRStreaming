#include "emitter.h"
#include <iostream>

emitter::emitter(const std::string& ip, const std::string& p, const std::string& dev)
    : ip_destino(ip), puerto(p), dispositivo(dev), 
      pipeline(nullptr), main_loop(nullptr), en_ejecucion(false) {
    
    // Inicia GStreamer de forma segura si no se ha hecho globalmente
    // Esto reserva la memoria base para los plugins H.264
    if (!gst_is_initialized()) {
        gst_init(nullptr, nullptr);
    }
}

emitter::~emitter() {
    Detener(); // Garantiza la liberación del puerto USB /dev/videoX al destruir el objeto
}

bool emitter::Iniciar() {
    if (en_ejecucion) return true;

    // Clon exacto del comando de terminal que funcionó, más el seguro I420
    std::string pipeline_str = 
        "v4l2src device=" + dispositivo + " ! "
        "video/x-raw,format=YUY2,width=640,height=480,framerate=30/1 ! "
        "videoconvert ! "
        "video/x-raw,format=I420 ! " // Seguro de memoria: Fuerza el formato exacto para x264enc
        "x264enc tune=zerolatency speed-preset=ultrafast ! "
        "rtph264pay pt=96 ! "
        "udpsink host=" + ip_destino + " port=" + puerto;

    GError *error = nullptr;
    pipeline = gst_parse_launch(pipeline_str.c_str(), &error);

    if (error) {
        std::cerr << "[emitter] Error al construir pipeline: " << error->message << std::endl;
        g_clear_error(&error);
        return false;
    }

    main_loop = g_main_loop_new(nullptr, FALSE);
    
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "[emitter] Fallo crítico al abrir " << dispositivo << std::endl;
        gst_object_unref(pipeline);
        pipeline = nullptr;
        g_main_loop_unref(main_loop);
        main_loop = nullptr;
        return false;
    }

    en_ejecucion = true;
    hilo_gstreamer = std::thread(&emitter::EjecutarBucle, this);
    
    std::cout << "[emitter] Transmisión iniciada idéntica a terminal hacia " << ip_destino << ":" << puerto << std::endl;
    return true;
}

void emitter::EjecutarBucle() {
    // Este código corre en paralelo. Se bloquea aquí procesando la cámara, 
    // pero permite que tu main.cpp siga calculando algoritmos del tráiler de forma ininterrumpida.
    g_main_loop_run(main_loop);
}

void emitter::Detener() {
    if (!en_ejecucion) return;

    std::cout << "[emitter] Deteniendo hilo GStreamer y apagando sensor..." << std::endl;
    
    if (main_loop) {
        // Enviar señal de apagado al bucle para que EjecutarBucle() finalice
        g_main_loop_quit(main_loop); 
    }
    
    // Esperar de forma segura a que el hilo secundario termine de morir
    if (hilo_gstreamer.joinable()) {
        hilo_gstreamer.join(); 
    }

    if (pipeline) {
        // Cambiar a estado NULL corta la energía/datos del puerto USB
        gst_element_set_state(pipeline, GST_STATE_NULL); 
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }

    if (main_loop) {
        g_main_loop_unref(main_loop);
        main_loop = nullptr;
    }

    en_ejecucion = false;
    std::cout << "[emitter] Hardware liberado correctamente." << std::endl;
}

bool emitter::EstaCorriendo() const {
    return en_ejecucion;
}