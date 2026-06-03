/* ==========================================================================
   MODELO DE ISING 2D: DINÁMICA DE KAWASAKI (TRABAJO VOLUNTARIO)
   ==========================================================================
   Diferencias clave con el obligatorio (Glauber):
   1. Conservación de la magnetización: No volteamos espines, INTERCAMBIAMOS 
      espines vecinos.
   2. Condiciones de contorno: Periódicas en X (izquierda-derecha), pero 
      FIJAS en Y (fila inferior siempre +1, fila superior siempre -1).
   3. Cálculos por dominios: La susceptibilidad y la magnetización se miden
      por separado en el dominio superior y en el inferior.
   ========================================================================== */

#include <iostream> 
#include <fstream>  
#include <cmath>    
#include <random>   
#include <algorithm>
#include <string>   
#include <vector>   // <-- AÑADIDO PARA AUTOMATIZAR EL TAMAÑO N

using namespace std;

// Generadores aleatorios globales
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<double> dist01(0.0, 1.0); 

// Función auxiliar para el mínimo calcular el mínimo de dos números (usada para calcular el tamaño de los dominios)
double minimo(double a, double b) {
    return (a < b) ? a : b;
}

// -------------------------------------------------------------------------
// FUNCIÓN DE INICIALIZACIÓN (Cumpliendo los bordes fijos y la magnetización m0)
// Esta función se llama al inicio de cada temperatura para generar una nueva configuración inicial
// HE CAMBIADO 'int s[N][N]' por 'vector<vector<int>>& s' para permitir que N cambie dinámicamente.
// -------------------------------------------------------------------------
void inicializarRed(vector<vector<int>>& s, int N, double m0) {
    // 1. Fijar los bordes (Fila 0 = Arriba/Superior, Fila N-1 = Abajo/Inferior)
    for (int j = 0; j < N; j++) {
        s[0][j] = -1;       // Borde superior FIJO a -1 (Azul)
        s[N-1][j] = 1;      // Borde inferior FIJO a +1 (Rojo)
    }

    // 2. Rellenar el interior respetando la magnetización inicial (m0)
    // Sitios interiores totales = N * (N - 2)
    int sitios_interiores = N * (N - 2);
    
    // Queremos que el total de espines (+) en el interior nos dé la magnetización m0
    double target_plus = (sitios_interiores + m0 * N * N) / 2.0;
    int n_plus = round(target_plus);

    // Rellenamos ordenadamente
    int contador_plus = 0;
    for (int i = 1; i < N - 1; i++) {
        for (int j = 0; j < N; j++) {
            if (contador_plus < n_plus) {
                s[i][j] = 1;
                contador_plus++;
            } else {
                s[i][j] = -1;
            }
        }
    }

    // 3. Agitar (mezclar) solo el interior para que empiece desordenado (Alta T)
    // Hacemos intercambios aleatorios masivos en el interior para termalizar rápido
    uniform_int_distribution<int> distX(0, N - 1);
    uniform_int_distribution<int> distY(1, N - 2);
    for (int k = 0; k < 100000; k++) {
        int i1 = distY(gen), j1 = distX(gen);
        int i2 = distY(gen), j2 = distX(gen);
        swap(s[i1][j1], s[i2][j2]);
    }
}

// -------------------------------------------------------------------------
// CÁLCULO DE ENERGÍA DE INTERCAMBIO (KAWASAKI)
// Esta función calcula la diferencia de energía al intercambiar dos espines vecinos (i1,j1) y (i2,j2)
// -------------------------------------------------------------------------
double calcularDeltaE_Kawasaki(const vector<vector<int>>& s, int i1, int j1, int i2, int j2, int N) {
    // Si los espines son iguales, intercambiarlos no cambia nada la red
    if (s[i1][j1] == s[i2][j2]) return 0.0;

    // Suma de vecinos del espín 1 (Excluyendo al espín 2)
    int S1 = 0;
    if (!(i1-1 == i2 && j1 == j2)) S1 += s[i1-1][j1];             // Arriba
    if (!(i1+1 == i2 && j1 == j2)) S1 += s[i1+1][j1];             // Abajo
    if (!(i1 == i2 && (j1-1+N)%N == j2)) S1 += s[i1][(j1-1+N)%N]; // Izquierda (Periódica)
    if (!(i1 == i2 && (j1+1)%N == j2)) S1 += s[i1][(j1+1)%N];     // Derecha (Periódica)

    // Suma de vecinos del espín 2 (Excluyendo al espín 1)
    int S2 = 0;
    if (!(i2-1 == i1 && j2 == j1)) S2 += s[i2-1][j2];
    if (!(i2+1 == i1 && j2 == j1)) S2 += s[i2+1][j2];
    if (!(i2 == i1 && (j2-1+N)%N == j1)) S2 += s[i2][(j2-1+N)%N];
    if (!(i2 == i1 && (j2+1)%N == j1)) S2 += s[i2][(j2+1)%N];

    // Fórmula exacta para la diferencia de energía en Kawasaki
    return (s[i1][j1] - s[i2][j2]) * (S1 - S2);
}

