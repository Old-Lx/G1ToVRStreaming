#pragma once
#include <gst/gst.h>
#include <string>
#include <thread>
#include <atomic>

// Definimos los perfiles de hardware soportados
enum class TipoCamara {
    WEBCAM_GENERICA,
    INTEL_REALSENSE
};

class emitter {
private:
    GstElement* pipeline;
    GMainLoop* main_loop;
    std::thread hilo_gstreamer;
    std::atomic<bool> en_ejecucion;
    
    std::string ip_destino;
    std::string puerto;
    std::string dispositivo;
    TipoCamara tipo_hardware; // Guardamos el perfil seleccionado

    void EjecutarBucle();

public:
    // El constructor ahora exige que le digas qué tipo de cámara es
    emitter(const std::string& ip, const std::string& p, const std::string& dev, TipoCamara tipo);
    ~emitter();

    bool Iniciar();
    void Detener();
    bool EstaCorriendo() const;
};