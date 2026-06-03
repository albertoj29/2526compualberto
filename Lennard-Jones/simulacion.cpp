// ==============================================================================
// SIMULACIÓN DE DINÁMICA MOLECULAR - GAS DE LENNARD-JONES 
// Basado en la estructura original de solar.cpp (el obligatorio del sistema solar)
// ==============================================================================

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>
#include <random> // Necesario para la generación de números aleatorios (mt19937)

using namespace std;

//En primer lugar vamos a definir ciertas constantes que nos interesan, así como algunas funciones
//auxiliares, para así después poder poner el motor de la simulación principal de forma limpia y ordenada.

//En este script de C++ incluiremos todo el código y funciones necesarias para el voluntario, y posteriormente
//haremos las gráficas y animaciones con Python a partir de los archivos de salida generados por esta simulación.

// ==============================================================================
// 1. CONSTANTES GLOBALES DEL SISTEMA (Según el PDF)
// ==============================================================================
const int N = 100;          // Número total de partículas (átomos de argón) 
const double L = 22.0;      // Longitud del lado de la caja cuadrada 
const double m = 1.0;       // Masa reducida de cada partícula
const double h = 0.002;     // Paso de tiempo Delta t 
const double rc = 3.0;      // Radio de corte (cutoff) para la fuerza

const double PI = acos(-1.0); // Definición precisa de Pi

// ==============================================================================
// 2. FUNCIÓN DE INICIALIZACIÓN (Posiciones y velocidades aleatorias)
// ==============================================================================
/* JUSTIFICACIÓN DEL FILTRO ANTIEXPLOSIONES:
   El guion de la práctica exige distribuir 100 átomos de forma puramente aleatoria y 
   uniforme en la caja. Sin embargo, al depender puramente del azar estadístico, la 
   probabilidad de que dos partículas caigan casi en la misma coordenada es altísima. 
   Dado que el potencial de Lennard-Jones tiene un núcleo fuertemente repulsivo que 
   escala como (1/r^12), cualquier distancia inicial r < 0.9 genera una fuerza de 
   repulsión numéricamente gigantesca (tendiendo a infinito) que rompe el integrador 
   de Verlet en el primer paso (explosión térmica).
   
   Solución: Se implementa un método de "Rejection Sampling" (filtro de solapamiento). 
   Se generan posiciones aleatorias uniformes cumpliendo el guion, pero se rechazan y 
   recalculan aquellas que queden a una distancia peligrosa (r^2 < 0.81) de cualquier 
   átomo ya posicionado.
*/

// La siguiente función sirve para inicializar el sistema con posiciones aleatorias pero filtradas para evitar explosiones,
// y con velocidades configurables según el modo de experimento deseado.
void inicializar_sistema_oficial(int N, double L, vector<vector<double>>& r, vector<vector<double>>& v, int modo_vel, double v_modulo) {
    
    // Inicializamos el motor del azar con una semilla única
    random_device rd; 
    mt19937 gen(rd());
    
    // Distribución uniforme para ocupar toda la caja dejando un margen en las paredes
    uniform_real_distribution<> dis_posicion(0.5, L - 0.5);
    uniform_real_distribution<> dis_angulo(0.0, 2.0 * PI);
    
    // NUEVA: Distribución uniforme en el rango 0-1 para el caso especial de viento en Vx
    uniform_real_distribution<> dis_viento(0.0, 1.0);

    for (int i = 0; i < N; i++) {
        bool posicion_valida = false;
        
        while (!posicion_valida) {
            // A. Proponemos una posición puramente aleatoria y uniforme
            double x_propuesto = dis_posicion(gen);
            double y_propuesto = dis_posicion(gen);
            
            posicion_valida = true;
            
            // B. Filtro: Comprobamos que no se solape con ninguna de las partículas ya creadas
            for (int j = 0; j < i; j++) {
                double dx = x_propuesto - r[j][0];
                double dy = y_propuesto - r[j][1];
                
                // Mínima imagen para tener en cuenta las paredes en el chequeo
                dx = dx - L * round(dx / L);
                dy = dy - L * round(dy / L);
                
                double dist2 = dx * dx + dy * dy;
                
                // Si la distancia al cuadrado es menor que 0.81 (distancia r < 0.9),
                // la fuerza repulsiva sería enorme. Marcamos como inválida.
                if (dist2 < 0.81) {
                    posicion_valida = false;
                    break; // Salimos del bucle j y volvemos a intentar un nuevo par aleatorio
                }
            }
            
            // C. Si pasó el filtro de todos los átomos anteriores, la guardamos definitivamente
            if (posicion_valida) {
                r[i][0] = x_propuesto;
                r[i][1] = y_propuesto;
            }
        }

        // ==========================================================================
        // CONFIGURACIÓN DE VELOCIDADES SEGÚN EL EXPERIMENTO 
        // ==========================================================================
        if (modo_vel == 1) {
            // MODO ESTÁNDAR: Generamos un ángulo aleatorio para que el módulo sea exactamente v_modulo
            double theta = dis_angulo(gen); 
            
            // Descomponemos el vector velocidad en X e Y
            v[i][0] = v_modulo * cos(theta); // vx
            v[i][1] = v_modulo * sin(theta); // vy
        } 
        else if (modo_vel == 2) {
            // MODO VIENTO: vx aleatoria positiva entre 0 y 1, vy inicialmente nula
            v[i][0] = dis_viento(gen); // vx > 0
            v[i][1] = 0.0;             // vy = 0
        }
    }
}

