# Física Computacional - Repositorio General

Este repositorio contiene todos los proyectos y simulaciones desarrollados durante el curso, divididos en ejercicios obligatorios y voluntarios.

---

## 1. Ejercicios obligatorios

### Sistema Solar - Estructura del Repositorio

En esta carpeta he incluido todos los archivos correspondientes a mi simulación del Sistema Solar usando el algoritmo de Verlet, además del estudio de rendimiento computacional que he realizado comparando mi máquina local con el clúster. 

A continuación detallo para qué sirve cada archivo del repositorio:

#### Memoria del Proyecto
* **`Informe_SistemaSolar.ipynb`**: Es el documento central de la práctica (Jupyter Notebook). Aquí explico todo el desarrollo físico, el algoritmo empleado, las gráficas, las animaciones y mis conclusiones finales sobre el benchmarking. Recomiendo empezar la lectura por aquí.

#### Códigos Fuente (C++)
* **`sistema_solar.cpp`**: Es el código principal desarrollado en C++ donde resuelvo las ecuaciones del movimiento para los N cuerpos mediante el integrador de Verlet, generando los datos de posiciones y energías.
* **`benchmark_solar.cpp`**: Un código secundario que he adaptado específicamente para el test de rendimiento. Simula 10 cuerpos durante 10^7 pasos temporales sin realizar operaciones de escritura (I/O) en disco para poder medir la potencia bruta de la CPU sin cuellos de botella.

#### Scripts de Análisis (Python)
* **`graficar_benchmark_solar.py`**: Script que lee los resultados de los ficheros de texto y genera automáticamente la gráfica de barras comparando los tiempos de ejecución.
* *(Nota: También se incluyen los scripts encargados de leer los datos para generar las animaciones en GIF de las trayectorias).*

#### Archivos de Datos de Rendimiento (.txt)
* **`tiempo_local_O0.txt`** y **`tiempo_local_O3.txt`**: Archivos donde he guardado los tiempos de ejecución reales (en segundos) sacados en mi máquina local (VirtualBox), compilando sin optimizar (-O0) y con optimización máxima (-O3).
* **`tiempo_cluster_O0.txt`** y **`tiempo_cluster_O3.txt`**: Lo mismo, pero con los tiempos extraídos al ejecutar el benchmark en el nodo secuencial del Clúster JOEL.

#### Recursos Visuales Generados
* **`grafica_rendimiento_sistema_solar.png`**: La gráfica final que saca el script de Python mostrando el factor de aceleración (speedup) entre mi máquina y el clúster.
* Los demás archivos **`.gif`** o **`.png`** que hay en la carpeta son las animaciones e imágenes de las órbitas planetarias que he incrustado en el informe.

#### Archivos de Volcado (.dat)
* Todos los archivos **`.dat`** son simplemente los volcados de datos brutos que genera C++ (posiciones, velocidades, energías, etc.) y que luego uso con Python para dibujar los resultados.

---

### Modelo de Ising (Dinámica de Glauber) - Estructura del Repositorio

En esta carpeta he incluido todos los archivos correspondientes a mi simulación del Modelo de Ising bidimensional utilizando el algoritmo de Metrópolis bajo la dinámica de Glauber (*spin-flip*). Además, incorporo el estudio de rendimiento computacional y escalabilidad que he realizado comparando el comportamiento de mi máquina local frente a los nodos de cálculo del clúster.

A continuación detallo para qué sirve cada archivo del repositorio:

#### Memoria del Proyecto
* **`Informe_Ising.ipynb`**: Es el documento central de la práctica (Jupyter Notebook). En este archivo explico detalladamente el fundamento físico del modelo, la deducción del algoritmo de selección estocástica, la evolución de las magnitudes macroscópicas (energía, magnetización, calor específico y susceptibilidad magnética) en función de la temperatura, las animaciones de los dominios de espín y las conclusiones finales del benchmarking. Recomiendo empezar la lectura por aquí.

