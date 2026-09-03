#include "emitter.h"
#include <iostream>
#include <csignal>
#include <unistd.h> 

emitter* stream_video = nullptr;

void InterceptarCtrlC(int signum) {
    if (stream_video) {
        stream_video->Detener();
    }
    exit(signum);
}

int main() {
    std::signal(SIGINT, InterceptarCtrlC);

    // Instanciar el módulo de video con el nuevo nombre de clase
    stream_video = new emitter("127.0.0.1", "5000");

    // Iniciar transmisión sin bloquear el programa
    if (!stream_video->Iniciar()) {
        return -1;
    }

    // --- BUCLE PRINCIPAL DEL ROBOT ---
    std::cout << "[Robot] Ejecutando rutinas de control de movimiento..." << std::endl;
    while (true) {
        std::cout << "[Robot] Calculando trayectorias..." << std::endl;
        sleep(2); // Simula el ciclo del controlador
    }

    delete stream_video;
    return 0;
}