// ==============================================================================
// 3. CÁLCULO DE ACELERACIÓN (Fuerza de Lennard-Jones y Mínima Imagen)
// ==============================================================================
// Esta función calcula las aceleraciones de cada partícula debido a las fuerzas de Lennard-Jones
void calcular_aceleracion_LJ(int N, double L, const vector<vector<double>>& r, vector<vector<double>>& a) {
    
    // Ponemos todas las aceleraciones a cero antes de empezar
    for (int i = 0; i < N; i++) {
        a[i][0] = 0.0;
        a[i][1] = 0.0;
    }

    double rc2 = rc * rc; // Radio de corte al cuadrado

    // Calculamos las fuerzas interaccionando todas las parejas (sin repetir)
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) { 
            
            // Distancia matemática cruda entre la partícula i y la j
            double dx = r[i][0] - r[j][0];
            double dy = r[i][1] - r[j][1];

            // CONVENCIÓN DE MÍNIMA IMAGEN (Efecto Pac-Man para fuerzas)
            dx = dx - L * round(dx / L);
            dy = dy - L * round(dy / L);

            double r2 = dx * dx + dy * dy; // Distancia real al cuadrado

            // Solo interactúan si están más cerca que el radio de corte
            if (r2 < rc2 && r2 > 0.0001) {
                
                double r2_inv = 1.0 / r2;
                double r6_inv = r2_inv * r2_inv * r2_inv;
                
                // Magnitud de la fuerza (derivada del potencial dividida por r)
                double f_mag = 24.0 * r2_inv * r6_inv * (2.0 * r6_inv - 1.0);

                // Como m=1, a=F
                a[i][0] += f_mag * dx;
                a[i][1] += f_mag * dy;
                
                // 3ª Ley de Newton
                a[j][0] -= f_mag * dx; 
                a[j][1] -= f_mag * dy;
            }
        }
    }
}

// ==============================================================================
// 4. FUNCIÓN PARA CALCULAR ENERGÍAS (Cinética, Potencial y Total)
// ==============================================================================
// Esta función calcula la energía cinética, potencial y total del sistema en un instante dado
vector<double> calcular_energias_LJ(int N, double L, const vector<vector<double>>& r, const vector<vector<double>>& v) {
    double E_cinetica = 0.0;
    double E_potencial = 0.0;
    double rc2 = rc * rc;

    // 4.1 Energía cinética del sistema
    for (int i = 0; i < N; i++) {
        double v2 = v[i][0]*v[i][0] + v[i][1]*v[i][1];
        E_cinetica += 0.5 * m * v2;
    }

    // 4.2 Energía potencial con mínima imagen
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            double dx = r[i][0] - r[j][0];
            double dy = r[i][1] - r[j][1];

            dx = dx - L * round(dx / L);
            dy = dy - L * round(dy / L);

            double r2 = dx * dx + dy * dy;

            if (r2 < rc2 && r2 > 0.0001) {
                double r2_inv = 1.0 / r2;
                double r6_inv = r2_inv * r2_inv * r2_inv;
                // V(r) = 4 * [ (1/r)^12 - (1/r)^6 ]
                E_potencial += 4.0 * r6_inv * (r6_inv - 1.0);
            }
        }
    }

    double E_total = E_cinetica + E_potencial;
    
    // Devolvemos los tres valores empaquetados
    return {E_cinetica, E_potencial, E_total};
}

