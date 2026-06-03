import matplotlib.pyplot as plt
import numpy as np

# Función auxiliar para leer el tiempo flotante dentro de cada archivo txt
def buscar_tiempo(nombre_archivo):
    try:
        with open(nombre_archivo, 'r') as f:
            for linea in f:
                if "Tiempo de ejecucion:" in linea:
                    # Extrae el número que va entre la frase y la palabra 'segundos'
                    return float(linea.split(":")[1].split()[0])
    except FileNotFoundError:
        print(f"¡Ojo! No se encontró el archivo: {nombre_archivo}. Se usará un valor temporal.")
        return 0.0

# 1. Leer los tiempos de los archivos de texto
t_local_O0   = buscar_tiempo('tiempo_local_O0.txt')
t_local_O3   = buscar_tiempo('tiempo_local_O3.txt')
t_cluster_O0 = buscar_tiempo('tiempo_cluster_O0.txt')
t_cluster_O3 = buscar_tiempo('tiempo_cluster_O3.txt')

tiempos_O0 = [t_local_O0, t_cluster_O0]
tiempos_O3 = [t_local_O3, t_cluster_O3]

# 2. Configuración del gráfico de barras doble
etiquetas = ['Máquina Local (VirtualBox)', 'Clúster JOEL']
x = np.arange(len(etiquetas))  # Localización de los grupos
ancho = 0.35                  # Ancho de las barras

fig, ax = plt.subplots(figsize=(8, 5.5), dpi=100)

# Pintar las barras de -O0 y -O3
barras_O0 = ax.bar(x - ancho/2, tiempos_O0, ancho, label='Sin optimizar (-O0)', color='#e74c3c', edgecolor='black')
barras_O3 = ax.bar(x + ancho/2, tiempos_O3, ancho, label='Optimización Máxima (-O3)', color='#2ecc71', edgecolor='black')

# Añadir títulos y etiquetas estéticas
ax.set_ylabel('Tiempo de ejecución (segundos)', fontsize=12, fontweight='bold')
ax.set_title('Benchmark Sistema Solar: Impacto de Optimización C++ y Entorno', fontsize=13, fontweight='bold', pad=15)
ax.set_xticks(x)
ax.set_xticklabels(etiquetas, fontsize=11, fontweight='bold')
ax.legend(fontsize=11)
ax.grid(axis='y', linestyle='--', alpha=0.7)

# Función para poner el valor numérico encima de cada barra
def autolabel(barras):
    for barra in barras:
        alto = barra.get_height()
        if alto > 0:
            ax.annotate(f'{alto:.2f} s',
                        xy=(barra.get_x() + barra.get_width() / 2, alto),
                        xytext=(0, 3),  # 3 puntos de desfase vertical
                        textcoords="offset points",
                        ha='center', va='bottom', fontsize=10, fontweight='bold')

autolabel(barras_O0)
autolabel(barras_O3)

plt.tight_layout()

# === AQUÍ ESTÁ EL CAMBIO ===
# Guardamos la gráfica en la carpeta actual en alta resolución antes de cerrar el objeto de la figura
plt.savefig('grafica_rendimiento_sistema_solar.png', dpi=300)
plt.close() # Cierra la figura para evitar que Jupyter o Python la muestren por pantalla
# ===========================

# 3. Mostrar los Speedups calculados por pantalla
print("====== FACTORES DE ACELERACIÓN (SPEEDUP) ======")
if t_local_O3 > 0:
    print(f"Ganancia por optimización en Local (-O0 -> -O3): {t_local_O0 / t_local_O3:.2f}x de velocidad.")
if t_cluster_O3 > 0:
    print(f"Ganancia por optimización en Clúster (-O0 -> -O3): {t_cluster_O0 / t_cluster_O3:.2f}x de velocidad.")