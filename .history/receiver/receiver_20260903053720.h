#pragma once
#include <gst/gst.h>
#include <string>
#include <thread>
#include <atomic>

class receiver {
private:
    GstElement* pipeline;
    GMainLoop* main_loop;
    std::thread hilo_gstreamer;
    std::atomic<bool> en_ejecucion;
    
    std::string puerto_escucha;

    void EjecutarBucle();

public:
    receiver(const std::string& puerto = "5000");
    ~receiver();

    bool Iniciar();
    void Detener();
    bool EstaCorriendo() const;
};