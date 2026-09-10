#pragma once
#include <gst/gst.h>
#include <string>
#include <thread>
#include <atomic>

class emitter {
private:
    GstElement* pipeline;
    GMainLoop* main_loop;
    std::thread hilo_gstreamer;
    std::atomic<bool> en_ejecucion;
    
    std::string ip_destino;
    std::string puerto;
    std::string dispositivo;

    // Método privado que vivirá exclusivamente en el hilo secundario
    // para no ahogar el hilo principal de control del robot.
    void EjecutarBucle();

public:
    // Constructor. Si no se pasa dispositivo, por defecto buscará video0
    emitter(const std::string& ip, const std::string& p, const std::string& dev = "/dev/video0");
    ~emitter();

    bool Iniciar();
    void Detener();
    bool EstaCorriendo() const;
};