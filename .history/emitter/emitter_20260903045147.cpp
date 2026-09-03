#include <gst/gst.h>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    // Inicialización del motor GStreamer
    gst_init(&argc, &argv);

    // Configuración de red: Ajustar a la IP de la estación receptora
    std::string host_ip = "127.0.0.1";
    std::string port = "5000";

    // Pipeline H.264 optimizado para retardo cero y bajo consumo de CPU
    std::string pipeline_str = 
        "v4l2src device=/dev/video0 ! "
        "videoconvert ! "
        "x264enc tune=zerolatency speed-preset=ultrafast ! "
        "rtph264pay ! "
        "udpsink host=" + host_ip + " port=" + port;

    GError *error = nullptr;
    GstElement *pipeline = gst_parse_launch(pipeline_str.c_str(), &error);

    if (error) {
        std::cerr << "Error al construir el pipeline: " << error->message << std::endl;
        g_clear_error(&error);
        return -1;
    }

    // Arrancar la transmisión de video
    std::cout << "Iniciando transmisión hacia " << host_ip << ":" << port << std::endl;
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Loop principal para mantener la ejecución
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);

    // Limpieza de memoria y liberación estricta de la cámara
    std::cout << "Deteniendo transmisión..." << std::endl;
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}