// -------------------------------------------------------------------------
// PASO DE MONTE CARLO (N*N INTENTOS DE INTERCAMBIO)
// Estamos usando el algoritmo de Metropolis para decidir si aceptamos o no el intercambio de espines
// -------------------------------------------------------------------------
int realizarPasoMonteCarloKawasaki(vector<vector<int>>& s, double T, int N) {
    int cambios = 0;
    
    // Distribuciones de probabilidad para las posiciones (adaptadas al N actual)
    uniform_int_distribution<int> distX(0, N - 1);
    uniform_int_distribution<int> distY(1, N - 2);
    uniform_int_distribution<int> distDir(0, 3);

    for (int n_intentos = 0; n_intentos < N * N; n_intentos++) {
        // 1. Seleccionar un espín del interior (nunca los bordes fijos)
        int i1 = distY(gen);
        int j1 = distX(gen);

        // 2. Elegir un vecino aleatorio
        int dir = distDir(gen);
        int i2 = i1, j2 = j1;
        
        if (dir == 0) i2 = i1 - 1;             // Arriba
        else if (dir == 1) i2 = i1 + 1;        // Abajo
        else if (dir == 2) j2 = (j1 - 1 + N)%N;// Izquierda (Periódica)
        else if (dir == 3) j2 = (j1 + 1)%N;    // Derecha (Periódica)

        // IMPORTANTE: Si el vecino elegido es un borde fijo, NO SE PUEDE INTERCAMBIAR.
        // Se aborta este intento y se pasa al siguiente.
        if (i2 == 0 || i2 == N - 1) continue; 

        // 3. Evaluar Energía y Probabilidad (Metrópolis)
        double dE = calcularDeltaE_Kawasaki(s, i1, j1, i2, j2, N);
        
        if (dE <= 0 || dist01(gen) < exp(-dE / T)) {
            // Aceptamos el intercambio
            swap(s[i1][j1], s[i2][j2]);
            cambios++;
        }
    }
    return cambios;
}

// -------------------------------------------------------------------------
// CÁLCULO DE LA ENERGÍA TOTAL DE LA RED
// Esta función se llama para medir la energía en cada paso de medida, sumando las 
// interacciones de cada espín con sus vecinos (sin contar dobles)
// -------------------------------------------------------------------------
double calcularEnergiaTotal(const vector<vector<int>>& s, int N) {
    double E = 0;
    // Recorremos toda la red. Para no contar enlaces dobles, solo sumamos 
    // las interacciones hacia la derecha y hacia abajo.
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int espin = s[i][j];
            // Derecha (Condición periódica)
            E += -espin * s[i][(j + 1) % N];
            // Abajo (Sin condición periódica, acaba en el borde)
            if (i + 1 < N) {
                E += -espin * s[i + 1][j];
            }
        }
    }
    return E;
}

