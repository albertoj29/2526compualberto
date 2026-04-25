/* ALGORITMO DE METROPOLIS PARA EL MODELO DE ISING 2D:
   -------------------------------------------------
   Para generar configuraciones típicas con probabilidad de equilibrio P(S) 
   utilizaremos los siguientes pasos:

   (0) Configuración Inicial:
       - Dar una T (Temperatura). 
       - Generar una configuración de espines (Ej: s(i,j) = 1 para todo i,j).

   (1) Selección: 
       - Elegir un punto al azar (n, m) de la red.

   (2) Evaluación de Probabilidad:
       - Evaluar p = min(1, exp[-DeltaE/T]) 
       - Donde DeltaE = 2*s(n,m) * [s(n+1,m) + s(n-1,m) + s(n,m+1) + s(n,m-1)]
       - IMPORTANTE: Usar condiciones de contorno periódicas.

   (3) Aceptación/Rechazo:
       - Generar un número aleatorio uniforme xi en [0, 1].
       - Si xi < p, entonces cambiar el signo del espín: s(n, m) = -s(n, m).

   (4) Iteración:
       - Repetir los pasos anteriores para evolucionar el sistema.
*/

#include <iostream> // Para entrada/salida
#include <fstream>  // Para escribir resultados en un archivo
#include <cmath>    // Para funciones matemáticas
#include <random>   // Para generar números aleatorios
#include <algorithm>// Para poder usar la función min()
#include <string>   // NECESARIO para usar to_string y nombres de archivos

using namespace std;

// Definimos el tamaño de la red de forma global para que sea fácil de cambiar
const int N = 32; // Recordad que la red es tamaño N x N

// Configuramos el generador de números aleatorios 
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<double> dist01(0.0, 1.0); // Genera entre 0 y 1
uniform_int_distribution<int> distN(0, N - 1);      // Genera índices entre 0 y N-1

// Función para calcular la diferencia de energía (DeltaE del Paso (2))
double calcularDeltaE(int s[N][N], int i, int j) {
    // Buscamos los vecinos con condiciones periódicas
    // (i+1)%N nos da el de la derecha, y (i-1+N)%N el de la izquierda
    int arriba = s[(i - 1 + N) % N][j]; 
    int abajo  = s[(i + 1) % N][j];
    int izq    = s[i][(j - 1 + N) % N];
    int der    = s[i][(j + 1) % N];

    // Sumamos los 4 vecinos
    int suma_vecinos = arriba + abajo + izq + der;

    // Aplicamos la fórmula final para DeltaE: 2 * espín_central * suma_vecinos
    return 2.0 * s[i][j] * (double)suma_vecinos;
}
// He usado el operador % (módulo) para implementar las condiciones periódicas  
// Este operador lo que hace es devolver el resto de una división entera
// Por ejemplo, si i = 0, entonces (i - 1 + N) % N = (0 - 1 + 32) % 32 = 31, lo que nos da el índice del vecino de arriba en la red.


// Función auxiliar para calcular el mínimo entre dos números
// Nota: Alternativamente se podría haber usado la librería <algorithm> y la función std::min()
double minimo(double a, double b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}


// Para aplicar los pasos (1), (2) y (3), diseño la siguiente función que se encargará de realizar un paso completo 
// de Monte Carlo sobre la red de espines s a una temperatura T dada.
void realizarPasoMonteCarlo(int s[N][N], double T) {
    for (int n = 0; n < N * N; n++) {
        // Paso (1): Selección de un punto al azar (i, j) (índices de la red)
        int i = distN(gen);
        int j = distN(gen);
        // La función distN(gen) me genera un número entero aleatorio entre 0 y N-1, lo que nos da un índice válido para la red de espines.

        // Paso (2): Evalúo de la probabilidad de aceptación
        double dE = calcularDeltaE(s, i, j); // Calculamos la DeltaE para el espín seleccionado
        
        // Aplicamos la fórmula: p = min(1, exp(-dE/T)) usando mi propia función minimo
        double p = minimo(1.0, exp(-dE / T));

        // Paso (3): Aceptación/Rechazo
        // Ahora comparamos directamente el número aleatorio con p
        if (dist01(gen) < p) {
            s[i][j] = -s[i][j]; // Aceptamos el cambio
        }
        // dist01(gen) me genera un número aleatorio entre 0 y 1, y lo comparo con p para decidir si acepto el cambio de signo del espín o no.
    }
}