// ==============================================================================
// 5. MOTOR GENERAL DE LA SIMULACIÓN (Algoritmo de Verlet Parametrizado)
// ==============================================================================
// Se encapsula el bucle original para poder llamarlo de forma repetida con prefijos dinámicos
//Esta función sirve para ejecutar la simulación principal del sistema de Lennard-Jones con diferentes 
//configuraciones de velocidad inicial y guardar los resultados en archivos con prefijos personalizados.
void ejecutar_simulacion(string prefijo, int modo_vel, double v_modulo) {
    cout << "   Ejecutando simulacion [" << prefijo << "] ... " << flush;

    vector<vector<double>> r(N, vector<double>(2));
    vector<vector<double>> v(N, vector<double>(2));
    vector<vector<double>> a(N, vector<double>(2, 0.0));

    // Llamamos a nuestra función oficial pasándole los parámetros dinámicos de velocidad
    inicializar_sistema_oficial(N, L, r, v, modo_vel, v_modulo);

    // Abrimos obligatoriamente los tres archivos de salida usando el prefijo único del experimento
    ofstream f_out(prefijo + "posParticulas.dat");
    ofstream f_eng(prefijo + "energias.dat");
    ofstream f_vel(prefijo + "velocidades.dat");

    if (!f_out || !f_eng || !f_vel) {
        cerr << "Error al abrir los archivos de salida para " << prefijo << endl;
        return;
    }

    calcular_aceleracion_LJ(N, L, r, a);

    // Para llegar a t = 250 con h = 0.002 necesitamos exactamente 125000 pasos
    int num_pasos = 125000; 
    double t = 0.0;

    // ================
    // BUCLE DE VERLET 
    // ================
    for (int n = 0; n < num_pasos; n++) {
        
        // A. Guardamos las posiciones actuales (cada 10 pasos) 
        if (n % 10 == 0) {
            for (int i = 0; i < N; i++) {
                f_out << r[i][0] << ", " << r[i][1] << endl;
            }
            f_out << endl; 
        }

        // B. Guardamos energías (cada 10 pasos)
        if (n % 10 == 0) {
            vector<double> energias = calcular_energias_LJ(N, L, r, v);
            f_eng << t << "\t" << energias[0] << "\t" << energias[1] << "\t" << energias[2] << endl;
        }

        // C. Guardamos velocidades 'brutas' para histogramas
        // Guardamos en t=0 y luego en el rango de equilibrio t=[100, 250] cada 50 pasos
        if (n == 0 || (t >= 100.0 && t <= 250.0 && n % 50 == 0)) {
            for (int i = 0; i < N; i++) {
                double mod_v = sqrt(v[i][0]*v[i][0] + v[i][1]*v[i][1]);
                f_vel << t << "\t" << v[i][0] << "\t" << v[i][1] << "\t" << mod_v << endl;
            }
        }

        // D. Etapa 1 de Verlet (Posiciones y medio paso de velocidad)
        for (int i = 0; i < N; i++) {
            for (int k = 0; k < 2; k++) {
                r[i][k] += v[i][k] * h + 0.5 * a[i][k] * h * h;
                v[i][k] += 0.5 * a[i][k] * h;
            }

            // --- CONDICIONES DE CONTORNO PERIÓDICAS (Efecto Pacman) ---
            // Para evitar que las partículas se salgan de la caja, aplicamos la función módulo con la longitud L.
            r[i][0] = fmod(r[i][0], L); if (r[i][0] < 0) r[i][0] += L;
            r[i][1] = fmod(r[i][1], L); if (r[i][1] < 0) r[i][1] += L;
        }

        // E. Recalcular las aceleraciones en las nuevas posiciones
        calcular_aceleracion_LJ(N, L, r, a);

        // F. Etapa 2 de Verlet (Completar el paso de velocidad)
        for (int i = 0; i < N; i++) {
            for (int k = 0; k < 2; k++) {
                v[i][k] += 0.5 * a[i][k] * h;
            }
        }

        t += h; // Avanzar el reloj de la simulación 
    }

    f_out.close();
    f_eng.close();
    f_vel.close();
    
    cout << "Terminada. Archivos guardados." << endl;
}