// -------------------------------------------------------------------------
// FUNCIÓN PRINCIPAL    
// Aquí se automatizan todas las simulaciones para los diferentes tamaños N y magnetizaciones iniciales m0.
// -------------------------------------------------------------------------
int main() {
int pasos_termalizacion = 100000;  // Le damos un empujón fuerte para romper el hielo a baja T
int pasos_medida = 5000;           // Lo mínimo decente para que la gráfica no sea puro ruido
    
    // =====================================================================
    // Aquí definimos las listas para hacer TODAS las simulaciones de golpe.
    // =====================================================================
    vector<double> lista_magnetizaciones = {0.0, 0.4}; // Estas son las magnetizaciones inciales
    // Concretamente usaremos: Actividades 1-7 (0.0) y Actividad 8 (0.4)
    vector<int> lista_tamanos = {32, 64, 128};         // Diferentes tamaños (N) que nos piden en la Actividad 2
    
    // BUCLE EXTERNO: Recorre las magnetizaciones iniciales (Actividad 8)
    for (double m0 : lista_magnetizaciones) {
        
        // BUCLE INTERNO: Recorre los diferentes tamaños de la red (Actividades 2, 4, 6, 7)
        for (int N : lista_tamanos) {
            
            cout << "\n============================================================" << endl;
            cout << ">> INICIANDO SIMULACION: N = " << N << " | m0 = " << m0 << endl;
            cout << "============================================================" << endl;

            // En los primeros siete apartados tomamos una magnetización inicial de 0
            // Pero el Apartado 8 pide probar una magnetización inicial no nula.
            // Calculamos la frontera que separa el dominio inferior del superior según m0.
            double x_frac = (1.0 + m0) / 2.0; 
            int R_limite = round(x_frac * N); 

            // Creamos la red con el tamaño N actual
            vector<vector<int>> s(N, vector<int>(N));
            
            // Etiqueta de texto que usaremos para no sobreescribir archivos 
            // Ej: "_N32_m0" o "_N128_m4"
            string etiqueta = "_N" + to_string(N) + "_m" + to_string((int)(m0*10));

            // ---------------------------------------------------------------------
            // [ACTIVIDAD 1]: GENERACIÓN DE FOTOGRAMAS PARA ANIMACIÓN
            // Representar posteriormente con python varios fotogramas asociados a varias temperaturas.
            // (Se hace para cada m0 y cada tamaño)
            // ---------------------------------------------------------------------
            double T_animaciones[] = {1.5, 2.3, 3.5}; // Elegí estas tres temperaturas: Baja, Crítica y Alta
            
            for (int t = 0; t < 3; t++) {
                double T_anim = T_animaciones[t];
                cout << "[Actividad 1] Generando fotogramas de animacion a T=" << T_anim << "..." << endl;
                
                // Nombres dinámicos para los archivos, ej: animacion_kawasaki_N32_m0_T1.5.dat
                string nombreArchivoAnim = "animacion_kawasaki" + etiqueta + "_T" + to_string(T_anim).substr(0, 3) + ".dat";
                ofstream archivoAnim(nombreArchivoAnim);
                
                inicializarRed(s, N, m0);
                for (int p = 0; p < 1000; p++) {
                    realizarPasoMonteCarloKawasaki(s, T_anim, N);
                    // Guardamos 1 de cada 10 pasos para no hacer un archivo demasiado grande
                    if (p % 10 == 0) {
                        for (int i = 0; i < N; i++) {
                            for (int j = 0; j < N; j++) {
                                archivoAnim << s[i][j] << (j == N - 1 ? "" : ",");
                            }
                            archivoAnim << "\n";
                        }
                        archivoAnim << "\n";
                    }
                }
                archivoAnim.close();
            }

            // ---------------------------------------------------------------------
            // [ACTIVIDADES 2, 4, 6 y 7]: BARRIDO CONTINUO DE TEMPERATURAS
            // Aquí se calcula la energía, calor específico, magnetización y susceptibilidad
            // "como función de la temperatura" (Hacemos un barrido T de 1.0 a 4.0).
            // ---------------------------------------------------------------------
            cout << "[Actividades 2-7] Iniciando barrido continuo de T (Esto puede tardar)..." << endl;
            string nomDatos = "observables_kawasaki" + etiqueta + ".txt";
            ofstream archivoDatos(nomDatos);
            archivoDatos << "T\tE_media\tCv\tM_bot\tM_top\tChi_bot\tChi_top\n"; // Cabecera

            // [ACTIVIDADES 3 y 5]: Fichero para la densidad de partículas (perfil vertical Y)
            // ----------------------------------------------------------------------------
            // La diferencia entre la 3 y la 5 es que en la 3 tenemos una densidad para cada T, 
            // mientras que en la 5 solo guardamos el perfil para una T fija (ej: T=2.3)
            // -----------------------------------------------------------------------------
            string nomPerfil = "perfil_densidad" + etiqueta + ".txt";
            ofstream archivoPerfil(nomPerfil);

            for (double T = 1.0; T <= 4.0; T += 0.1) {
                inicializarRed(s, N, m0); // Empezamos de nuevo para cada T

                // 1. Termalización (Dejar que el sistema se estabilice)
                for (int p = 0; p < pasos_termalizacion; p++) {
                    realizarPasoMonteCarloKawasaki(s, T, N);
                }

                // Variables para ir acumulando los valores
                double suma_E = 0, suma_E2 = 0;
                double suma_Mbot = 0, suma_Mbot2 = 0;
                double suma_Mtop = 0, suma_Mtop2 = 0;

                // Vector para ir acumulando la densidad de partículas por cada fila i
                vector<double> perfil_y(N, 0.0);

                // 2. Medidas (Promediando sobre un numero suficiente de pasos)
                for (int p = 0; p < pasos_medida; p++) {
                    realizarPasoMonteCarloKawasaki(s, T, N);

                    // Medir Energía
                    double E_actual = calcularEnergiaTotal(s, N);
                    suma_E += E_actual;
                    suma_E2 += E_actual * E_actual;

                    // [ACTIVIDAD 2]: Obtener magnetización por dominios (mitades)
                    double m_bot_actual = 0; // Dominio Inferior (Filas N-R_limite a N-1)
                    double m_top_actual = 0; // Dominio Superior (Filas 0 a N-R_limite-1)

                    for (int i = 0; i < N; i++) {
                        double particulas_en_fila = 0;
                        for (int j = 0; j < N; j++) {
                            if (i >= (N - R_limite)) {
                                m_bot_actual += s[i][j];
                            } else {
                                m_top_actual += s[i][j];
                            }

                            // [ACTIVIDADES 3 y 5]: Contamos espín +1 como partícula para densidad
                            if (s[i][j] == 1) {
                                particulas_en_fila++;
                            }
                        }
                        perfil_y[i] += (particulas_en_fila / N);
                    }

                    suma_Mbot += m_bot_actual;
                    suma_Mbot2 += m_bot_actual * m_bot_actual;
                    suma_Mtop += m_top_actual;
                    suma_Mtop2 += m_top_actual * m_top_actual;
                }

                // 3. Calcular Promedios y Fluctuaciones
                double norm = 1.0 / pasos_medida;
                
                // [ACTIVIDAD 4]: Energía media por partícula
                double E_media = suma_E * norm;
                double E2_media = suma_E2 * norm;
                
                double Mbot_media = suma_Mbot * norm;
                double Mbot2_media = suma_Mbot2 * norm;
                
                double Mtop_media = suma_Mtop * norm;
                double Mtop2_media = suma_Mtop2 * norm;

                double N2 = N * N;
                double N_bot = R_limite * N;
                double N_top = (N - R_limite) * N;

                // [ACTIVIDAD 6]: Calculamos el Calor específico (cN) a partir de las fluctuaciones de la energía
                double cN = (E2_media - E_media * E_media) / (N2 * T * T);
                
                // [ACTIVIDAD 7]: Calculamos la usceptibilidad magnética a partir de las fluctuaciones de la magnetización en cada dominio
                double Chi_bot = (Mbot2_media - Mbot_media * Mbot_media) / (N_bot * T);
                double Chi_top = (Mtop2_media - Mtop_media * Mtop_media) / (N_top * T);

                // Guardamos todo
                archivoDatos << T << "\t" 
                             << E_media / N2 << "\t" 
                             << cN << "\t" 
                             << Mbot_media / N_bot << "\t" 
                             << Mtop_media / N_top << "\t" 
                             << Chi_bot << "\t" 
                             << Chi_top << "\n";

                // Guardamos el perfil de densidad medio para esta T 
                archivoPerfil << T;
                for (int i = 0; i < N; i++) {
                    archivoPerfil << "\t" << (perfil_y[i] * norm);
                }
                archivoPerfil << "\n";
            }

            archivoDatos.close();
            archivoPerfil.close();
            cout << ">> Finalizado barrido para N = " << N << ", m0 = " << m0 << endl;
        }
    }
    
    cout << "\n¡SIMULACION TOTAL COMPLETADA EXITOSAMENTE!" << endl;
    return 0;
}