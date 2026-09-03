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
    
    // Función estática que escucha los eventos de GStreamer (como el cierre de ventana)
    static gboolean EscucharBus(GstBus* bus, GstMessage* msg, gpointer data);

public:
    receiver(const std::string& puerto = "5000");
    ~receiver();

    bool Iniciar();
    void Detener();
    bool EstaCorriendo() const;
};