#### Códigos Fuente (C++)
* **`ising_glauber.cpp`**: Es el código principal desarrollado en C++ donde implemento el algoritmo de Metrópolis para una red cuadrada de espines con condiciones de contorno periódicas. El programa evoluciona el sistema usando la dinámica de Glauber (volteo de un único espín de manera local) para muestrear el espacio de fases a distintas temperaturas y exportar las medias termodinámicas.
* **`benchmark_ising.cpp`**: Un código secundario que he adaptado específicamente para el test de rendimiento. Simula la red fijando la temperatura en el punto crítico de Onsager ($T_c \approx 2.27$) para forzar al procesador a evaluar un flujo constante de aceptaciones y rechazos estocásticos, evitando que prediga los saltos condicionales (*branch prediction*). Realiza un volumen fijo de 2000 Pasos Monte Carlo por espín para distintos tamaños de red ($N = 16, 32, 64, 128$) sin operaciones de escritura en disco (I/O) para medir la potencia bruta de cálculo.

#### Scripts de Análisis (Python)
* **`plot_rendimiento_ising.py`**: Script en Python encargado de procesar y leer de forma automatizada los resultados de los ficheros de texto con los tiempos de CPU, generando la gráfica comparativa en escala doblemente logarítmica.
* *(Nota: En las celdas del Jupyter también se integran los bloques de código en Python necesarios para leer las matrices de espines y generar las animaciones visuales de la magnetización y la transición de fase).*

#### Archivos de Datos de Rendimiento (.txt)
* **`tiempos_local_O0.txt`** y **`tiempos_local_O3.txt`**: Archivos donde se guardan los tiempos reales de CPU (en segundos) obtenidos en mi máquina virtual local (VirtualBox), compilando el benchmark sin optimizar (`-O0`) y con optimización máxima (`-O3`).
* **`tiempos_joel_O0.txt`** y **`tiempos_joel_O3.txt`**: Ficheros análogos que contienen los tiempos de ejecución extraídos tras conectarme por SSH y lanzar los ejecutables en el nodo secuencial del Clúster JOEL de la UGR.

#### Recursos Visuales Generados
* **`grafica_rendimiento_ising.jpg`**: La gráfica final generada por el script de Python. Muestra de forma limpia la evolución temporal del algoritmo y permite contrastar el desplome del tiempo de ejecución (en un orden de magnitud) gracias a las optimizaciones del compilador, así como la equivalencia en el rendimiento mononúcleo de ambas arquitecturas.

#### Archivos de Volcado (.dat)
* Todos los archivos **`.dat`** generados de manera temporal son los volcados brutos de las configuraciones de espines y las propiedades termodinámicas calculadas por el ejecutable de C++, los cuales sirven como puente para que Python dibuje las gráficas del informe y las animaciones de los dominios magnéticos.

---

### Ecuación de Schrödinger - Estructura del Repositorio

En esta carpeta he incluido todos los archivos correspondientes a mi simulación de la ecuación de Schrödinger unidimensional usando el algoritmo de Crank-Nicolson, además de los scripts utilizados para generar las animaciones interactivas y comprobar la conservación de la probabilidad (la norma).

A continuación detallo para qué sirve cada archivo del repositorio:

#### Memoria del Proyecto
* **`Informe_Schrodinger_v2.ipynb`**: Es el documento central de la práctica (Jupyter Notebook). Aquí explico todo el desarrollo físico, el algoritmo numérico empleado, analizo a fondo fenómenos como el efecto túnel o la reflexión frente a la barrera de potencial, y muestro mis conclusiones. Recomiendo empezar la lectura por aquí, ya que tiene las animaciones y las gráficas incrustadas.

#### Códigos Fuente (C++)
* **`Schrodinger.cpp`**: Es el código principal desarrollado en C++ donde resuelvo la ecuación de Schrödinger dependiente del tiempo. He implementado el método implícito de Crank-Nicolson para asegurar que la evolución del paquete de ondas sea incondicionalmente estable y garantice la unitariedad.
* **`schrodinger`** (o ejecutable sin extensión): El archivo binario resultante tras compilar el código de C++ en mi entorno local (normalmente usando el flag `-O3` para acelerar los cálculos).

