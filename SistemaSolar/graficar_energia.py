# ================================================================================
# GRAFICAR ENERGÍA
#
# Lee los datos de energía guardados en energia.dat
# y genera gráficos para verificar que se conserva durante la simulación.
#
# ================================================================================

import matplotlib.pyplot as plt
import numpy as np

# Lectura del fichero de datos
# ========================================
data = []
try:
    with open('data/energia.dat', 'r') as f:
        for line in f:
            line = line.strip()
            if line:
                parts = [float(x) for x in line.split()]
                data.append(parts)
except FileNotFoundError:
    print("Error: No se encuentra el fichero 'data/energia.dat'")
    print("Asegúrate de que primero has compilado y ejecutado solar.cpp")
    exit(1)

if not data:
    print("Error: No se pudieron leer datos del fichero")
    exit(1)

# Estructura: cada línea tiene [t, E_1, E_2, ..., E_10, E_total]
# Donde E_i es la energía del cuerpo i (cinética + potencial)
# y E_total es la energía total del sistema
nbodies = 10

# Extraer datos
times = []
energies = [[] for _ in range(nbodies)]
total_energy = []

for row in data:
    times.append(row[0])
    for i in range(nbodies):
        energies[i].append(row[1 + i])
    total_energy.append(row[1 + nbodies])

print(f"Datos leídos correctamente")
print(f"  - Número de cuerpos: {nbodies}")
print(f"  - Número de iteraciones: {len(times)}")

# Nombres de los cuerpos celestes
bodies = ['Sol', 'Mercurio', 'Venus', 'Tierra', 'Marte', 'Júpiter', 'Saturno', 'Urano', 'Neptuno', 'Plutón'][:nbodies]

# Graficar energía de cada cuerpo
# ========================================
plt.figure(figsize=(14, 7))
for i in range(nbodies):
    plt.plot(times, energies[i], label=bodies[i], linewidth=1.5, alpha=0.8)

plt.xlabel('Tiempo (unidades adimensionales)', fontsize=12)
plt.ylabel('Energía (E)', fontsize=12)
plt.title('Energía individual de cada cuerpo vs Tiempo', fontsize=14)
plt.legend(loc='best', fontsize=10)
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('energia_individual.png', dpi=300, bbox_inches='tight')
print("Gráfico guardado: energia_individual.png")
plt.close()

# Graficar energía TOTAL
# ========================================
plt.figure(figsize=(14, 6))
plt.plot(times, total_energy, color='red', linewidth=2.5, label='E_total')
plt.xlabel('Tiempo (unidades adimensionales)', fontsize=12)
plt.ylabel('Energía total (E_total)', fontsize=12)
plt.title('Energía TOTAL del sistema (debe ser CONSTANTE)', fontsize=14)
plt.grid(True, alpha=0.3)
plt.legend(fontsize=11)
plt.tight_layout()
plt.savefig('energia_total.png', dpi=300, bbox_inches='tight')
print("Gráfico guardado: energia_total.png")
plt.close()

# Graficar energía total e individual en la misma gráfica
# ========================================
plt.figure(figsize=(14, 7))
for i in range(nbodies):
    plt.plot(times, energies[i], label=bodies[i], linewidth=1, alpha=0.5)

# Graficar el total con línea más gruesa y diferente color
plt.plot(times, total_energy, label='E_total', linewidth=3, color='red', linestyle='--')

plt.xlabel('Tiempo (unidades adimensionales)', fontsize=12)
plt.ylabel('Energía (E)', fontsize=12)
plt.title('Energía: componente de cada cuerpo y total vs Tiempo', fontsize=14)
plt.legend(loc='best', fontsize=10)
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('energia_comparacion.png', dpi=300, bbox_inches='tight')
print("Gráfico guardado: energia_comparacion.png")
plt.close()

# Estadísticas
# ========================================
print("\n" + "="*60)
print("ESTADÍSTICAS DE LA ENERGÍA")
print("="*60)
print(f"E_total inicial: {total_energy[0]:.10e}")
print(f"E_total final: {total_energy[-1]:.10e}")
print(f"Variación total: {abs(total_energy[-1] - total_energy[0]):.10e}")
print(f"Variación relativa: {abs(total_energy[-1] - total_energy[0])/abs(total_energy[0])*100:.6f}%")
print("="*60)

print("\n¡Todos los gráficos han sido generados correctamente!")
print("Archivos creados:")
print("  - energia_individual.png")
print("  - energia_total.png")
print("  - energia_comparacion.png")
