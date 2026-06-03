import matplotlib.pyplot as plt
import os

def cargar_datos_bench(nombre_archivo):
    N_lista = []
    tiempo_lista = []
    if not os.path.exists(nombre_archivo):
        print(f"Aviso: No se encuentra '{nombre_archivo}'.")
        return None, None
    
    with open(nombre_archivo, 'r') as f:
        for linea in f:
            if linea.strip():
                partes = linea.split()
                N_lista.append(int(partes[0]))
                tiempo_lista.append(float(partes[1]))
    return N_lista, tiempo_lista

# 1. Cargamos los 4 ficheros de datos
N_PC_0, T_PC_0 = cargar_datos_bench("tiempos_lento.txt")
N_PC_3, T_PC_3 = cargar_datos_bench("tiempos_rapido.txt")
N_JL_0, T_JL_0 = cargar_datos_bench("tiempos_lento_joel.txt")
N_JL_3, T_JL_3 = cargar_datos_bench("tiempos_rapido_joel.txt")

# Comprobación de seguridad
if None in (N_PC_0, N_PC_3, N_JL_0, N_JL_3):
    print("Error: Faltan archivos .txt. Asegúrate de tener los 4 archivos en la misma carpeta.")
    exit()

# 2. Configuración de la gráfica
plt.figure(figsize=(11, 7))

# 3. Dibujamos las 4 curvas con colores bien diferenciados
plt.plot(N_PC_0, T_PC_0, color='red', marker='o', linestyle='-', linewidth=2, markersize=8, label='PC Local: Sin optimizar (-O0)')
plt.plot(N_JL_0, T_JL_0, color='purple', marker='s', linestyle='--', linewidth=2, markersize=8, label='JOEL: Sin optimizar (-O0)')

plt.plot(N_PC_3, T_PC_3, color='green', marker='o', linestyle='-', linewidth=2, markersize=8, label='PC Local: Optimizado (-O3)')
plt.plot(N_JL_3, T_JL_3, color='blue', marker='s', linestyle='--', linewidth=2, markersize=8, label='JOEL: Optimizado (-O3)')

# 4. Textos y etiquetas profesionales
plt.title('Estudio de Rendimiento Global: Hardware vs Optimización del Compilador', fontsize=14, fontweight='bold')
plt.xlabel('Tamaño del sistema (Matriz N x N)', fontsize=12)
plt.ylabel('Tiempo de cómputo puro (segundos)', fontsize=12)

# Ajustes de los ejes
plt.xticks(N_PC_0)
plt.grid(True, linestyle=':', alpha=0.7)
plt.legend(fontsize=11, loc='upper left')

# 5. Anotaciones de los tiempos exactos para N=128
# Usamos desplazamientos distintos (ha, va) para que los textos no se pisen entre ellos
plt.text(N_PC_0[-1], T_PC_0[-1], f"  {T_PC_0[-1]:.2f} s", va='bottom', ha='left', color='red', fontweight='bold')
plt.text(N_JL_0[-1], T_JL_0[-1], f"  {T_JL_0[-1]:.2f} s", va='top', ha='left', color='purple', fontweight='bold')
plt.text(N_PC_3[-1], T_PC_3[-1], f"  {T_PC_3[-1]:.2f} s", va='bottom', ha='left', color='green', fontweight='bold')
plt.text(N_JL_3[-1], T_JL_3[-1], f"  {T_JL_3[-1]:.2f} s", va='top', ha='left', color='blue', fontweight='bold')

# Guardar la imagen
nombre_grafica = 'rendimiento_total_4bandas.png'
plt.tight_layout()
plt.savefig(nombre_grafica, dpi=300)
print(f"¡Gráfica generada y guardada como '{nombre_grafica}'!")

plt.show()