// ==============================================================================
// == NUEVAS FUNCIONES DE APOYO AVANZADAS PARA APARTADOS 3, 4, 5, 6, 7 Y 8 =====
// ==============================================================================

// INICIALIZADOR AVANZADO PARAMÉTRICO: Permite estructurar redes cristalinas ideales y desordenadas
//Esta función permite inicializar el sistema con diferentes configuraciones de red (cuadrada, hexagonal o desordenada) y con velocidades iniciales
// parametrizadas según el experimento.
void inicializar_red_avanzada(int N_loc, double L_loc, vector<vector<double>>& r, vector<vector<double>>& v, int tipo_red, double v_modulo) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis_angulo(0.0, 2.0 * PI);
    // Pequeño ruido térmico inicial estocástico para romper asimetrías de fuerzas idénticas en redes simétricas perfectas
    uniform_real_distribution<> dis_ruido(-0.0001, 0.0001); 

    if (tipo_red == 1) { 
        // 1. RED CUADRADA PERFECTA (Exigido en Apartado 4)
        int M = round(sqrt(N_loc));
        double espaciado = L_loc / M;
        int index = 0;
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                if (index < N_loc) {
                    r[index][0] = (i + 0.5) * espaciado + dis_ruido(gen);
                    r[index][1] = (j + 0.5) * espaciado + dis_ruido(gen);
                    index++;
                }
            }
        }
    } 
    else if (tipo_red == 2) { 
        // 2. RED EN PANAL / HEXAGONAL (Exigido en Apartado 5: Cada partícula con 3 vecinas directas)
        int p = 0;
        // Para N=144, organizamos una rejilla de 12x6 celdas unitarias (cada una contiene 2 átomos)
        int nx = 12; 
        int ny = 6;
        
        double dx = L_loc / nx;
        double dy = L_loc / ny;

        for (int i = 0; i < nx; ++i) {
            for (int j = 0; j < ny; ++j) {
                if (p < N_loc) {
                    // Primer átomo de la celda unidad
                    r[p][0] = i * dx + dx * 0.25 + dis_ruido(gen);
                    r[p][1] = j * dy + dy * 0.25 + dis_ruido(gen);
                    p++;
                }
                if (p < N_loc) {
                    // Segundo átomo de la celda unidad (desplazado para formar el panal)
                    r[p][0] = i * dx + dx * 0.75 + dis_ruido(gen);
                    r[p][1] = j * dy + dy * 0.75 + dis_ruido(gen);
                    p++;
                }
            }
        }
    }
    else { 
        // 3. DISTRIBUCIÓN DESORDENADA PARA ALTA DENSIDAD (Líquidos/Gases densos)
        // Colocamos en red y aplicamos un "terremoto" aleatorio masivo para romper el orden 
        // sin riesgo de entrar en un bucle infinito buscando huecos.
        int M = ceil(sqrt(N_loc));
        double espaciado = L_loc / M;
        // Agitación espacial: las movemos al azar hasta un 40% del espacio que las separa
        uniform_real_distribution<> dis_desorden(-espaciado * 0.4, espaciado * 0.4); 
        
        int index = 0;
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                if (index < N_loc) {
                    r[index][0] = (i + 0.5) * espaciado + dis_desorden(gen);
                    r[index][1] = (j + 0.5) * espaciado + dis_desorden(gen);
                    
                    // Aseguramos que no se salgan de la caja al agitarlas (Efecto Pacman inicial)
                    r[index][0] = fmod(r[index][0], L_loc); if (r[index][0] < 0) r[index][0] += L_loc;
                    r[index][1] = fmod(r[index][1], L_loc); if (r[index][1] < 0) r[index][1] += L_loc;
                    
                    index++;
                }
            }
        }
    }

    // Configuración paramétrica de velocidades iniciales
    for (int i = 0; i < N_loc; i++) {
        if (v_modulo > 0.0) {
            double theta = dis_angulo(gen);
            v[i][0] = v_modulo * cos(theta);
            v[i][1] = v_modulo * sin(theta);
        } else {
            v[i][0] = 0.0; v[i][1] = 0.0; // Sistema en total reposo absoluto
        }
    }
}

