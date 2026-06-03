// Código para simular la ecuación de Schrödinger unidimensional usando el método de Crank-Nicolson
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <cmath>
#include <string>

using namespace std;

// Definimos un alias para que el código sea más legible
typedef complex<double> dcomplex;


// Función para calcular la norma de un vector de números complejos (en particular una función de onda)
double calcular_norma(const vector<complex<double>>& phi) {
    double suma = 0.0;
    for (const auto& valor : phi) {
        suma += norm(valor); // norm(z) en C++ devuelve |z|^2
    }
    return suma;
}

// Función para calcular el potencial en la posición j
double calcular_potencial(int j, int N, double lambda, double k0_tilde) {
    // Definimos la barrera entre 2N/5 y 3N/5 según las diapositivas
    if (j >= 2 * N / 5 && j <= 3 * N / 5) {
        return lambda * pow(k0_tilde, 2);
    }
    return 0.0;
}

// Función para inicializar la función de onda phi en la posición j
complex<double> inicializar_phi(int j, int N, double k0_tilde) {
    if (j == 0 || j == N) return 0.0; // Condiciones de contorno
    
    double exponente = -8.0 * pow(4.0 * j - N, 2) / pow(N, 2);
    complex<double> fase(0, k0_tilde * j);
    
    return exp(fase) * exp(exponente);
}

