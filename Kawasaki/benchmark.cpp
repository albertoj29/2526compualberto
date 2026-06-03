/* ==========================================================================
   BENCHMARK DE ESCALABILIDAD - MODELO DE KAWASAKI
   VAMOS A COMPARAR EL RENDIMIENTO DE NUESTRO CÓDIGO DE KAWASAKI,
   EJECUTÁNDOLO EN MI ORDENADOR CON EL COMANDO: "g++ -O0 benchmark.cpp -o bench_lento
 ./bench_lento tiempos_lento.txt" VS ACELERÁNDOLO CON: "g++ -O3 benchmark.cpp -o bench_rapido"
   RECALCAR QUE PARA HACER LAS ACTIVIDADES PEDIDAS Y EL CÓDIGO PRINCIPAL "Kawasaki.cpp" USÉ
   LA OPTIMIZACIÓN -O3 PARA QUE CORRIERA MÁS RÁPIDO
   ========================================================================== */

#include <iostream> 
#include <fstream>  
#include <cmath>    
#include <random>   
#include <algorithm>
#include <string>   
#include <vector>   
#include <chrono>   // MUY IMPORTANTE: Librería para medir tiempos de alta precisión

using namespace std;

// Generadores aleatorios globales
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<double> dist01(0.0, 1.0); 

// Función de inicialización respetando bordes fijos y m0
void inicializarRed(vector<vector<int>>& s, int N, double m0) {
    for (int j = 0; j < N; j++) {
        s[0][j] = -1;       
        s[N-1][j] = 1;      
    }
    int sitios_interiores = N * (N - 2);
    double target_plus = (sitios_interiores + m0 * N * N) / 2.0;
    int n_plus = round(target_plus);

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
    // Agitar el interior
    uniform_int_distribution<int> distX(0, N - 1);
    uniform_int_distribution<int> distY(1, N - 2);
    for (int k = 0; k < 100000; k++) {
        int i1 = distY(gen), j1 = distX(gen);
        int i2 = distY(gen), j2 = distX(gen);
        swap(s[i1][j1], s[i2][j2]);
    }
}

// Cálculo de energía de intercambio (Kawasaki)
double calcularDeltaE_Kawasaki(const vector<vector<int>>& s, int i1, int j1, int i2, int j2, int N) {
    if (s[i1][j1] == s[i2][j2]) return 0.0;

    int S1 = 0;
    if (!(i1-1 == i2 && j1 == j2)) S1 += s[i1-1][j1];             
    if (!(i1+1 == i2 && j1 == j2)) S1 += s[i1+1][j1];             
    if (!(i1 == i2 && (j1-1+N)%N == j2)) S1 += s[i1][(j1-1+N)%N]; 
    if (!(i1 == i2 && (j1+1)%N == j2)) S1 += s[i1][(j1+1)%N];     

    int S2 = 0;
    if (!(i2-1 == i1 && j2 == j1)) S2 += s[i2-1][j2];
    if (!(i2+1 == i1 && j2 == j1)) S2 += s[i2+1][j2];
    if (!(i2 == i1 && (j2-1+N)%N == j1)) S2 += s[i2][(j2-1+N)%N];
    if (!(i2 == i1 && (j2+1)%N == j1)) S2 += s[i2][(j2+1)%N];

    return (s[i1][j1] - s[i2][j2]) * (S1 - S2);
}

// Paso de Monte Carlo (N*N intentos de intercambio)
int realizarPasoMonteCarloKawasaki(vector<vector<int>>& s, double T, int N) {
    int cambios = 0;
    uniform_int_distribution<int> distX(0, N - 1);
    uniform_int_distribution<int> distY(1, N - 2);
    uniform_int_distribution<int> distDir(0, 3);

    for (int n_intentos = 0; n_intentos < N * N; n_intentos++) {
        int i1 = distY(gen);
        int j1 = distX(gen);

        int dir = distDir(gen);
        int i2 = i1, j2 = j1;
        
        if (dir == 0) i2 = i1 - 1;             
        else if (dir == 1) i2 = i1 + 1;        
        else if (dir == 2) j2 = (j1 - 1 + N)%N;
        else if (dir == 3) j2 = (j1 + 1)%N;    

        if (i2 == 0 || i2 == N - 1) continue; 

        double dE = calcularDeltaE_Kawasaki(s, i1, j1, i2, j2, N);
        if (dE <= 0 || dist01(gen) < exp(-dE / T)) {
            swap(s[i1][j1], s[i2][j2]);
            cambios++;
        }
    }
    return cambios;
}

// Cálculo de la energía total
double calcularEnergiaTotal(const vector<vector<int>>& s, int N) {
    double E = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int espin = s[i][j];
            E += -espin * s[i][(j + 1) % N];
            if (i + 1 < N) {
                E += -espin * s[i + 1][j];
            }
        }
    }
    return E;
}

// PROGRAMA PRINCIPAL CON CRONÓMETRO Y SALIDA A ARCHIVO DILIGENTE
int main(int argc, char* argv[]) {
    // Verificamos si pasaste el nombre del archivo por terminal
    string nombre_archivo_salida = "tiempos_defecto.txt";
    if (argc > 1) {
        nombre_archivo_salida = argv[1];
    }

    // Configuración fija del test de estrés computacional
    int pasos_termalizacion = 1000;  
    int pasos_medida = 1000;         
    double T = 2.269; // Temperatura crítica teórica
    double m0 = 0.0;
    
    vector<int> lista_tamanos = {16, 32, 64, 128};

    // Abrimos el archivo donde guardaremos los resultados directamente
    ofstream archivoOut(nombre_archivo_salida);
    if (!archivoOut.is_open()) {
        cerr << "Error creando el archivo de salida." << endl;
        return 1;
    }

    cout << "=== INICIANDO BENCHMARK DE KAWASAKI ===" << endl;
    cout << "Guardando resultados en: " << nombre_archivo_salida << endl;
    cout << "N\tTiempo (s)" << endl;

    for (int N : lista_tamanos) {
        // Inicializamos las estructuras necesarias antes de activar el reloj
        vector<vector<int>> s(N, vector<int>(N));
        inicializarRed(s, N, m0);

        // ¡¡ARRANCA EL RELOJ DE ALTA PRECISIÓN!!
        auto inicio = chrono::high_resolution_clock::now();

        // 1. Bucle de Termalización masivo
        for (int p = 0; p < pasos_termalizacion; p++) {
            realizarPasoMonteCarloKawasaki(s, T, N);
        }

        // 2. Bucle de Medida (Cálculo matemático puro sin guardar en disco)
        for (int p = 0; p < pasos_medida; p++) {
            realizarPasoMonteCarloKawasaki(s, T, N);
            volatile double E_dummy = calcularEnergiaTotal(s, N); // Fuerza al procesador a calcular la energía
        }

        // ¡¡PARAMOS EL RELOJ!!
        auto fin = chrono::high_resolution_clock::now();
        chrono::duration<double> duracion = fin - inicio;

        // Mostrar por pantalla y escribir simultáneamente en el archivo .txt
        cout << N << "\t" << duracion.count() << " s" << endl;
        archivoOut << N << "\t" << duracion.count() << "\n";
    }

    archivoOut.close();
    cout << "=== BENCHMARK FINALIZADO EXITOSAMENTE ===" << endl;
    return 0;
}