// MOTOR DE SIMULACIÓN PARA EL APARTADO 3: Ecuación de Estado midiendo Presión
//Esta función sirve para  medir la presión a partir del momento acumulado de los rebotes periódicos (PBC) 
//y se registra junto con la temperatura media en equilibrio para diferentes velocidades iniciales. 
//Los resultados se guardan en un archivo consolidado para su posterior análisis en Python.
void simular_ap3_presion(string prefijo, double v_mod) {
    int N_3 = 160; double L_3 = 32.0; double h_local = 0.002;
    int pasos = 125000; // Simulación hasta t = 250

    vector<vector<double>> r(N_3, vector<double>(2)), v(N_3, vector<double>(2)), a(N_3, vector<double>(2));
    inicializar_red_avanzada(N_3, L_3, r, v, 3, v_mod); // Distribución desordenada
    calcular_aceleracion_LJ(N_3, L_3, r, a);

    double momento_pbc_acumulado = 0.0;
    double T_acumulada_eq = 0.0;
    int conteo_eq = 0;

    for (int n = 0; n < pasos; n++) {
        double t = n * h_local;
        
        // Medimos la temperatura media solo en la ventana temporal estabilizada de equilibrio: t = [100, 250]
        if (t >= 100.0) {
            double v2_sum = 0;
            for(int i = 0; i < N_3; i++) v2_sum += (v[i][0]*v[i][0] + v[i][1]*v[i][1]);
            T_acumulada_eq += 0.5 * (v2_sum / N_3);
            conteo_eq++;
        }

        // Integración de Verlet Etapa 1 + Monitor de rebote/salto macroscópico por PBC para cálculo de Presión
        for (int i = 0; i < N_3; i++) {
            for (int k = 0; k < 2; k++) {
                r[i][k] += v[i][k] * h_local + 0.5 * a[i][k] * h_local * h_local;
                v[i][k] += 0.5 * a[i][k] * h_local;
            }

            // Detección de colisión virtual con la pared (salto periódico en X)
            double old_x = r[i][0];
            r[i][0] = fmod(r[i][0], L_3); if (r[i][0] < 0) r[i][0] += L_3;
            if (fabs(r[i][0] - old_x) > L_3 / 2.0) momento_pbc_acumulado += 2.0 * m * fabs(v[i][0]);

            // Detección de colisión virtual con la pared (salto periódico en Y)
            double old_y = r[i][1];
            r[i][1] = fmod(r[i][1], L_3); if (r[i][1] < 0) r[i][1] += L_3;
            if (fabs(r[i][1] - old_y) > L_3 / 2.0) momento_pbc_acumulado += 2.0 * m * fabs(v[i][1]);
        }

        calcular_aceleracion_LJ(N_3, L_3, r, a);
        for (int i = 0; i < N_3; i++) {
            for (int k = 0; k < 2; k++) v[i][k] += 0.5 * a[i][k] * h_local;
        }
    }

    double T_media = T_acumulada_eq / conteo_eq;
    double tiempo_total = pasos * h_local;
    double perimetro_caja = 4.0 * L_3;
    double Presion = momento_pbc_acumulado / (tiempo_total * perimetro_caja);

    // Registro consolidado en archivo único para ajuste lineal en Python
    ofstream f_res("ap3_ecuacion_estado.dat", ios::app);
    f_res << fixed << setprecision(5) << T_media << "\t" << Presion << "\t" << v_mod << endl;
    f_res.close();
    cout << "   -> [Ap3 Done] Punto registrado: Temp=" << T_media << " | Presion=" << Presion << endl;
}

