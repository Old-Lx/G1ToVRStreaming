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

    std::cout << "[Estacion Base] Listo. Esperando flujo de video..." << std::endl;
    while (true) {
        sleep(1); 
    }

    delete visor_video;
    return 0;
}