#### Scripts de Análisis (Python)
* **`animar.py`**: Script de Python que lee los volcados de las funciones de onda generados por C++ y se encarga de renderizar los fotogramas para construir las animaciones en vídeo.
* *(Nota: El análisis de la conservación de la norma lo realizo leyendo los datos con Python directamente dentro del Jupyter Notebook para generar la gráfica comparativa).*

#### Archivos de Datos de la Norma (.dat)
* **`norma_N500_L0.5.dat`**, **`norma_N500_L10.0.dat`**, etc.: Archivos donde he guardado la evolución del valor de la norma en función del tiempo extraídos de la simulación. Hay varios archivos correspondientes a distintos escenarios (variando la resolución espacial $N$ y la altura de la barrera de potencial $\lambda$).

#### Recursos Visuales Generados (.mp4)
* Los archivos **`.mp4`**: Son las animaciones en vídeo generadas por Python que muestran dinámicamente cómo el paquete de ondas se desplaza, interfiere consigo mismo al rebotar o logra atravesar la barrera naranja (efecto túnel). Estos vídeos los he integrado dentro del notebook para facilitar su visualización.

#### Archivos de Volcado (.dat)
* El resto de archivos **`.dat`** son simplemente los volcados de datos brutos que genera C++ en cada iteración temporal. Contienen los arrays de la densidad de probabilidad cuántica ($|\Phi|^2$) y la forma del potencial ($V$), que luego uso como entrada en Python para dibujar los resultados.

---

### Cohete: Problema Restringido de los Tres Cuerpos - Estructura del Repositorio

En esta carpeta he incluido todos los archivos correspondientes a mi simulación dinámica de la órbita de un cohete en el sistema Tierra-Luna. Para resolver este problema (que no tiene solución analítica exacta), he programado un motor de cálculo numérico en C++ utilizando el algoritmo de Runge-Kutta de 4º orden (RK4) y he desarrollado un entorno en Python para el post-procesado visual y el análisis de la conservación de la energía.

A continuación detallo para qué sirve cada archivo de este repositorio:

#### Memoria del Proyecto
* **`Informe_Cohete_v1.ipynb`**: Es el documento central de la práctica (Jupyter Notebook). Aquí explico detalladamente el desarrollo físico en coordenadas polares, las ecuaciones diferenciales del movimiento, la implementación del integrador RK4, los 4 casos de estudio analizados, las animaciones incrustadas y mis conclusiones finales sobre la asistencia gravitatoria y la estabilidad del sistema. Recomiendo empezar la lectura por aquí.

#### Códigos Fuente (C++)
* **`cohete.cpp`**: Es el código principal desarrollado en C++. En él se implementa el algoritmo de integración RK4 para resolver el sistema de ecuaciones diferenciales del cohete. El programa simula de forma secuencial los 4 escenarios de lanzamiento (Casos 0, 1, 2 y 3) modificando las condiciones de inyección (velocidad y ángulo), y realiza el volcado de posiciones y el cálculo energético a disco de manera eficiente.

#### Scripts de Análisis (Python)
* **`procesar_datos.py`** *(o el nombre que le hayas dado a tu script, por ejemplo `animacion_cohete.py`)*: Un script unificado en Python que automatiza todo el post-procesado. Se encarga de buscar los ficheros generados por el código C++, renderizar los movimientos fluidos directamente en formato GIF (ajustando los FPS óptimos para que carguen rápido en Jupyter) y generar de golpe el panel comparativo de la estabilidad energética.

#### Recursos Visuales Generados
* **`estabilidad_jacobi.png`**: Gráfica final en formato panel 2x2 generada por Python. Muestra la micro-fluctuación de la constante de Jacobi ($H' - H'_0$) a lo largo de los 500.000 pasos de simulación para cada caso. Sirve como prueba visual directa de la extrema precisión de nuestro integrador numérico.
* **`cohete_0.gif`, `cohete_1.gif`, `cohete_2.gif` y `cohete_3.gif`**: Son las animaciones resultantes de la trayectoria del cohete y la posición orbital de la Luna. El Caso 3 es el más destacado, ya que muestra visualmente cómo el cohete es capturado temporalmente por la esfera de influencia lunar, sufriendo una maniobra de asistencia gravitatoria.