// MOTOR DE SIMULACIÓN PARA LOS APARTADOS 4, 5, 6 Y 7: Transiciones de Fase y Calentamientos
//Esta función sirve para simular la evolución temporal de un sistema de Lennard-Jones con 144 partículas, partiendo de una red cristalina perfecta
void simular_fases_y_calentamiento(string prefijo, int tipo_red, int modo_experimento) {
    int N_fases = 144; double L_fases = 12.0; double h_local = 0.002;
    int pasos = (modo_experimento == 7) ? 1250000 : 250000; // El apartado 7 requiere un rango extendido hasta t=2500

    vector<vector<double>> r(N_fases, vector<double>(2)), v(N_fases, vector<double>(2)), a(N_fases, vector<double>(2));
    vector<vector<double>> r_unfolded(N_fases, vector<double>(2)); // Coordenadas desenvueltas continuas para evitar discontinuidades de PBC en el MSD

    inicializar_red_avanzada(N_fases, L_fases, r, v, tipo_red, 0.0); // Arrancan en reposo inicial absoluto
    r_unfolded = r;
    vector<vector<double>> r_iniciales = r; // Almacén de posiciones nativas en t=0 para cálculo exacto del MSD
    calcular_aceleracion_LJ(N_fases, L_fases, r, a);

    ofstream f_out(prefijo + "evolucion.dat");
    ofstream f_prop(prefijo + "propiedades.dat");

    for (int n = 0; n < pasos; n++) {
        double t = n * h_local;

        // --- PROTOCOLO DE CALENTAMIENTO SECTORIZADO ---
        if (modo_experimento == 6) { 
            // Apartado 6: Rescalado térmico instantáneo por factor 1.5 en tiempos definidos por guión
            if (fabs(t-180.0)<1e-4 || fabs(t-270.0)<1e-4 || fabs(t-300.0)<1e-4 || fabs(t-400.0)<1e-4) {
                for(int i = 0; i < N_fases; i++) { v[i][0] *= 1.5; v[i][1] *= 1.5; }
            }
        }
        else if (modo_experimento == 7) { 
            // Apartado 7: Calentamiento cuasiestático lento continuo. Factor 1.1 cada 500 unidades de tiempo
            if (n > 0 && n % 250000 == 0) { 
                for(int i = 0; i < N_fases; i++) { v[i][0] *= 1.1; v[i][1] *= 1.1; }
            }
        }

        // Algoritmo de integración clásica de Verlet
        for (int i = 0; i < N_fases; i++) {
            for (int k = 0; k < 2; k++) {
                double dr = v[i][k] * h_local + 0.5 * a[i][k] * h_local * h_local;
                r[i][k] += dr;
                r_unfolded[i][k] += dr; // Espacio métrico real recorrido continuo (sin saltos de frontera)
                v[i][k] += 0.5 * a[i][k] * h_local;
            }
            // Las posiciones de visualización sí sufren la compresión periódica de la caja
            r[i][0] = fmod(r[i][0], L_fases); if (r[i][0] < 0) r[i][0] += L_fases;
            r[i][1] = fmod(r[i][1], L_fases); if (r[i][1] < 0) r[i][1] += L_fases;
        }

        calcular_aceleracion_LJ(N_fases, L_fases, r, a);
        for (int i = 0; i < N_fases; i++) {
            for (int k = 0; k < 2; k++) v[i][k] += 0.5 * a[i][k] * h_local;
        }

        // Exportación de frames cada 500 pasos para análisis estructural en Python
        if (n % 500 == 0) {
            for (int i = 0; i < N_fases; i++) f_out << r[i][0] << ", " << r[i][1] << endl;
            f_out << endl;
        }

        // Medidas termodinámicas de observables moleculares
        if (n % 100 == 0) {
            double v2_sum = 0;
            for(int i = 0; i < N_fases; i++) v2_sum += (v[i][0]*v[i][0] + v[i][1]*v[i][1]);
            double T_inst = 0.5 * (v2_sum / N_fases);

            if (modo_experimento == 6) { 
                // Registro de Fluctuaciones de posición mediante MSD lineal
                double msd_sum = 0.0;
                for(int i = 0; i < N_fases; i++) {
                    double dx = r_unfolded[i][0] - r_iniciales[i][0];
                    double dy = r_unfolded[i][1] - r_iniciales[i][1];
                    msd_sum += (dx*dx + dy*dy);
                }
                f_prop << t << "\t" << T_inst << "\t" << (msd_sum / N_fases) << endl;
            }
            else if (modo_experimento == 7) { 
                // Registro del alejamiento cuadrático medio de todos los pares atómicos del sistema
                double suma_dist2_pares = 0.0;
                long long total_pares = 0;
                for(int i = 0; i < N_fases; i++) {
                    for(int j = i + 1; j < N_fases; j++) {
                        double dx = r_unfolded[i][0] - r_unfolded[j][0];
                        double dy = r_unfolded[i][1] - r_unfolded[j][1];
                        suma_dist2_pares += (dx*dx + dy*dy);
                        total_pares++;
                    }
                }
                f_prop << t << "\t" << T_inst << "\t" << (suma_dist2_pares / total_pares) << endl;
            }
        }
    }
    f_out.close(); f_prop.close();
    cout << "   -> [Fases Done] Completado experimento base: " << prefijo << endl;
}

