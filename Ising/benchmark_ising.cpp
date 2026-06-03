#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <chrono> // Librería para medir el tiempo de ejecución

using namespace std;

// Generadores aleatorios globales
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<double> dist01(0.0, 1.0);

// Función para calcular la diferencia de energía (DeltaE)
double calcularDeltaE(const vector<vector<int>>& s, int i, int j, int N) {
    int arriba = s[(i - 1 + N) % N][j]; 
    int abajo  = s[(i + 1) % N][j];
    int izq    = s[i][(j - 1 + N) % N];
    int der    = s[i][(j + 1) % N];

    int suma_vecinos = arriba + abajo + izq + der;
    return 2.0 * s[i][j] * (double)suma_vecinos;
}

double minimo(double a, double b) {
    return (a < b) ? a : b;
}

// Paso de Monte Carlo adaptado a tamaño N dinámico
int realizarPasoMonteCarlo(vector<vector<int>>& s, double T, int N) {
    int cambios = 0;
    uniform_int_distribution<int> distN(0, N - 1); // Generador adaptado al tamaño actual

    for (int n = 0; n < N * N; n++) {
        int i = distN(gen);
        int j = distN(gen);

        double dE = calcularDeltaE(s, i, j, N);
        double p = minimo(1.0, exp(-dE / T));

        if (dist01(gen) < p) {
            s[i][j] = -s[i][j];
            cambios++; 
        }
    }
    return cambios;
}

int main() {
    // Parámetros del benchmark (Iguales a los que usamos en Kawasaki para poder comparar)
    vector<int> N_vals = {16, 32, 64, 128};
    double T_critica = 2.27; // Temperatura crítica teórica
    int pasos_totales = 2000; // 1000 termalización + 1000 medida

    cout << "========================================================\n";
    cout << " BENCHMARK ISING 2D (DINAMICA DE GLAUBER) a T = 2.27\n";
    cout << "========================================================\n";

    for (int N : N_vals) {
        // Inicializamos la red para el tamaño N
        vector<vector<int>> s(N, vector<int>(N, 1)); 

        // Arrancamos el cronómetro
        auto inicio = chrono::high_resolution_clock::now();

        // Ejecutamos la simulación bruta (sin imprimir nada en pantalla para no falsear el tiempo)
        for (int p = 0; p < pasos_totales; p++) {
            realizarPasoMonteCarlo(s, T_critica, N);
        }

        // Paramos el cronómetro
        auto fin = chrono::high_resolution_clock::now();
        chrono::duration<double> duracion = fin - inicio;

        // Imprimimos el resultado EXACTAMENTE en el formato que lee el script de Python
        cout << "N = " << N << "\t|\tTiempo : " << duracion.count() << " segundos" << endl;
    }

    return 0;
}