#### Archivos de Volcado (.dat)
* **`cohete_0.dat`, `cohete_1.dat`, `cohete_2.dat` y `cohete_3.dat`**: Ficheros de texto brutos que exporta C++ con las trayectorias calculadas en coordenadas cartesianas (tanto para el cohete como para la Luna) listos para que los procese el script de animación.
* **`jacobi_0.dat`, `jacobi_1.dat`, `jacobi_2.dat` y `jacobi_3.dat`**: Archivos de datos independientes donde se almacena el paso de simulación junto con el valor exacto del Hamiltoniano rotacional ($H'$) en cada instante, empleados para construir la gráfica de estabilidad.

---

## 2. Ejercicios voluntarios

### Modelo de Ising (Dinámica de Kawasaki) - Estructura del Repositorio

En esta carpeta he incluido todos los archivos correspondientes a mi simulación del Modelo de Ising bidimensional utilizando el algoritmo de Metrópolis bajo la dinámica de Kawasaki (*spin-exchange*). Además, incorporo el estudio completo de rendimiento computacional y escalabilidad que he realizado comparando mi entorno local con el Clúster JOEL de la UGR.

A diferencia de la dinámica de Glauber, el algoritmo de Kawasaki introduce una ligadura física crítica: la conservación de la magnetización total del sistema, simulando de manera realista procesos cinéticos de difusión molecular y separación de fases.

A continuación detallo para qué sirve cada archivo del repositorio:

#### Memoria del Proyecto
* **`Informe_Kawasaki.ipynb`**: Es el documento central de la práctica (Jupyter Notebook). En este archivo desarrollo todo el fundamento teórico de las dinámicas conservativas, la implementación algorítmica del intercambio de espines vecinos, el análisis del crecimiento de dominios magnéticos (*coarsening*), las animaciones de la segregación de fases y mis conclusiones analíticas sobre el benchmarking. Recomiendo empezar la lectura por aquí.

#### Códigos Fuente (C++)
* **`ising_kawasaki.cpp`**: Es el código principal desarrollado en C++ donde implemento el bucle de Metrópolis modificado para Kawasaki. El programa selecciona un espín al azar y un vecino adyacente, e intenta intercambiar sus posiciones evaluando el cambio energético $\Delta E$ con condiciones de contorno periódicas, garantizando que el número total de espines +1 y -1 permanezca estrictamente constante en el tiempo.
* **`benchmark_kawasaki.cpp`**: Código secundario optimizado y desprovisto de escrituras en disco (I/O) diseñado exclusivamente para los tests de rendimiento. El programa corre la simulación fijando la temperatura en la región subcrítica para forzar al sistema a buscar constantemente reordenamientos energéticos locales complejos. Evalúa redes cuadradas de tamaños N = 16, 32, 64, 128 ejecutando un volumen fijo de Pasos Monte Carlo por espín para medir la velocidad de procesamiento pura de la CPU.

#### Scripts de Análisis (Python)
* **`plot_rendimiento_kawasaki.py`**: Script automatizado en Python encargado de procesar los ficheros `.txt` con los tiempos de CPU acumulados y generar la gráfica comparativa final utilizando escalas logarítmicas.
* *(Nota: Dentro del propio Jupyter Notebook se incluyen también las funciones en Python encargadas de interpretar las matrices binarias para renderizar los mapas de densidad de espín y exportar las animaciones de la separación de fases).*

#### Archivos de Datos de Rendimiento (.txt)
* **`tiempos_local_O0.txt`** y **`tiempos_local_O3.txt`**: Ficheros de texto donde he guardado las mediciones de tiempo de CPU (en segundos) registradas en mi máquina virtual local (VirtualBox), compilando el benchmark en bruto (-O0) y con optimización agresiva de bucles y vectorización (-O3).
* **`tiempos_joel_O0.txt`** y **`tiempos_joel_O3.txt`**: Archivos equivalentes que contienen los tiempos de cómputo mononúcleo extraídos tras compilar y lanzar el código directamente en el nodo de cálculo científico del Clúster JOEL.

#### Recursos Visuales Generados
* **`grafica_rendimiento_kawasaki.png`**: Imagen final producida por el script de Python que muestra las curvas de escalabilidad temporal. Permite analizar visualmente cómo impactan las optimizaciones del compilador en una dinámica que, al requerir la búsqueda explícita de parejas de vecinos y cálculos geométricos adicionales, resulta computacionalmente más pesada que la de Glauber.
* Las animaciones o imágenes adicionales integradas en el informe representan la evolución geométrica del sistema, mostrando gráficamente cómo los espines se agrupan en islas o dominios puros con el paso del tiempo.

#### Archivos de Volcado (.dat)
* Todos los archivos con extensión **`.dat`** actúan como ficheros temporales de datos brutos generados por las rutinas de C++. Contienen los históricos de configuraciones espaciales de la red y sirven de puente para que las librerías de Python realicen el post-procesado visual.

---

### Dinámica Molecular (Lennard-Jones) - Estructura del Repositorio

En esta carpeta están guardados todos los archivos de la simulación de Dinámica Molecular para un gas de partículas encerradas en una caja bidimensional. Las partículas interactúan de forma determinista bajo el potencial de Lennard-Jones y el sistema se resuelve usando el integrador de Verlet.

A continuación explico la función de cada archivo incluido:

#### Memoria del Proyecto
* **`Informe_Lennard_Jones.ipynb`**: Es el Jupyter Notebook definitivo y la memoria central de este trabajo. Contiene la teoría de la Dinámica Molecular, el proceso de adimensionalización, la discusión estadística (como la termalización hacia la distribución de Maxwell-Boltzmann) y las conclusiones finales. Sugiero empezar leyendo este documento.

#### Códigos de Cómputo (C++)
* **`lennard_jones.cpp`**: Es el programa que se encarga de todo el cálculo numérico pesado. Configura el estado inicial del gas, calcula las fuerzas atractivas y repulsivas en cada paso y aplica Verlet para ir actualizando las posiciones y velocidades.

#### Scripts de Análisis (Python)
* **`analisis.py`**: Mi script de Python para el post-procesado. Se encarga de leer los datos en bruto que saca C++, comprobar que la energía mecánica se conserva y generar de forma automática todas las gráficas y animaciones.

#### Gráficas y Resultados Visuales (.png y .gif)
He estructurado las imágenes y animaciones según los dos grandes bloques de la práctica: el Apartado 1 (configuración base) y el Apartado 2 (con sus variantes de densidad y temperatura v2, v3 y v4):
* **`ap1_resultados_combinados.png`**, **`ap2_v2_resultados_combinados.png`**, etc.: Son los gráficos compuestos donde muestro la evolución temporal de las energías para auditar el integrador, junto con el histograma de velocidades frente a la curva teórica de Maxwell-Boltzmann.
* **`ap1_evolucion_gas.gif`**, **`ap2_v2_evolucion_gas.gif`**, etc.: Las animaciones que muestran el comportamiento dinámico real, los choques y cómo se expande el gas por la caja.

#### Archivos de Volcado de Datos Brutos (.dat)
Son los ficheros de texto que genera el ejecutable de C++ y que luego leo desde Python. Para cada uno de los escenarios estudiados, genero tres tipos de archivos:
* **`..._posParticulas.dat`**: Guarda las posiciones de todas las partículas en cada instante para poder animar el movimiento.
* **`..._velocidades.dat`**: Almacena las componentes de velocidad de las partículas, necesario para el análisis estadístico.
* **`..._energias.dat`**: Registra los valores de energía cinética, potencial y total durante toda la simulación para comprobar su conservación.