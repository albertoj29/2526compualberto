/*
 * PROGRAMA: 
 Simulación del Problema Restringido de los Tres Cuerpos (Tierra-Luna-Cohete)
 * -------------------------------------------------------------------------------------
 * OBJETIVO:
 * Resolver las ecuaciones de movimiento de un cohete en el campo gravitatorio
 * Tierra-Luna usando el método numérico de Runge-Kutta de 4º orden.
 * FUNCIONAMIENTO:
 * 1. El sistema usa coordenadas polares (r, phi) y sus momentos (pr, pphi).
 * 2. Las distancias se normalizan dividiendo por la distancia Tierra-Luna (dTL = 1).
 * 3. Se calcula la trayectoria y se guarda en "cohete_data.dat" en formato cartesiano
 * para que sea compatible con el script de animación de Python.
 * 4. Se monitoriza la constante H' (Integral de Jacobi) para verificar la precisión.
 * * PARÁMETROS QUE SE PUEDEN VARIAR PARA VISUALIZAR DISTINTAS TRAYECTORIAS:
 * - v0_factor: Qué tan cerca de la velocidad de escape estamos (ej. 0.99, 1.01).
 * - theta: Ángulo de lanzamiento. Cambiarlo permite "apuntar" a la Luna.
 * - h: El paso de tiempo. Si es muy grande, perdemos precisión; si es muy pequeño, tarda mucho.
 * * QUÉ SE HARÁ:
 * En primer lugar queremos simular distintas trayectorias variando parámetros, hasta encontrar
 * unos parámetros dados en los que la trayectoria del cohete se vea visualmente afectada
 * por el campo gravitatorio de la luna
 * Después, se justificará que el hamiltoniano rotacional H' debe mantenerse constante a lo largo de la simulación,
 * donde H'=H-omega*pphi, siendo H el hamiltoniano del sistema (energia total), omega es la velocidad angular de 
 * la Luna y pphi el momento angular conjugado a phi.
 * Después se comprobará so el valor de H' se mantiene constante a lo largo de la simulación, lo que indicará que 
 * el método numérico es preciso y que la energía del sistema se conserva adecuadamente.
 * -------------------------------------------------------------------------------------
 */

#include <iostream> // Para entrada/salida por pantalla
#include <fstream>  // Para manejar archivos (.dat)
#include <cmath>    // Para funciones matemáticas (sin, cos, sqrt, pow)
#include <iomanip>  // Para controlar la precisión de los decimales
#include <string>   // Necesario para generar nombres de archivo dinámicos (ej: cohete_0.dat)

using namespace std;

// =========================================================================
// --- CONSTANTES FÍSICAS (Diapositivas de Prado) ---
// =========================================================================
const double G = 6.674e-11;      // Constante de gravitación universal (N·m²/kg²)
const double MT = 5.9736e24;     // Masa de la Tierra (kg)
const double ML = 0.07349e24;    // Masa de la Luna (kg)
const double dTL = 3.844e8;      // Distancia media Tierra-Luna (m)
const double omega = 2.6617e-6;  // Velocidad angular de la Luna (rad/s)
const double RT = 6.3781e6;      // Radio de la Tierra (m)
const double RL = 1.7374e6;      // Radio de la Luna (m)

// Parámetros adimensionales para simplificar las ecuaciones
const double delta = G * MT / pow(dTL, 3);
const double mu = ML / MT;

// =========================================================================
// --- FUNCIONES AUXILIARES ---
// =========================================================================

/**
 * Calcula la distancia cohete-Luna (r') usando el teorema del coseno.
 * r: distancia Tierra-cohete (normalizada)
 * phi: ángulo del cohete
 * t: tiempo actual
 */
double calcular_r_prima(double r, double phi, double t) {
    return sqrt(pow(r, 2) + 1.0 - 2.0 * r * cos(phi - omega * t));
}

/**
 * Calcula el valor de H' (Hamiltoniano en el sistema rotante).
 * Sirve para demostrar la conservación de la energía, aunque no es estrictamente necesario para la simulación.
 * H' = H - omega * p_phi
 */
