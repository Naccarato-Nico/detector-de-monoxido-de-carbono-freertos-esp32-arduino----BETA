# Sistema de Monitoreo de Monóxido de Carbono (CO)
### Implementación sobre ESP32 y Arquitectura FreeRTOS

Este proyecto detalla el diseño y la construcción de un sistema embebido de seguridad destinado a la detección de Monóxido de Carbono (CO). La solución tecnológica implementa un microcontrolador ESP32 y un sensor químico analógico MQ-7. Con el fin de garantizar la ejecución en tiempo real y evitar bloqueos en las rutinas críticas (como la activación inmediata de los sistemas de alerta), el firmware se estructuró de manera nativa bajo el sistema operativo FreeRTOS.

El entorno de desarrollo principal es PlatformIO (bajo VS Code) para asegurar un control estricto de las dependencias, manteniendo compatibilidad absoluta con Arduino IDE (copiando el código fuente en un archivo con extensión .ino). La lectura del sensor se realiza mapeando directamente el ADC interno del microcontrolador, prescindiendo de librerías de terceros para optimizar los tiempos de respuesta.

---

## 🛒 Especificación de Componentes

Para la réplica del sistema, se requiere el siguiente listado de materiales:
* 1x Microcontrolador ESP32 (Placa de desarrollo DevKit v1, 30 pines).
* 1x Módulo Sensor de Gas MQ-7 (Salida analógica).
* 1x Buzzer Pasivo de 5V.
* 1x LED Difuso de 5mm (Verde).
* 1x LED Difuso de 5mm (Rojo).
* 1x LED Difuso de 5mm (Azul).
* 3x Resistencias de película de carbón de 220 Ohms.
* 1x Pulsador de cuatro pines (Tact Switch).
* 1x Protoboard de 830 puntos.
* 1x Módulo adaptador de alimentación para protoboard (MB102).
* 1x Banco de energía portátil (PowerBank).
* 1x Cable USB Tipo C (junto a un adaptador USB-C hembra a USB macho para conformar un cable de extremos macho-macho).
* Conductores varios (Jumpers tipo Macho-Macho y Macho-Hembra).

---

## 🔌 Conexiones y Distribución de Pines

El mapa de terminales (GPIO) del microcontrolador quedó asignado de la siguiente manera:

* **Sensor MQ-7 (Señal analógica):** Conectado al pin GPIO 34 (Canal ADC del ESP32).
* **Pulsador de Testeo:** Configurado en el pin GPIO 12, empleando la resistencia interna de Pull-Up del integrado.
* **Indicadores Lumínicos (con acoplamiento de resistencias de 220 Ohms):**
  * GPIO 25 -> LED Verde (Estado del ambiente: Seguro / Normal).
  * GPIO 26 -> LED Rojo (Estado del ambiente: Alerta crítica por Gas).
  * GPIO 27 -> LED Azul (Estado del sistema: Modo de prueba activo).
* **Alerta Sonora:** Buzzer pasivo asociado al pin GPIO 14 mediante el periférico de modulación por ancho de pulsos (LEDC).

---

## 🔬 Análisis Técnico del Sensor MQ-7 y Alternativas

El MQ-7 es un dispositivo semiconductor diseñado específicamente para registrar Monóxido de Carbono (CO). Su composición química interna reacciona ante las moléculas de este gas invisible, inodoro y altamente tóxico, ofreciendo una variación de conductividad eléctrica proporcional a la concentración detectada.

### Sensores sustitutos por compatibilidad de hardware:
En caso de discontinuidad de stock del componente principal, se contemplan las siguientes alternativas de la serie MQ, considerando las variaciones en su curva de sensibilidad:
1. MQ-9: Es el reemplazo directo más eficiente. Comparte la lógica de funcionamiento del MQ-7, estando calibrado para detectar tanto Monóxido de Carbono como gases inflamables (Metano y GLP).
2. MQ-2: Aunque su sensibilidad principal está orientada a la detección de Gas Licuado de Petróleo (GLP), Propano y Humo, cuenta con una curva de respuesta secundaria al CO, siendo útil para entornos de seguridad general.
3. MQ-135: Diseñado para el análisis de la calidad del aire en interiores. Registra la presencia de Amoníaco, Benceno, Humo y bajas concentraciones de Monóxido de Carbono.

