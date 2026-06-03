/* ==========================================================================
   BENCHMARK DE ESCALABILIDAD - DINÁMICA MOLECULAR (LENNARD-JONES)
   VAMOS A COMPARAR EL RENDIMIENTO DE NUESTRO CÓDIGO DE LENNARD-JONES,
   EJECUTÁNDOLO EN MI ORDENADOR CON EL COMANDO: "g++ -O0 benchmark_lj.cpp -o bench_lento"
   Y LUEGO "./bench_lento tiempos_lento.txt" VS ACELERÁNDOLO CON: "g++ -O3 benchmark_lj.cpp -o bench_rapido"
   Y HAREMOS LO MISMO EN EL CLÚSTER JOEL PARA VER LAS DIFERENCIAS DE HARDWARE.
   ========================================================================== */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>
#include <chrono> // MUY IMPORTANTE: Librería para medir tiempos de alta precisión

using namespace std;

// Constantes físicas fijas para el benchmark
const double m = 1.0;
const double h = 0.002;
const double rc = 3.0;

// ==============================================================================
// FUNCIÓN PARA CALCULAR ACELERACIONES (El cuello de botella O(N^2) a medir)
// ==============================================================================
void calcular_aceleracion_LJ(int N, double L, const vector<vector<double>>& r, vector<vector<double>>& a) {
    for (int i = 0; i < N; i++) {
        a[i][0] = 0.0;
        a[i][1] = 0.0;
    }

    double rc2 = rc * rc;

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            double dx = r[i][0] - r[j][0];
            double dy = r[i][1] - r[j][1];

            // Mínima imagen
            dx = dx - L * round(dx / L);
            dy = dy - L * round(dy / L);

            double r2 = dx * dx + dy * dy;

            if (r2 < rc2 && r2 > 0.0001) {
                double r2_inv = 1.0 / r2;
                double r6_inv = r2_inv * r2_inv * r2_inv;
                double f_mag = 24.0 * r2_inv * r6_inv * (2.0 * r6_inv - 1.0);

                a[i][0] += f_mag * dx;
                a[i][1] += f_mag * dy;
                a[j][0] -= f_mag * dx; 
                a[j][1] -= f_mag * dy;
            }
        }
    }
}

// ==============================================================================
// MAIN: MOTOR DEL BENCHMARK
// ==============================================================================
int main(int argc, char* argv[]) {
    // Control de paso de argumentos por terminal
    if (argc != 2) {
        cerr << "Uso correcto: " << argv[0] << " <archivo_salida.txt>" << endl;
        return 1;
    }

    string nombre_archivo_salida = argv[1];
    ofstream f_out(nombre_archivo_salida);

    if (!f_out) {
        cerr << "Error creando el archivo de salida." << endl;
        return 1;
    }

    cout << "=== INICIANDO BENCHMARK DE LENNARD-JONES ===" << endl;
    cout << "Guardando resultados en: " << nombre_archivo_salida << endl;
    cout << "N\tTiempo (s)" << endl;

    // Tamaños del sistema a evaluar (N partículas). 
    // Como es O(N^2), 1600 ya supone bastante carga para el procesador.
    vector<int> lista_tamanos = {100, 200, 400, 800, 1200, 1600};
    
    int pasos_medida = 2000; // Número fijo de pasos de integración por experimento
    
    // Densidad constante extraída del Apartado 1: rho = N / L^2 = 100 / (22^2)
    double densidad_base = 100.0 / (22.0 * 22.0);

    for (int N_bench : lista_tamanos) {
        // Adaptamos el tamaño de la caja L para mantener la densidad idéntica al crecer N
        double L_bench = sqrt(N_bench / densidad_base);

        vector<vector<double>> r(N_bench, vector<double>(2));
        vector<vector<double>> v(N_bench, vector<double>(2, 1.0)); // Velocidad dummy inicial
        vector<vector<double>> a(N_bench, vector<double>(2, 0.0));

        // Inicializamos las posiciones en una cuadrícula segura para evitar 
        // explosiones numéricas infinitas durante el benchmark.
        int M = ceil(sqrt(N_bench));
        double espaciado = L_bench / M;
        int idx = 0;
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                if (idx < N_bench) {
                    r[idx][0] = (i + 0.5) * espaciado;
                    r[idx][1] = (j + 0.5) * espaciado;
                    idx++;
                }
            }
        }

        calcular_aceleracion_LJ(N_bench, L_bench, r, a);

        // ¡¡ARRANCA EL RELOJ DE ALTA PRECISIÓN!!
        auto inicio = chrono::high_resolution_clock::now();

        // Bucle de integración puro (Sin imprimir nada, solo estresar la CPU)
        for (int p = 0; p < pasos_medida; p++) {
            for (int i = 0; i < N_bench; i++) {
                for (int k = 0; k < 2; k++) {
                    r[i][k] += v[i][k] * h + 0.5 * a[i][k] * h * h;
                    v[i][k] += 0.5 * a[i][k] * h;
                }
                r[i][0] = fmod(r[i][0], L_bench); if (r[i][0] < 0) r[i][0] += L_bench;
                r[i][1] = fmod(r[i][1], L_bench); if (r[i][1] < 0) r[i][1] += L_bench;
            }

            calcular_aceleracion_LJ(N_bench, L_bench, r, a);

            for (int i = 0; i < N_bench; i++) {
                for (int k = 0; k < 2; k++) v[i][k] += 0.5 * a[i][k] * h;
            }
        }

        // Truco para evitar que el compilador ignore el bucle con la optimización -O3
        volatile double sum_v = v[0][0]; 

        // ¡¡PARAMOS EL RELOJ!!
        auto fin = chrono::high_resolution_clock::now();
        chrono::duration<double> duracion = fin - inicio;

        // Mostrar por pantalla y escribir simultáneamente en el archivo .txt
        cout << N_bench << "\t" << duracion.count() << " s" << endl;
        f_out << N_bench << "\t" << duracion.count() << endl;
    }

    f_out.close();
    cout << "=== BENCHMARK COMPLETADO ===" << endl;
    return 0;
}