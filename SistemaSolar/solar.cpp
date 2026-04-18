//SISTEMA SOLAR - Obligatorio
//Lo he programado con ayuda de Copilot y Gemini
//Como "ordenes", pido a las IAs que voy a usar lenguaje C++ y no mezclen con C

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <vector>

using namespace std;

//Defino unas constantes 
//Constantes globales de conversión
const double M_SOL = 1.989e30;
const double c = 1.496e11;
const double G = 6.67430e-11;

////FUNCIÓN DE REESCALADO
//Recibe unos vectores y los reescala, de acuerdo a como "pide" el Verlet
void reescalar(int N, vector<double>& m, vector<vector<double>>& r, vector<vector<double>>& v) {
    // Factor para que la velocidad sea adimensional (donde G=1)
    double tau = sqrt(pow(c,3) / (G * M_SOL));
    double factor_v = tau / c;

    for (int i =0; i < N; i++){
        //Rescalo la masa y las posiciones
        m[i] /= M_SOL;        // Masa en Masas Solares
        r[i][0] /= c;    // Posición X en UA
        r[i][1] /= c;    // Posición Y en UA
        //Hay que reescalar la velocidad
        v[i][0] *= factor_v; // Velocidad X en UA/tau
        v[i][1] *= factor_v; // Velocidad Y en UA/tau
    }
}

////FUNCIÓN DE LECTURA DE DATOS
//Servirá para leer los datos del fichero, y guardarlos en los vectores correspondientes.
void leer_datos(ifstream &data, int N, vector<double> &masa, vector<vector<double>> &x0, vector<vector<double>> &v0) {
    
    for (int i = 0; i < N; i++) {
        data >> masa[i] >> x0[i][0] >> x0[i][1] >> v0[i][0] >> v0[i][1];
    }
    return;
}

////FUNCIÓN DE CÁLCULO DE ACELERACIÓN
void calcular_aceleracion(int N, const vector<double>& m, const vector<vector<double>>& r, vector<vector<double>>& a) {
    double dx, dy, dist2, dist3;
    const double eps = 1e-10; //Añadido para evitar singularidades

    for (int i = 0; i < N; i++) {
        a[i][0] = 0.0;
        a[i][1] = 0.0;
    }

    // El planeta 0 es el Sol fijo en el origen
    for (int i = 1; i < N; i++) { // El Sol no se mueve

        // Fuerza del Sol (CORREGIDO)
        dx = r[0][0] - r[i][0];
        dy = r[0][1] - r[i][1];

        dist2 = dx * dx + dy * dy + eps;
        dist3 = dist2 * sqrt(dist2);

        a[i][0] += m[0] * dx / dist3;
        a[i][1] += m[0] * dy / dist3;

        // Interacción con otros planetas
        for (int j = 1; j < N; j++) {
            if (i != j) {
                dx = r[j][0] - r[i][0];
                dy = r[j][1] - r[i][1];

                dist2 = dx * dx + dy * dy + eps;
                dist3 = dist2 * sqrt(dist2);

                a[i][0] += m[j] * dx / dist3;
                a[i][1] += m[j] * dy / dist3;
            }
        }
    }
}
 
// FUNCIÓN PARA CALCULAR LA ENERGÍA DE CADA PLANETA Y LA TOTAL
vector<double> calcular_energias_detalladas(int N, const vector<double>& m, const vector<vector<double>>& r, const vector<vector<double>>& v) {
    vector<double> E_individual(N, 0.0);
    double E_total = 0.0;

    // Energía cinética total
    for (int i = 0; i < N; i++) {
        double v2 = v[i][0] * v[i][0] + v[i][1] * v[i][1];
        double cinetica_i = 0.5 * m[i] * v2;
        E_individual[i] = cinetica_i;
        E_total += cinetica_i;
    }

    // Energía potencial total (sin doble conteo)
    double potencial_total_sistema = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            double dx = r[j][0] - r[i][0];
            double dy = r[j][1] - r[i][1];
            double dist = sqrt(dx * dx + dy * dy);

            double potencial_ij = -(m[i] * m[j]) / dist;

            potencial_total_sistema += potencial_ij;

            // Repartimos el potencial entre ambas partículas
            E_individual[i] += 0.5 * potencial_ij;
            E_individual[j] += 0.5 * potencial_ij;
        }
    }

    E_total += potencial_total_sistema;

    E_individual.push_back(E_total); 
    
    return E_individual;
}

