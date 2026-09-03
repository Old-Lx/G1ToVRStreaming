#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <iostream>

// Callback que se ejecuta cada vez que el appsink recibe un frame
static GstFlowReturn on_new_sample(GstElement *sink, gpointer user_data) {
    GstSample *sample;
    // Extraer el frame del sumidero
    g_signal_emit_by_name(sink, "pull-sample", &sample);
    
    if (sample) {
        GstBuffer *buffer = gst_sample_get_buffer(sample);
        gsize size = gst_buffer_get_size(buffer);
        std::cout << "Frame recibido en memoria. Tamaño: " << size << " bytes." << std::endl;
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }
    return GST_FLOW_ERROR;
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    // Definimos el pipeline, terminando en el appsink
    const char* pipeline_str = 
        "udpsrc port=5000 ! application/x-rtp, encoding-name=H264, payload=96 ! "
        "rtph264depay ! avdec_h264 ! videoconvert ! "
        "appsink name=vr_sink emit-signals=true max-buffers=1 drop=true";

    GError *error = nullptr;
    GstElement *pipeline = gst_parse_launch(pipeline_str, &error);

    if (error) {
        std::cerr << "Error creating the pipeline: " << error->message << std::endl;
        g_clear_error(&error);
        return -1;
    }

    // Obtenemos el elemento appsink por su nombre y conectamos el callback
    GstElement *appsink = gst_bin_get_by_name(GST_BIN(pipeline), "vr_sink");
    g_signal_connect(appsink, "new-sample", G_CALLBACK(on_new_sample), nullptr);
    gst_object_unref(appsink);

    // Iniciamos la reproducción
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    std::cout << "Waiting frames for UDP in port 5000..." << std::endl;

    // Mantenemos el programa vivo
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);

    // Limpieza
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}