double calcular_H_prima(double r, double phi, double pr, double pphi, double t) {
    double r_prima = calcular_r_prima(r, phi, t);

    // Energía cinética + Energía potencial (Tierra) + Energía potencial (Luna) - omega*pphi
    // Fórmula basada en el Hamiltoniano de la página 8 de las diapositivas
    double H = (pow(pr, 2) / 2.0) + (pow(pphi, 2) / (2.0 * pow(r, 2)))
               - delta * ((1.0 / r) + (mu / r_prima));

    return H - omega * pphi;
}

/**
 * Define las derivadas para el método de Runge-Kutta 4to orden.
 * t: tiempo actual
 * r, phi, pr, pphi: variables de estado actuales
 * dr, dphi, dpr, dpphi son las variaciones temporales de cada variable.
 */
void derivadas(double t, double r, double phi, double pr, double pphi,
               double &dr, double &dphi, double &dpr, double &dpphi) {
    double r_p = calcular_r_prima(r, phi, t);

    dr = pr;                 // dr/dt
    dphi = pphi / pow(r, 2); // dphi/dt

    // dpr/dt: Incluye fuerza centrífuga y fuerzas gravitatorias
    dpr = (pow(pphi, 2) / pow(r, 3)) - delta * ((1.0 / pow(r, 2)) +
          (mu / pow(r_p, 3)) * (r - cos(phi - omega * t)));

    // dpphi/dt: Torque gravitatorio de la Luna
    dpphi = -(delta * mu * r / pow(r_p, 3)) * sin(phi - omega * t);
}

// =========================================================================
// --- BLOQUE PRINCIPAL ---
// =========================================================================