// MOTOR DE SIMULACIÓN PARA EL APARTADO 8: Histograma estadístico de la Función de Distribución Radial g(r)
//Esta función sirve para calcular el histograma de la función de distribución radial g(r) a partir de una 
//simulación de 144 partículas, partiendo de una red cristalina perfecta, y se exporta el histograma crudo 
//para su posterior normalización y graficado en Python.
void simular_ap8_g_de_r(string archivo_salida, int tipo_red, double L_caja, double v_inicial) {
    int N_8 = 144; double h_8 = 0.001; // El guión de la práctica exige explícitamente usar h = 0.001 aquí
    int pasos = 150000;

    vector<vector<double>> r(N_8, vector<double>(2)), v(N_8, vector<double>(2)), a(N_8, vector<double>(2));
    inicializar_red_avanzada(N_8, L_caja, r, v, tipo_red, v_inicial);
    calcular_aceleracion_LJ(N_8, L_caja, r, a);

    int num_bins = 40;
    double r_min = 1.0, r_max = 3.0;
    double dr = (r_max - r_min) / num_bins;
    vector<long long> histograma_gr(num_bins, 0);

    for (int n = 0; n < pasos; n++) {
        for (int i = 0; i < N_8; i++) {
            for (int k = 0; k < 2; k++) {
                r[i][k] += v[i][k] * h_8 + 0.5 * a[i][k] * h_8 * h_8;
                v[i][k] += 0.5 * a[i][k] * h_8;
            }
            r[i][0] = fmod(r[i][0], L_caja); if (r[i][0] < 0) r[i][0] += L_caja;
            r[i][1] = fmod(r[i][1], L_caja); if (r[i][1] < 0) r[i][1] += L_caja;
        }
        
        calcular_aceleracion_LJ(N_8, L_caja, r, a);
        for (int i = 0; i < N_8; i++) {
            for (int k = 0; k < 2; k++) v[i][k] += 0.5 * a[i][k] * h_8;
        }

        // MUESTREO ESTADÍSTICO DE g(r): Esperamos al paso 50000 para garantizar la termalización completa del sistema
        if (n >= 50000 && n % 100 == 0) { 
            for (int i = 0; i < N_8; i++) {
                for (int j = 0; j < N_8; j++) {
                    if (i == j) continue; 
                    
                    double dx = r[i][0] - r[j][0];
                    double dy = r[i][1] - r[j][1];
                    dx -= L_caja * round(dx / L_caja);
                    dy -= L_caja * round(dy / L_caja);
                    double distancia = sqrt(dx*dx + dy*dy);

                    if (distancia >= r_min && distancia < r_max) {
                        int bin_objetivo = (distancia - r_min) / dr;
                        histograma_gr[bin_objetivo]++;
                    }
                }
            }
        }
    }

    // Exportación directa del histograma crudo normalizable en Python
    ofstream f_hist(archivo_salida);
    for (int b = 0; b < num_bins; b++) {
        double r_centro = r_min + (b + 0.5) * dr;
        f_hist << r_centro << "\t" << histograma_gr[b] << endl;
    }
    f_hist.close();
    cout << "   -> [Ap8 Done] Histograma g(r) exportado a: " << archivo_salida << endl;
}

