# ================================================================================
# GRAFICAR MOMENTO LINEAL (MODIFICADO)
# SIN GRÁFICA DE MAGNITUD
# ================================================================================

import matplotlib.pyplot as plt
import numpy as np

# Lectura del fichero de datos para Px
data_x = []
try:
    with open('data/momento_lineal_x.dat', 'r') as f:
        for line in f:
            line = line.strip()
            if line:
                parts = [float(x) for x in line.split()]
                data_x.append(parts)
except FileNotFoundError:
    print("Error: No se encuentra el fichero 'data/momento_lineal_x.dat'")
    exit(1)

# Lectura del fichero de datos para Py
data_y = []
try:
    with open('data/momento_lineal_y.dat', 'r') as f:
        for line in f:
            line = line.strip()
            if line:
                parts = [float(x) for x in line.split()]
                data_y.append(parts)
except FileNotFoundError:
    print("Error: No se encuentra el fichero 'data/momento_lineal_y.dat'")
    exit(1)

if not data_x or not data_y:
    print("Error: No se pudieron leer datos")
    exit(1)

nbodies = 10

# Extraer datos
times = []
momentum_x = [[] for _ in range(nbodies)]
momentum_y = [[] for _ in range(nbodies)]
total_momentum_x = []
total_momentum_y = []

for row_x, row_y in zip(data_x, data_y):
    times.append(row_x[0])
    for i in range(nbodies):
        momentum_x[i].append(row_x[1 + i])
        momentum_y[i].append(row_y[1 + i])
    total_momentum_x.append(row_x[1 + nbodies])
    total_momentum_y.append(row_y[1 + nbodies])

print("Datos leídos correctamente")

bodies = ['Sol', 'Mercurio', 'Venus', 'Tierra', 'Marte',
          'Júpiter', 'Saturno', 'Urano', 'Neptuno', 'Plutón']

# ============================
# GRÁFICA Px
# ============================
plt.figure(figsize=(14, 7))

for i in range(nbodies):
    plt.plot(times, momentum_x[i], label=bodies[i], linewidth=1.2)

plt.plot(times, total_momentum_x, label='P_x_total',
         linewidth=2.5, color='red', linestyle='--')

plt.xlabel('Tiempo')
plt.ylabel('Momento lineal X (Px)')
plt.title('Momento lineal X')
plt.legend()
plt.grid(alpha=0.3)
plt.tight_layout()
plt.savefig('momento_lineal_x.png', dpi=300)
plt.close()

# ============================
# GRÁFICA Py
# ============================
plt.figure(figsize=(14, 7))

for i in range(nbodies):
    plt.plot(times, momentum_y[i], label=bodies[i], linewidth=1.2)

plt.plot(times, total_momentum_y, label='P_y_total',
         linewidth=2.5, color='red', linestyle='--')

plt.xlabel('Tiempo')
plt.ylabel('Momento lineal Y (Py)')
plt.title('Momento lineal Y')
plt.legend()
plt.grid(alpha=0.3)
plt.tight_layout()
plt.savefig('momento_lineal_y.png', dpi=300)
plt.close()

# ============================
# ESTADÍSTICAS
# ============================
print("\n" + "="*50)
print("ESTADÍSTICAS DEL MOMENTO LINEAL")
print("="*50)

print(f"Px inicial: {total_momentum_x[0]:.3e}")
print(f"Px final:   {total_momentum_x[-1]:.3e}")
print(f"ΔPx:        {abs(total_momentum_x[-1] - total_momentum_x[0]):.3e}")
print()

print(f"Py inicial: {total_momentum_y[0]:.3e}")
print(f"Py final:   {total_momentum_y[-1]:.3e}")
print(f"ΔPy:        {abs(total_momentum_y[-1] - total_momentum_y[0]):.3e}")

print("="*50)

print("\nGráficas generadas:")
print(" - momento_lineal_x.png")
print(" - momento_lineal_y.png")