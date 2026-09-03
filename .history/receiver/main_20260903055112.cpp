#include "receiver.h"
#include <iostream>
#include <csignal>
#include <unistd.h>

receiver* visor_video = nullptr;

void InterceptarCtrlC(int signum) {
    if (visor_video) visor_video->Detener();
    exit(signum);
}

int main() {
    std::signal(SIGINT, InterceptarCtrlC);

    visor_video = new receiver("5000");

    if (!visor_video->Iniciar()) return -1;

    std::cout << "[Estacion Base] Esperando. Cierra la ventana de video para salir..." << std::endl;
    
    // Mientras la variable atómica sea verdadera, el programa sigue vivo
    while (visor_video->EstaCorriendo()) {
        usleep(500000); // Duerme medio segundo para no consumir CPU
    }

    std::cout << "[Estacion Base] Cerrando programa principal..." << std::endl;
    delete visor_video;
    return 0;
}