// ==============================================================================
// 6. FUNCIÓN PRINCIPAL MAIN (Lanzador de la batería de experimentos)
// ==============================================================================
int main() {
    cout << "====================================================" << endl;
    cout << "  INICIANDO AUTOMATIZACION: APARTADOS 1 Y 2" << endl;
    cout << "====================================================" << endl;
    
    // --- APARTADO 1: Simulación original con módulo v = 1 ---
    ejecutar_simulacion("ap1_", 1, 1.0);
    
    // --- APARTADO 2: Estudio cambiando el módulo inicial a v = 2, 3 y 4 ---
    ejecutar_simulacion("ap2_v2_", 1, 2.0);
    ejecutar_simulacion("ap2_v3_", 1, 3.0);
    ejecutar_simulacion("ap2_v4_", 1, 4.0);
    
    // --- APARTADO 2: Caso especial viento uniforme (vx en [0,1], vy = 0) ---
    ejecutar_simulacion("ap2_viento_", 2, 0.0);
    //El hecho de que nos refiramos a esa vx>0 como "viento" es porque tenemos un sistema con un flujo direccional 
    //preferente, aunque en realidad es una configuración de velocidades iniciales asimétrica que induce un movement 
    //colectivo en la dirección X, como veremos al hacer las animaciones

    // ==========================================================================
    // Lanzamos los experimentos complementarios de los apartados 3 al 8, 
    //cada uno con su función específica y parámetros personalizados
    // ==========================================================================
    cout << "\n====================================================" << endl;
    cout << "  LANZANDO SECCIONES COMPLEMENTARIAS (APARTADOS 3 AL 8)" << endl;
    cout << "====================================================" << endl;

    // --- APARTADO 3: Obtención de puntos (Temperatura, Presión) para la ley de gases ideales
    cout << "\n[PROCESANDO] Apartado 3: Midiendo curvas de Presion..." << endl;
    ofstream("ap3_ecuacion_estado.dat", ios::trunc).close(); // Vaciado preventivo estructural
    simular_ap3_presion("ap3_p1_", 1.0);
    simular_ap3_presion("ap3_p2_", 2.2);
    simular_ap3_presion("ap3_p3_", 3.5);
    simular_ap3_presion("ap3_p4_", 4.8);

    // --- APARTADOS 4 Y 5: Relajación de fases sólidas hacia estructuras triangulares estables
    cout << "\n[PROCESANDO] Apartados 4 y 5: Relajacion y cristalizacion de redes..." << endl;
    simular_fases_y_calentamiento("ap4_red_cuadrada_", 1, 4); 
    simular_fases_y_calentamiento("ap5_red_panal_",    2, 4); 
    simular_fases_y_calentamiento("ap5_desordenado_",  3, 4); 

    // --- APARTADO 6: Fusión brusca mediante saltos de rescalado térmico (1.5x) y cálculo del MSD
    cout << "\n[PROCESANDO] Apartado 6: Estudio de MSD y Fluctuaciones en Fusion..." << endl;
    simular_fases_y_calentamiento("ap6_fusion_", 1, 6);

    // --- APARTADO 7: Calentamiento cuasiestático lento para localizar la Temperatura Crítica
    cout << "\n[PROCESANDO] Apartado 7: Busqueda fina de Temperatura Critica..." << endl;
    simular_fases_y_calentamiento("ap7_critico_", 1, 7);

    // --- APARTADO 8: Muestreo estadístico de g(r) en Fase Sólida, Líquida y Gaseosa (Diluida)
    cout << "\n[PROCESANDO] Apartado 8: Perfiles de Distribucion Radial g(r)..." << endl;
    simular_ap8_g_de_r("ap8_gr_solido.dat",  1, 12.0, 0.0);  // Sólido denso ordenado espontáneamente
    simular_ap8_g_de_r("ap8_gr_liquido.dat", 1, 12.0, 4.5);  // Líquido denso fundido
    simular_ap8_g_de_r("ap8_gr_gas.dat",     3, 35.0, 5.0);  // Gas diluido (Caja extendida L=35.0)

    cout << "\n====================================================" << endl;
    cout << "  ¡PROCESO COMPLETADO! Todos los .dat estan listos." << endl;
    cout << "====================================================" << endl;

    return 0;
}// Versión definitiva
