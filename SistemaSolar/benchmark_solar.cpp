// ==============================================================================
// BENCHMARK - SISTEMA SOLAR (CPU PURA)
// Versión optimizada sin I/O para medir rendimiento mononúcleo
// ==============================================================================

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

using namespace std;

// Función para calcular aceleraciones gravitatorias (sin el Sol moviéndose)
void calcular_aceleracion(int N, const vector<double>& m, const vector<vector<double>>& r, vector<vector<double>>& a) {
    // Resetear aceleraciones
    for (int i = 0; i < N; i++) {
        a[i][0] = 0.0;
        a[i][1] = 0.0;
    }
    
    // Bucle de fuerzas gravitatorias O(N^2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i != j) {
                double dx = r[j][0] - r[i][0];
                double dy = r[j][1] - r[i][1];
                double dist2 = dx * dx + dy * dy;
                double dist = sqrt(dist2);
                
                // En unidades reescaladas G = 1
                double f = m[j] / (dist2 * dist); 
                
                a[i][0] += f * dx;
                a[i][1] += f * dy;
            }
        }
    }
}

int main() {
    const int N = 10;            // Sol + 9 planetas
    const double h = 0.001;      // Paso de integración
    const int pasos_totales = 10000000; // 10 millones de pasos para estresar la CPU

    // Inicialización de variables
    vector<double> m(N, 1.0); 
    m[0] = 333000.0; // Masa del Sol dominante
    
    vector<vector<double>> r(N, vector<double>(2, 0.0));
    vector<vector<double>> v(N, vector<double>(2, 0.0));
    vector<vector<double>> a(N, vector<double>(2, 0.0));

    // Condiciones iniciales ficticias pero estables para el benchmark
    for (int i = 1; i < N; i++) {
        r[i][0] = i * 1.5; // Distancias crecientes
        v[i][1] = sqrt(m[0] / r[i][0]); // Velocidad orbital circular aproximada
    }

    // Aceleración inicial
    calcular_aceleracion(N, m, r, a);

    cout << "Iniciando benchmark de CPU (Sistema Solar - " << pasos_totales << " pasos)..." << endl;

    // Iniciar cronómetro
    auto start = chrono::high_resolution_clock::now();

    // Bucle principal de Velocity Verlet (sin escritura en disco)
    for (int paso = 0; paso < pasos_totales; paso++) {
        // Actualizar posiciones (El Sol en i=0 se asume estático)
        for (int i = 1; i < N; i++) { 
            r[i][0] += v[i][0] * h + 0.5 * a[i][0] * h * h;
            r[i][1] += v[i][1] * h + 0.5 * a[i][1] * h * h;
            v[i][0] += 0.5 * a[i][0] * h;
            v[i][1] += 0.5 * a[i][1] * h;
        }

        calcular_aceleracion(N, m, r, a);

        // Actualizar velocidades
        for (int i = 1; i < N; i++) {
            v[i][0] += 0.5 * a[i][0] * h;
            v[i][1] += 0.5 * a[i][1] * h;
        }
    }

    // Parar cronómetro
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "----------------------------------------" << endl;
    cout << "Benchmark finalizado con exito." << endl;
    cout << "Tiempo de ejecucion: " << duration.count() << " segundos." << endl;
    cout << "----------------------------------------" << endl;

    return 0;
}