---

## 🧠 Distribución de Tareas en FreeRTOS

La arquitectura de software saca provecho de los dos núcleos de procesamiento del ESP32, aislando las tareas para evitar retardos en la respuesta del hardware:

* **Core 1 (Entradas y Lectura de Periféricos):** Gobierna de forma exclusiva la ejecución de la tarea del sensor (muestreo analógico periódico cada 500ms) y la tarea del pulsador (monitoreo del estado digital con filtrado de rebotes elétricos por software).
* **Core 0 (Salidas y Seguridad):** Ejecuta la tarea de la alarma. Actúa como una máquina de estados que evalúa las variables compartidas y determina de manera inmediata la frecuencia del buzzer y la activación de los LEDs.

---

## 🛠️ Bitácora de Hardware: Desafíos y Protocolos de Trabajo

Durante el desarrollo y testeo en el laboratorio, se presentaron desafíos asociados a la gestión de energía y la integridad del hardware. A continuación, se detallan las problemáticas y los procedimientos técnicos adoptados:

### 1. El ciclo de auto-apagado del PowerBank
Al utilizar un banco de energía portátil para dotar de autonomía al sistema, el prototipo sufría un corte total de suministro a los pocos segundos de iniciar. Esto se debe a que el consumo de corriente del ESP32 en estado pasivo se encuentra por debajo del umbral mínimo de carga de la batería inteligente, provocando que esta interprete que no hay ningún dispositivo conectado y proceda a su apagado automático. Para solucionarlo, se implementó una rutina por software que asegura picos de consumo mínimos y controlados, manteniendo el circuito del PowerBank en estado activo.

### 2. Estabilización de Voltaje y Unificación de Masas
El filamento térmico del MQ-7 genera consumos intermitentes de corriente. Alimentar el sensor directamente desde los pines de la placa de desarrollo generaba caídas de tensión generalizadas (Brownouts), provocando el reinicio del microcontrolador. La solución consistió en independizar las etapas de potencia utilizando un módulo adaptador para protoboard alimentado por la fuente externa. Para garantizar la fidelidad de los datos del ADC, se unificaron obligatoriamente las masas (GND) del ESP32 y de la fuente externa, eliminando el ruido térmico en las mediciones.

### 3. Protocolo Seguro para Programación y Testeo Autónomo
Para mitigar el riesgo de cortocircuitos por diferencias de potencial entre la computadora y la fuente externa, se estableció el siguiente orden de operaciones:

1. **Desconexión Preventiva:** Antes de conectar el ESP32 a la computadora, es mandatorio desconectar el pin VIN de la línea de la protoboard para aislar eléctricamente las fuentes de alimentación.
2. **Transferencia de Código:** Con el pin VIN libre, se procede a conectar el cable USB a la PC para compilar, transferir el firmware al ESP32 y realizar el debug del software mediante el monitor serie.
3. **Testeo Integral de Componentes (Hardware y Software):** Para probar el circuito completo sin depender de la PC, se debe desconectar el cable USB de programación, reestablecer la conexión del pin VIN a la línea común de la protoboard y acoplar el cable puente macho-macho desde el PowerBank hacia el adaptador de la protoboard. De este modo, el sistema opera de forma 100% autónoma.

---

## 📸 Registro Fotográfico del Desarrollo

| 1. Disposición de Componentes | 2. Etapa de Alimentación y Cables | 3. Ensayo de Laboratorio Activo |
| :---: | :---: | :---: |
| ![Montaje Inicial](foto1.jpg) | ![Circuito de Alimentación](foto2.jpg) | ![Prueba Final con Alerta](foto3.jpg) |