// Envolvemos tu lógica en una función para llamarla varias veces
void simular_caso(int N, double lambda, string nombre_datos, string nombre_norma) {
    cout << "Simulando N=" << N << ", lambda=" << lambda << "..." << endl;

    // 1. Parámetros de la simulación
    // Estos parámetros se pueden modificar para experimentar con diferentes condiciones iniciales y potenciales
    // (N y lambda ahora los pasamos por parámetro a esta función)
    const int n_ciclos = N / 8;  // Momento inicial, define qué tan rápido se mueve la función de onda al inicio
    //Como se nos sugiere, ponemos N/8 para que se mueva a una velocidad razonable y no se disperse demasiado rápido
    
    /* El valor de lambda se puede ajustar para ver cómo afecta la transmisión y reflexión de la función de onda. 
    Un valor más alto hará que la barrera sea más difícil de atravesar, mientras que un valor más bajo permitirá una mayor transmisión.
    */
    const double PI = acos(-1.0);
    const int pasos_tiempo = 2000; // Número total de pasos de tiempo para la simulación
    const int n_D = 10;            // Guardar datos cada n_D pasos para no saturar el PC

    // 2. Variables derivadas (Cálculos previos)
    double k0_tilde = 2.0 * PI * n_ciclos / N;  // k0_tilde es el número de onda inicial, relacionado con la velocidad de la función de onda
    
    // CAMBIO CLAVE PARA ESTABILIDAD: Un valor de 0.5 asegura que la onda sea suave y no "explote"
    double s_tilde = 0.5; // s_tilde es el paso de tiempo adimensionalizado, relacionado con la estabilidad numérica de la simulación
    
    dcomplex i_unidad(0.0, 1.0); // La unidad imaginaria, se usa para manejar la parte compleja de la función de onda y los coeficientes

    // 3. Vectores del sistema
    vector<dcomplex> phi(N + 1);    // Función de onda (j = 0 hasta N) 
    vector<double> V(N + 1);        // Potencial del problema (j = 0 hasta N)
    /*
    Los siguientes vectores son para el método de Crank-Nicolson, donde alfa y gamma son coeficientes que se calculan hacia atrás y beta es el coeficiente dinámico 
    que se actualiza en cada paso de tiempo
    */
    vector<dcomplex> alfa(N);       // Coeficientes alfa (estáticos)   
    vector<dcomplex> gamma(N + 1);  // Coeficientes gamma auxiliares
    vector<dcomplex> beta(N);       // Coeficientes beta (dinámicos) 
    vector<dcomplex> chi(N + 1);    // Vector chi intermedio para el paso temporal

    // Archivos de salida
    ofstream file_data(nombre_datos); 
    /* Aquí guardaremos los datos del módulo de la función de onda |phi(j,t)|², que es la probabilidad de encontrar la partícula en la posición x en el tiempo t.
    El formato será: x, probabilidad, y cada n_D pasos de tiempo se insertará un salto de línea doble para indicar un nuevo frame temporal.
    Después graficaremos esta probabilidad mediante una animación de python usando Matplotlib, donde el eje x representará la posición y el eje y la probabilidad. 
    */
    ofstream file_norma(nombre_norma);          
     /* Aquí guardaremos, para cada n_D pasos de tiempo, la norma total de la función de onda (osea sumo |phi(j)|² para j=0,1,2,...,N), que debería ser constante 
     (idealmente 1) si el método numérico es estable y correcto. También haremos una gráfica con python de esta norma a lo largo del tiempo para verificar que 
     se mantiene constante, lo que confirmará la estabilidad y precisión de nuestro método de Crank-Nicolson.
    */

    // 4. Inicialización (instante t=0)
    for (int j = 0; j <= N; ++j) {
        V[j] = calcular_potencial(j, N, lambda, k0_tilde); 
        phi[j] = inicializar_phi(j, N, k0_tilde);
    }

    // --- NUEVO: NORMALIZACIÓN A 1 (Para que norma.dat no de 22.1557) ---
    double norma_inicial = calcular_norma(phi);
    for (int j = 0; j <= N; ++j) {
        phi[j] /= sqrt(norma_inicial);
    }

    // 5. Cálculo de coeficientes ALFA y GAMMA para el método de Crank-Nicolson
    // Estos no dependen del tiempo, se calculan una sola vez hacia atrás, como ya dijimos
    alfa[N - 1] = 0.0; 
    for (int j = N - 1; j > 0; --j) {
        // A0 = -2 + 2i/s - Vj
        dcomplex A0 = dcomplex(-2.0, 0.0) + (2.0 * i_unidad / s_tilde) - V[j];
        gamma[j] = 1.0 / (A0 + alfa[j]); // A_plus es 1 
        alfa[j - 1] = -gamma[j];         // A_minus es 1 
    }

     
// 6. BUCLE TEMPORAL para actualizar la función de onda en cada paso de tiempo usando el método de Crank-Nicolson
    for (int n = 0; n < pasos_tiempo; ++n) {
        
        // A. Cálculo de los coeficientes BETA (Hacia atrás)
        // Según la teoría: beta[j-1] = gamma[j] * (b[j] - beta[j])
        // Donde b[j] = (4i / s_tilde) * phi[j]
        beta[N - 1] = 0.0; 
        for (int j = N - 1; j > 0; --j) {
            dcomplex b_j = (4.0 * i_unidad / s_tilde) * phi[j];
            beta[j - 1] = gamma[j] * (b_j - beta[j]);
        }

        // B. Cálculo de CHI (Hacia adelante)
        // Según la teoría: chi[j] = alfa[j-1] * chi[j-1] + beta[j-1]
        chi[0] = 0.0; 
        chi[N] = 0.0;
        for (int j = 0; j < N; ++j) {
            chi[j + 1] = alfa[j] * chi[j] + beta[j];
        }

        // C. Actualización de la función de onda y cálculo de la norma
        // phi_nuevo = chi - phi_viejo
        for (int j = 0; j <= N; ++j) {
            phi[j] = chi[j] - phi[j];
        }

        // D. Guardar datos y para la norma total lo hago cada n_D pasos
        if (n % n_D == 0) {
            double norma_actual = calcular_norma(phi);
            file_norma << n << ", " << norma_actual << endl;

            // Formato para el script de Python: x, probabilidad
            for (int j = 0; j <= N; ++j) {
                file_data << j << ", " << norm(phi[j]) << endl;
            }
            // ¡IMPORTANTE! Doble salto de línea para que Python sepa que es otro frame
            file_data << endl << endl; 
        }
    }
    file_data.close();
    file_norma.close();
}

// Función principal del código
int main() {
    cout << "--- INICIANDO BATERIA DE EXPERIMENTOS ---" << endl;
    
    // 1. Estudio de N (Resolución)
    simular_caso(500,  0.5,  "datos_N500_L0.5.dat",  "norma_N500_L0.5.dat");
    simular_caso(2000, 0.5,  "datos_N2000_L0.5.dat", "norma_N2000_L0.5.dat");
    
    // 2. Estudio de Lambda (Efecto Túnel vs Reflexión)
    simular_caso(500,  0.3,  "datos_N500_L0.3.dat",  "norma_N500_L0.3.dat");
    simular_caso(500,  10.0, "datos_N500_L10.0.dat", "norma_N500_L10.0.dat");
    
    // 3. El caso intermedio
    simular_caso(1000, 1.0,  "datos_N1000_L1.0.dat", "norma_N1000_L1.0.dat");
    
    cout << "--- TODAS LAS SIMULACIONES COMPLETADAS CON EXITO ---" << endl;
    return 0;
}