//FUNCIÓN PARA CALCULAR EL MOMENTO ANGULAR INDIVIDUAL Y TOTAL
vector<double> calcular_momento_angular_detallado(int N, const vector<double>& m, const vector<vector<double>>& r, const vector<vector<double>>& v) {
    vector<double> L_lista;
    double L_total = 0.0;

    for (int i = 0; i < N; i++) {
        double L_i = m[i] * (r[i][0] * v[i][1] - r[i][1] * v[i][0]);
        L_lista.push_back(L_i);
        L_total += L_i;
    }

    L_lista.push_back(L_total);
    return L_lista;
}

//FUNCIÓN PARA CALCULAR EL MOMENTO LINEAL (Px y Py) INDIVIDUAL Y TOTAL
vector<vector<double>> calcular_momento_lineal_detallado(int N, const vector<double>& m, const vector<vector<double>>& v) {
    vector<vector<double>> P_lista(2, vector<double>(N + 1, 0.0));

    for (int i = 0; i < N; i++) {
        P_lista[0][i] = m[i] * v[i][0];
        P_lista[1][i] = m[i] * v[i][1];
        P_lista[0][N] += P_lista[0][i];
        P_lista[1][N] += P_lista[1][i];
    }

    return P_lista;
}

int main() {
    int N = 10;
    double t = 0.0;
    double h_si = 14400;   // Paso de tiempo en segundos (4 hora) para la simulación, luego se reescala a adimensional
    //Este parámetro puede cambiarse, si lo pongo muy grande va más rápido pero pierde precisión, 
    //si lo pongo muy pequeño va más lento pero es más preciso. Hay que encontrar un equilibrio.
   int num_paso=1000000 // Número de pasos de la simulación, también se puede ajustar para equilibrar precisión y tiempo de ejecución
    
    vector<double> m(N);
    vector<vector<double>> r(N, vector<double>(2));
    vector<vector<double>> v(N, vector<double>(2));
    vector<vector<double>> a(N, vector<double>(2));
    
    ifstream f_in("planetas_inicial.txt");
    if (!f_in) {
        cerr << "Error: No se encuentra planetas_inicial.txt" << endl;
        return 1;
    }

    //Lee los datos del fichero de datos con las condiciones iniciales
    leer_datos(f_in, N, m, r, v);
    f_in.close();

    //Reescala las masas, posiciones y velocidades para que sean adimensionales
    reescalar(N, m, r, v);

    //Reescala también el paso de tiempo, para que sea adimensional
    double tau = sqrt(pow(c, 3) / (G * M_SOL));
    double h = h_si / tau;

    //Ficheros donde se guardarán los datos de salida, para luego graficar y animar
    ofstream f_out("data/evolucion.dat");
    ofstream f_eng("data/energia.dat");
    ofstream f_ang("data/momento_angular.dat");
    ofstream f_lin_x("data/momento_lineal_x.dat");
    ofstream f_lin_y("data/momento_lineal_y.dat");

    calcular_aceleracion(N, m, r, a);

    // Aplica el algoritmo de Verlet, además calcula energías, y momentos lineales y angulares 
    for (int n = 0; n < num_pasos; n++) {
    
        for (int i = 0; i < N; i++) {
            f_out << r[i][0] << ", " << r[i][1] << endl;
        }
        f_out << endl;

        for (int i = 1; i < N; i++) { // eL Sol no se mueve
            for (int k = 0; k < 2; k++) {
                r[i][k] += v[i][k] * h + 0.5 * a[i][k] * h * h;
                v[i][k] += 0.5 * a[i][k] * h;
            }
        }

        calcular_aceleracion(N, m, r, a);

        for (int i = 1; i < N; i++) { // el SOl no se mueve
            for (int k = 0; k < 2; k++) {
                v[i][k] += 0.5 * a[i][k] * h;
            }
        }
         
        //Se calcula el invariante energía
        vector<double> energias = calcular_energias_detalladas(N, m, r, v);
        f_eng << t;
        for (double val : energias) f_eng << "\t" << val;
        f_eng << endl;

        //Se calcula el invariante momento angular
        vector<double> momentos = calcular_momento_angular_detallado(N, m, r, v);
        f_ang << t;
        for (double val : momentos) f_ang << "\t" << val;
        f_ang << endl;
        
        //Se calcula el invariante momento lineal
        vector<vector<double>> momentos_lineales = calcular_momento_lineal_detallado(N, m, v);
        f_lin_x << t;
        for (int i = 0; i <= N; i++) f_lin_x << "\t" << momentos_lineales[0][i];
        f_lin_x << endl;
        f_lin_y << t;
        for (int i = 0; i <= N; i++) f_lin_y << "\t" << momentos_lineales[1][i];
        f_lin_y << endl;

        t += h;
    }

    cout << "Simulación terminada correctamente." << endl;

    return 0;
}