int main() {
    // Definimos los parámetros "inteligentes" para comparar trayectorias
    // Caso 0: Trayectoria original (poca afectación lunar) osea casi tangencial a la Tierra
    // Caso 1: Velocidad mayor (escape rápido) osea casi tangencial pero más rápida, lo que hace 
    // que el cohete se aleje rápidamente de la Tierra y la Luna tenga menos tiempo para influir
    // Caso 2: Ángulo hacia la Luna (Interacción fuerte/Desviación) isea casi tangencial pero apuntando
    // hacia la Luna, lo que hace que la Luna tenga una influencia más fuerte y cause una desviación 
    // significativa en la trayectoria del cohete
    // Caso 3: Velocidad crítica (Posible captura o impacto) osea casi tangencial pero con una velocidad 
    // que podría permitir que el cohete sea capturado por la gravedad de la Luna o incluso impactar contra
    // ella, dependiendo de la interacción gravitatoria durante el vuelo.
    // --- NUEVOS PARÁMETROS PARA VER LA LUNA ---
    double v_factors[] = {0.9910, 0.9916, 0.9922, 0.9930}; 
    double thetas_deg[] = {20.0, 45.0, 55.0, 60.0};
    int total_casos = 4;

    for (int i = 0; i < total_casos; i++) {
        // 1. VARIABLES DE ESTADO
        double r, phi, pr, pphi;
        double t = 0.0;

        // 2. PARÁMETROS DE SIMULACIÓN
        double h = 2.0;      // Paso de tiempo (Aumentado a 2.0 para llegar a la Luna)
        int pasos = 500000;  // Aumentado para ver la trayectoria completa
        int skip = 500;      // Guardamos cada 500 para que la animación sea fluida

        // 3. CONDICIONES INICIALES (Ahora tomadas de los arrays superiores)
        r = RT / dTL;                          // Lanzamiento desde la superficie
        phi = 0.0;                             // Ángulo de la posición inicial
        double v_esc = sqrt(2.0 * G * MT / RT); // Velocidad de escape terrestre
        double v0 = v_factors[i] * v_esc / dTL; // Velocidad de lanzamiento (normalizada)
        double theta = thetas_deg[i] * M_PI / 180.0; // Ángulo respecto a la vertical

        // Momentos iniciales (Página 11)
        pr = v0 * cos(theta - phi);
        pphi = r * v0 * sin(theta - phi);

        // 4. PREPARACIÓN DE ARCHIVOS (Nombres dinámicos: cohete_0.dat, cohete_1.dat...)
        string nombre_fichero = "cohete_" + to_string(i) + ".dat";
        ofstream archivo(nombre_fichero);
        if (!archivo.is_open()) {
            cerr << "Error abriendo el archivo " << nombre_fichero << endl;
            return 1;
        }

        // --- NUEVO: Archivo para las gráficas de la Integral de Jacobi (Energía) ---
        string nombre_jacobi = "jacobi_" + to_string(i) + ".dat";
        ofstream archivo_jacobi(nombre_jacobi);
        if (!archivo_jacobi.is_open()) {
            cerr << "Error abriendo el archivo " << nombre_jacobi << endl;
            return 1;
        }

        cout << "--- Iniciando Caso " << i << " (v=" << v_factors[i] << ", theta=" << thetas_deg[i] << ") ---" << endl;
        double H_inicial = calcular_H_prima(r, phi, pr, pphi, t);
        cout << "Valor inicial de H': " << H_inicial << endl;

        // --- 5. BUCLE PRINCIPAL DE SIMULACIÓN (RK4) ---
        double k1[4], k2[4], k3[4], k4[4];

        for (int j = 0; j < pasos; j++) {
            // --- PASO A: Guardar datos en el archivo ---
            if (j % skip == 0) {
                double x_cohete = r * cos(phi);
                double y_cohete = r * sin(phi);
                double x_luna = cos(omega * t);
                double y_luna = sin(omega * t);

                archivo << setprecision(10) << x_cohete << "," << y_cohete << endl;
                archivo << setprecision(10) << x_luna << "," << y_luna << endl;
                archivo << endl; 

                // --- NUEVO: Escritura de datos de energía (Paso y Valor H') ---
                double H_actual = calcular_H_prima(r, phi, pr, pphi, t);
                archivo_jacobi << j << " " << setprecision(10) << H_actual << endl;
            }

            // --- PASO B: Algoritmo Runge-Kutta 4º Orden ---
            derivadas(t, r, phi, pr, pphi, k1[0], k1[1], k1[2], k1[3]);
            derivadas(t + h / 2.0, r + h * k1[0] / 2.0, phi + h * k1[1] / 2.0, pr + h * k1[2] / 2.0, pphi + h * k1[3] / 2.0, k2[0], k2[1], k2[2], k2[3]);
            derivadas(t + h / 2.0, r + h * k2[0] / 2.0, phi + h * k2[1] / 2.0, pr + h * k2[2] / 2.0, pphi + h * k2[3] / 2.0, k3[0], k3[1], k3[2], k3[3]);
            derivadas(t + h, r + h * k3[0], phi + h * k3[1], pr + h * k3[2], pphi + h * k3[3], k4[0], k4[1], k4[2], k4[3]);

            r    += (h / 6.0) * (k1[0] + 2.0 * k2[0] + 2.0 * k3[0] + k4[0]);
            phi  += (h / 6.0) * (k1[1] + 2.0 * k2[1] + 2.0 * k3[1] + k4[1]);
            pr   += (h / 6.0) * (k1[2] + 2.0 * k2[2] + 2.0 * k3[2] + k4[2]);
            pphi += (h / 6.0) * (k1[3] + 2.0 * k2[3] + 2.0 * k3[3] + k4[3]);

            t = t + h;

            // --- PASO C: Control de seguridad ---
            double r_p = calcular_r_prima(r, phi, t);
            if (r < (RT / dTL) || r_p < (RL / dTL)) {
                cout << "¡Impacto detectado en el caso " << i << "!" << endl;
                break;
            }
        }

        // --- 6. RESULTADOS FINALES DEL CASO ---
        double H_final = calcular_H_prima(r, phi, pr, pphi, t);
        cout << "Valor final de H': " << H_final << endl;
        cout << "Diferencia (Error de conservación): " << abs(H_final - H_inicial) << endl << endl;

        archivo.close();
        archivo_jacobi.close(); // --- NUEVO: Cierre del archivo de energía ---
    }

    cout << "Simulación de todos los casos finalizada." << endl;
    return 0;
}