// Función principal donde se ejecuta la simulación de Ising y se generan los archivos de salida para la animación y la magnetización.
int main() {
    // Definimos la red de espines
    int s[N][N];
    int k_animacion = 200; //Número de pasos de Monte Carlo que se hacen para LA ANIMACION (ESTO PUEDE CAMBIARSE)
    int k_mag = 1000; // Número de pasos de Monte Carlo para alcanzar el equilibrio antes de medir la magnetización (ESTO TAMBIÉN PUEDE CAMBIARSE)
    /*
    ¡OJO!
    En mi código de la Parte 2, he definido una variable llamada k_mag con un valor de 1000. Quería hacer que el sistema evolucione 10⁶ pasos, 
    pero hay que tener en cuenta cómo funciona mi función realizarPasoMonteCarlo.
    Dentro de esa función, ya tengo un bucle que se repite N×N veces (que en mi caso, como la red es de 32×32, son 1024 intentos individuales). 
    Por tanto, cada vez que llamo a la función una sola vez, ya estoy intentando cambiar todos los espines de la red.
    Al poner k_mag = 1000 en el main, lo que estoy haciendo realmente es multiplicar esos 1000 pasos de red completa por los 1024 espines que tiene mi red. 
    El resultado total es:
    1000×1024=1.024.000 intentos de volteo
    Con esto consigo cumplir exactamente con el requisito del millón de pasos. De esta forma, me aseguro de que el sistema tenga tiempo suficiente para 'termalizar' 
    (es decir, que llegue al equilibrio) antes de calcular la magnetización final para cada temperatura. 
    */

    // PARTE 1: GENERACIÓN DE DATOS PARA LAS ANIMACIONES PARA CALCULAR LA TEMPERATURA CRÍTICA
    //Recalcar que la temperatura crítica del modelo de Ising 2D sin campo magnético externo es aproximadamente T_c ≈ 2.27 (en unidades donde k_B = 1 y J = 1).
    //(ésto fue calculado por Lars Osanger en 1944, y es un resultado teórico muy importante en física estadística)
    // Definimos las 5 temperaturas específicas que queremos observar 
    double temperaturas[] = {1.25, 1.75, 2.25, 2.75, 3.25};
    // Después con el código de python hago animaciones del sistema para estas temperaturas, y así podré ver cuál es la temperatura crítica
    
    for (int t = 0; t < 5; t++) {
        double T_actual = temperaturas[t];
        
        // Creo un nombre de archivo que indique la temperatura
        // Ejemplo: ising_T_1.25.dat
        string nombreArchivo = "ising_T_" + to_string(T_actual).substr(0, 4) + ".dat";
        ofstream archivoAnim(nombreArchivo);
        
        cout << "Generando animacion para T = " << T_actual << "..." << endl;

        // Paso (0): Configuración inicial ordenada (todos los espines hacia arriba, osea + 1)
        // CABE RECALCAR QUE ESTO PUEDE CAMBIARSE, es decir también podríamos haber puesto todos los espines hacia abajo, osea -1
        for (int i_red = 0; i_red < N; i_red++) {
            for (int j_red = 0; j_red < N; j_red++) {
                s[i_red][j_red] = 1;
            }
        }

        // Utilizamos la variable k_animacion que definimos antes
        for (int paso = 0; paso < k_animacion; paso++) {
            realizarPasoMonteCarlo(s, T_actual);

            // Guardamos el estado de la red en el archivo
            for (int r = 0; r < N; r++) {
                for (int c = 0; c < N; c++) {
                    archivoAnim << s[r][c] << (c == N - 1 ? "" : ",");
                }
                archivoAnim << "\n";
            }
            archivoAnim << "\n"; // Separador de fotogramas
        }
        archivoAnim.close();
    }
    cout << "Los 5 archivos de animacion han sido creados con " << k_animacion << " pasos cada uno." << endl;
    
    
    // PARTE 2: CALCULO DE MAGNETIZACIÓN VS TEMPERATURA. REPRESENTACIÓN GRÁFICA
    // Aquí haremos un barrido de temperaturas para ver la transición de fase
    // Para esto, vamos a generar un archivo de texto con dos columnas: Temperatura (T) y Magnetización (M)
  ofstream archivoMag("magnetizacion.txt");
    
    cout << "\nCalculando magnetizacion por temperaturas (Barrido)..." << endl;

    // Hacemos el bucle de temperaturas desde 1.0 hasta 4.0
    // Incrementamos de 0.2 en 0.2 para tener una buena resolución cerca de la temperatura crítica (que es aproximadamente 2.25, como vimos tras hacer las animaciones de la parte 1)
    // De hecho la temperatura crítica teórica, como ya dije, es 2.27 y fue calculada por Lars Osanger en 1944
    for (double T = 1.0; T <= 4.0; T += 0.2) {
        
        // Reiniciamos la red a estado ordenado (+1) para cada nueva temperatura
        // Esto es importante para asegurar que cada temperatura comience desde la misma configuración inicial, lo que nos permitirá comparar los resultados de manera justa.
        for (int i_m = 0; i_m < N; i_m++) {
            for (int j_m = 0; j_m < N; j_m++) {
                s[i_m][j_m] = 1;
            }
        }

        // Dejamos que el sistema se estabilice (termalización)
        // Utilizamos la variable k_mag para controlar la precisión
        //  Cuantos más pasos de Monte Carlo hagamos, más cerca estaremos del equilibrio, pero también tardará más en ejecutarse el programa.
        for (int p_mag = 0; p_mag < k_mag; p_mag++) {
            realizarPasoMonteCarlo(s, T);
        }

        // Una vez en equilibrio, medimos la magnetización absoluta media: M = |sum(s_i)| / N^2
        // Sumamos todos los espines de la red para obtener la magnetización total, y luego dividimos por el número total de sitios (N^2) para obtener la magnetización por posición.
        double suma_s = 0;
        for (int r_m = 0; r_m < N; r_m++) {
            for (int c_m = 0; c_m < N; c_m++) {
                suma_s += s[r_m][c_m];
            }
        }
        // Calculamos la magnetización por posición (en valor absoluto)
        // Usamos (double) para asegurar que la división no trunque decimales
        double m = abs(suma_s) / (double)(N * N);

        archivoMag << T << "\t" << m << endl;
        cout << "T = " << T << " | Magnetizacion M = " << m << " (Completado)" << endl;
    }
    
    archivoMag.close();
    cout << "\nSimulacion terminada. El archivo 'magnetizacion.txt' esta listo." << endl;


    return 0;
}