#include "emitter.h"
#include <iostream>

emitter::emitter(const std::string& ip, const std::string& p, const std::string& dev, TipoCamara tipo)
    : ip_destino(ip), puerto(p), dispositivo(dev), tipo_hardware(tipo),
      pipeline(nullptr), main_loop(nullptr), en_ejecucion(false) {
    
    if (!gst_is_initialized()) {
        gst_init(nullptr, nullptr);
    }
}

emitter::~emitter() {
    Detener(); 
}

bool emitter::Iniciar() {
    if (en_ejecucion) return true;

    std::string pipeline_str;

    // Selector arquitectónico (Fácilmente mapeable a parámetros ROS 2)
    switch (tipo_hardware) {
        case TipoCamara::INTEL_REALSENSE:
            std::cout << "[emitter] Configuración: Intel RealSense (YUY2, Latencia Cero Estricta)" << std::endl;
            pipeline_str = 
                "v4l2src device=" + dispositivo + " do-timestamp=true ! "
                "video/x-raw,format=YUY2,width=640,height=480,framerate=30/1 ! "
                "videoconvert ! video/x-raw,format=I420 ! "
                "x264enc tune=zerolatency speed-preset=ultrafast ! "
                "rtph264pay pt=96 ! "
                "udpsink host=" + ip_destino + " port=" + puerto + " sync=false async=false";
            break;

        case TipoCamara::WEBCAM_GENERICA:
        default:
            std::cout << "[emitter] Configuración: Webcam Genérica (Auto-negociación)" << std::endl;
            pipeline_str = 
                "v4l2src device=" + dispositivo + " do-timestamp=true ! "
                "videoconvert ! video/x-raw,format=I420 ! "
                "x264enc tune=zerolatency speed-preset=ultrafast ! "
                "rtph264pay pt=96 ! "
                "udpsink host=" + ip_destino + " port=" + puerto + " sync=false async=false";
            break;
    }

    GError *error = nullptr;
    pipeline = gst_parse_launch(pipeline_str.c_str(), &error);

    if (error) {
        std::cerr << "[emitter] Error de compilación GStreamer: " << error->message << std::endl;
        g_clear_error(&error);
        return false;
    }

    main_loop = g_main_loop_new(nullptr, FALSE);
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "[emitter] Fallo crítico al encender el sensor en " << dispositivo << std::endl;
        gst_object_unref(pipeline);
        pipeline = nullptr;
        g_main_loop_unref(main_loop);
        main_loop = nullptr;
        return false;
    }

    en_ejecucion = true;
    hilo_gstreamer = std::thread(&emitter::EjecutarBucle, this);
    return true;
}

void emitter::EjecutarBucle() {
    g_main_loop_run(main_loop);
}

// ... [El resto de métodos (Detener, EstaCorriendo) quedan exactamente igual que antes] ...