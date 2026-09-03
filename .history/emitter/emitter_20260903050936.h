#pragma once
#include <gst/gst.h>
#include <string>
#include <thread>
#include <atomic>

class EmisorVR {
private:
    GstElement* pipeline;
    GMainLoop* main_loop;
    std::thread hilo_gstreamer;
    std::atomic<bool> en_ejecucion;
    
    std::string ip_destino;
    std::string puerto;
    std::string dispositivo;

    // Método privado que vivirá en el hilo secundario
    void EjecutarBucle();

public:
    EmisorVR(const std::string& ip, const std::string& p, const std::string& dev = "/dev/video0");
    ~EmisorVR();

    bool Iniciar();
    void Detener();
    bool EstaCorriendo() const;
};