import matplotlib.pyplot as plt
import numpy as np
import os
from matplotlib.ticker import NullFormatter # Importación nueva para borrar etiquetas superpuestas

# Tamaños de red que evaluaremos
N_vals = [16, 32, 64, 128]

# Función para leer los tiempos de los archivos generados por C++
def leer_tiempos(nombre_archivo):
    tiempos = []
    if os.path.exists(nombre_archivo):
        with open(nombre_archivo, 'r') as f:
            for linea in f:
                if "Tiempo" in linea:
                    # Extraemos el valor numérico (segundos)
                    tiempos.append(float(linea.split()[6]))
    else:
        print(f"Aviso: No se encuentra {nombre_archivo}")
        tiempos = [0, 0, 0, 0] # Relleno de seguridad
    return tiempos

# Leer los 4 escenarios
t_local_O0 = leer_tiempos('tiempos_local_O0.txt')
t_local_O3 = leer_tiempos('tiempos_local_O3.txt')
t_joel_O0 = leer_tiempos('tiempos_joel_O0.txt')
t_joel_O3 = leer_tiempos('tiempos_joel_O3.txt')

# Crear la gráfica
plt.figure(figsize=(10, 6))

plt.plot(N_vals, t_local_O0, 'o-', color='red', label='Local (VM) -O0', linewidth=2)
plt.plot(N_vals, t_local_O3, 'o-', color='darkred', label='Local (VM) -O3', linewidth=2)
plt.plot(N_vals, t_joel_O0, 's--', color='blue', label='Clúster JOEL -O0', linewidth=2)
plt.plot(N_vals, t_joel_O3, 's--', color='darkblue', label='Clúster JOEL -O3', linewidth=2)

# Escala logarítmica
plt.yscale('log')
plt.xscale('log', base=2) # Ajustado a base 2
plt.xticks(N_vals, labels=[str(n) for n in N_vals])


plt.gca().xaxis.set_minor_formatter(NullFormatter()) 

plt.title('Rendimiento y Escalabilidad: Modelo de Ising (Dinámica de Glauber)', fontsize=14, fontweight='bold')
plt.xlabel('Tamaño de la Red (N)', fontsize=12)
plt.ylabel('Tiempo de CPU (segundos)', fontsize=12)
plt.grid(True, which="both", ls="--", alpha=0.5)
plt.legend(fontsize=11)

plt.tight_layout()
# Lo guardo como jpg igual que me lo has subido
plt.savefig('grafica_rendimiento_ising.jpg', dpi=300) 
print("¡Gráfica generada y guardada como 'grafica_rendimiento_ising.jpg' limpia y sin superposiciones!")
plt.show()