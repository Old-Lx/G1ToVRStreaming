#pragma once
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>

class receiver {
private:
    GstElement* pipeline;
    GstElement* appsink;
    GMainLoop* main_loop;
    std::thread hilo_gstreamer;
    std::atomic<bool> en_ejecucion;
    std::string puerto_escucha;

    // Buffer crudo y candado de seguridad para los hilos
    std::vector<uint8_t> frame_buffer;
    std::mutex mutex_buffer;
    uint64_t contador_frames = 0;

    void EjecutarBucle();
    static gboolean EscucharBus(GstBus* bus, GstMessage* msg, gpointer data);
    
    // Callback que se dispara cada vez que llega un frame nuevo
    static GstFlowReturn OnNewSample(GstElement* sink, gpointer data);

public:
    receiver(const std::string& puerto = "5000");
    ~receiver();

    bool Iniciar();
    void Detener();
    bool EstaCorriendo() const;
};