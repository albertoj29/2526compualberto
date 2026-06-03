# PROGRAMA PARA GENERAR LA GRÁFICA DE MAGNETIZACIÓN VS TEMPERATURA A PARTIR DE LOS DATOS OBTENIDOS EN ISING.CPP
import numpy as np
import matplotlib.pyplot as plt

# Cargamos los datos: T es la primera columna y M la segunda
try:
    T, M = np.loadtxt('magnetizacion.txt', unpack=True)
except FileNotFoundError:
    print("Error: No se encuentra 'magnetizacion.txt'. Ejecuta primero el programa C++.")
    exit()


# --- SECCIÓN: CÁLCULO DE AJUSTES FÍSICAMENTE REALISTAS Y Tc EXPERIMENTAL ---

Tc_teorica = 2.269

# 1. MÁSCARAS REALISTAS: 
# Izquierda: Atrapamos SOLAMENTE el precipicio más inclinado
mascara_izq_ajuste = (T >= 2.0) & (T <= 2.4)
# Derecha: Atrapamos la zona paramagnética pura (ruido térmico estable) a partir de 2.6
mascara_der_ajuste = T >= 2.6 

# 2. ECUACIONES DE AJUSTE:
# Logarítmico para la caída ferromagnética
a_izq, b_izq = np.polyfit(np.log(T[mascara_izq_ajuste]), M[mascara_izq_ajuste], 1)
# Constante para el ruido paramagnético
valor_paramagnetico = np.mean(M[mascara_der_ajuste]) 

# 3. CÁLCULO DE LA Tc EXPERIMENTAL (Intersección)
ln_Tc_exp = (valor_paramagnetico - b_izq) / a_izq
Tc_exp = np.exp(ln_Tc_exp)

print(f"--- RESULTADOS DEL AJUSTE ---")
print(f"Temperatura Crítica Teórica: {Tc_teorica}")
print(f"Temperatura Crítica Experimental (Cruce de ajustes): {Tc_exp:.3f}")

# 4. CURVAS TEÓRICAS PARA PINTAR (Las generamos en un rango visual para que se crucen)
T_visual_izq = T[T <= 2.5]
M_visual_izq = a_izq * np.log(T_visual_izq) + b_izq

T_visual_der = T[T >= 1.8]
M_visual_der = np.full_like(T_visual_der, valor_paramagnetico)

# 5. CÁLCULO DE ERRORES (¡SOLO EN LOS PUNTOS DEL AJUSTE!)
# Así evitamos que las barras se disparen en la zona de la meseta ferromagnética
M_fit_izq_puntos = a_izq * np.log(T[mascara_izq_ajuste]) + b_izq
error_izq = np.abs(M[mascara_izq_ajuste] - M_fit_izq_puntos)

M_fit_der_puntos = np.full_like(T[mascara_der_ajuste], valor_paramagnetico)
error_der = np.abs(M[mascara_der_ajuste] - M_fit_der_puntos)

# -------------------------------------------------------------------------


# Creamos la figura de la gráfica
plt.figure(figsize=(10, 7))

# Pintamos las barras de error (residuos) SOLO donde los modelos son válidos
plt.errorbar(T[mascara_izq_ajuste], M[mascara_izq_ajuste], yerr=error_izq, fmt='none', ecolor='gray', capsize=3, label='Residuos (Error al ajuste)', alpha=0.7)
plt.errorbar(T[mascara_der_ajuste], M[mascara_der_ajuste], yerr=error_der, fmt='none', ecolor='gray', capsize=3, alpha=0.7)

# Pintamos los ajustes
plt.plot(T_visual_izq, M_visual_izq, 'g--', linewidth=2, label='Ajuste Logarítmico (Caída Ferromagnética)')
plt.plot(T_visual_der, M_visual_der, 'm--', linewidth=2, label='Línea base (Ruido Paramagnético)')

# Pintamos los datos medidos
plt.plot(T, M, 'bo-', label='Magnetización medida')

# Líneas verticales de las Temperaturas Críticas
plt.axvline(x=Tc_teorica, color='red', linestyle='-', alpha=0.5, label='Temperatura Crítica teórica ($T_c \\approx 2.27$)')
plt.axvline(x=Tc_exp, color='green', linestyle='-.', label=f'Temperatura Crítica Experimental ($T_c^{{exp}} \\approx {Tc_exp:.2f}$)')

# Títulos y etiquetas
plt.title('Objetivo 2: Magnetización vs Temperatura (Red 32x32) con Ajustes', fontsize=14)
plt.xlabel('Temperatura (T)', fontsize=12)
plt.ylabel('Magnetización Media |M|', fontsize=12)

# Límites y rejilla para enfocar bien el resultado
plt.ylim(-0.1, 1.2)
plt.xlim(0.8, 4.2)
plt.grid(True, alpha=0.3)
plt.legend()

# Guardamos
print("Generando imagen 'grafica_magnetizacion_ajustes.png'...")
plt.savefig('grafica_magnetizacion_ajustes.png', dpi=300) 
plt.show()