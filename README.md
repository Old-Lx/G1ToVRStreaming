# G1ToVRStreaming
This is an interface between Oculus Rift S and robot G1 Edu initially developed during a short exchanged in Krakow 


## Módulo Emisor de Telemetría Visual (C++)

Este módulo del repositorio contiene la implementación en C++ del cliente emisor que se ejecuta en el hardware del robot (Linux). Su función principal es capturar el flujo de video de los sensores físicos (Intel RealSense o cámaras genéricas), codificarlo en tiempo real y transmitirlo vía UDP hacia la estación base de Unreal Engine 5.

### ✨ Características Principales
* **Transmisión Multihilo (Non-blocking):** La ejecución del *pipeline* de transmisión (`GMainLoop`) está encapsulada en un `std::thread` independiente. Esto permite que el módulo se integre en sistemas como ROS 2 sin interrumpir ni bloquear el bucle principal de control y cinemática del robot.
* **Codificación H.264 de Ultra Baja Latencia:** Implementa un pipeline nativo de GStreamer parametrizado con `x264enc tune=zerolatency speed-preset=ultrafast`, optimizado para enviar paquetes RTP continuos sobre UDP sin almacenamiento en búfer (`sync=false async=false`).
* **Hardware Intercambiable:** Cuenta con un selector arquitectónico basado en `enum class TipoCamara`, permitiendo preconfigurar formatos de píxel específicos (ej. `YUY2` a 30fps para la Intel RealSense) o aplicar auto-negociación para webcams genéricas mediante V4L2.
* **Gestión Segura de Recursos (Graceful Shutdown):** Intercepta señales del sistema (como `SIGINT` vía Ctrl+C) para destruir el hilo de forma segura (`join`), detener el estado de GStreamer (`GST_STATE_NULL`) y liberar correctamente el nodo USB (`/dev/videoX`) del sistema operativo.

### 🏗️ Arquitectura del Sistema

La implementación se divide en tres componentes clave:

#### 1. Cabecera (Header - `emitter.h`)
* **Descripción:** Define la clase `emitter` y las enumeraciones de perfiles de hardware (`TipoCamara`). Protege los hilos con variables atómicas (`std::atomic<bool> en_ejecucion`) para asegurar lecturas seguras del estado de la transmisión.

#### 2. Lógica del Transmisor (`emitter.cpp`)
* **Inicialización:** Evalúa mediante `gst_is_initialized()` si el entorno de GStreamer ya fue levantado (crucial para no duplicar recursos en nodos de ROS 2).
* **Compilación de Pipeline:** Utiliza `gst_parse_launch` para generar la tubería de captura V4L2, conversión de color (`I420`), codificación H.264, empaquetado RTP (`rtph264pay`) y envío por socket UDP (`udpsink`).
* **Ejecución:** Invoca el `GMainLoop` dentro del método `EjecutarBucle()` que es ejecutado en el hilo paralelo.

#### 3. Integración Base (`main.cpp`)
* **Descripción:** Demuestra la implementación del objeto `emitter` en un flujo de trabajo de robótica. 
* **Configuración:** Permite inyectar los parámetros de red (IP de destino, puerto) y los descriptores de hardware (ej. `/dev/video6`) de forma modular. Se mantiene activo mediante un bucle infinito simulado, independiente de la cámara.

### ⚙️ Requisitos y Dependencias
* **Sistema Operativo:** Distribuciones Linux con soporte V4L2 (Video4Linux).
* **Compilador:** C++11 o superior (por `<thread>`, `<atomic>`).
* **Librerías del Sistema:** 
  * `libgstreamer1.0-dev`
  * `gstreamer1.0-plugins-base`, `good`, `bad`, `ugly` (para la codificación x264 y UDP).



## Visual Telemetry Emitter Module (C++)

This module of the repository contains the C++ implementation of the emitter client that runs on the robot's hardware (Linux). Its primary function is to capture the video stream from the physical sensors (Intel RealSense or generic cameras), encode it in real-time, and transmit it via UDP to the Unreal Engine 5 base station.

### ✨ Main Features
* **Multi-threaded Streaming (Non-blocking):** The transmission pipeline's execution (`GMainLoop`) is encapsulated within an independent `std::thread`. This allows the module to be seamlessly integrated into systems like ROS 2 without interrupting or blocking the robot's main control and kinematics loop.
* **Ultra-Low Latency H.264 Encoding:** Implements a native GStreamer pipeline parameterized with `x264enc tune=zerolatency speed-preset=ultrafast`, optimized for sending continuous RTP packets over UDP with zero buffering (`sync=false async=false`).
* **Hot-Swappable Hardware:** Features an architectural selector based on `enum class TipoCamara`, allowing pre-configuration of specific pixel formats (e.g., `YUY2` at 30fps for the Intel RealSense) or auto-negotiation for generic webcams via V4L2.
* **Safe Resource Management (Graceful Shutdown):** Intercepts system signals (such as `SIGINT` via Ctrl+C) to safely destroy the thread (`join`), stop the GStreamer state (`GST_STATE_NULL`), and correctly release the USB node (`/dev/videoX`) back to the operating system.

### 🏗️ System Architecture

The implementation is divided into three key components:

#### 1. Header (`emitter.h`)
* **Description:** Defines the `emitter` class and hardware profile enumerations (`TipoCamara`). Protects threads using atomic variables (`std::atomic<bool> en_ejecucion`) to ensure thread-safe reads of the transmission state.

#### 2. Transmitter Logic (`emitter.cpp`)
* **Initialization:** Evaluates via `gst_is_initialized()` whether the GStreamer environment has already been set up (crucial for avoiding resource duplication in ROS 2 nodes).
* **Pipeline Compilation:** Uses `gst_parse_launch` to generate the V4L2 capture pipeline, color conversion (`I420`), H.264 encoding, RTP payloading (`rtph264pay`), and UDP socket sinking (`udpsink`).
* **Execution:** Invokes the `GMainLoop` within the `EjecutarBucle()` method, which runs on the parallel thread.

#### 3. Base Integration (`main.cpp`)
* **Description:** Demonstrates the implementation of the `emitter` object within a robotics workflow.
* **Configuration:** Allows modular injection of network parameters (target IP, port) and hardware descriptors (e.g., `/dev/video6`). It is kept alive by a simulated infinite loop, completely independent of the camera thread.

### ⚙️ Requirements and Dependencies
* **Operating System:** Linux distributions with V4L2 (Video4Linux) support.
* **Compiler:** C++11 or higher (for `<thread>`, `<atomic>`).
* **System Libraries:** 
  * `libgstreamer1.0-dev`
  * `gstreamer1.0-plugins-base`, `good`, `bad`, `ugly` (for x264 encoding and UDP sink).
