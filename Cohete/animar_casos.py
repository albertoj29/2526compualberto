import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import glob

def crear_y_guardar_animacion(nombre_archivo):
    """Lee los datos de posición de un caso y genera su GIF animado."""
    # 1. Leer datos del archivo .dat de posiciones
    with open(nombre_archivo, 'r') as f:
        data = [b for b in f.read().split('\n\n') if b.strip()]

    pos_cohete = []
    pos_luna = []

    for bloque in data:
        lineas = bloque.strip().split('\n')
        if len(lineas) >= 2:
            try:
                c_x, c_y = map(float, lineas[0].split(','))
                l_x, l_y = map(float, lineas[1].split(','))
                pos_cohete.append((c_x, c_y))
                pos_luna.append((l_x, l_y))
            except: continue

    if not pos_cohete:
        print(f"Archivo {nombre_archivo} vacío o corrupto.")
        return

    # 2. Configuración del dibujo de la animación
    fig, ax = plt.subplots(figsize=(8, 8))
    ax.set_xlim(-1.2, 1.2)
    ax.set_ylim(-1.2, 1.2)
    ax.set_aspect('equal')
    ax.set_title(f"Simulación: {nombre_archivo.replace('.dat', '')}", fontsize=14, fontweight='bold')

    # Dibujamos los cuerpos celestes
    tierra = plt.Circle((0, 0), 0.05, color='blue', label='Tierra')
    ax.add_patch(tierra)
    luna_plot = plt.Circle((0, 0), 0.02, color='gray', label='Luna')
    ax.add_patch(luna_plot)
    
    # Rastro y punto del cohete
    trayectoria, = ax.plot([], [], 'r-', lw=1, alpha=0.5, label='Trayectoria Cohete')
    cohete_punto, = ax.plot([], [], 'ro', markersize=4)
    ax.legend(loc='upper right')

    def update(frame):
        lx, ly = pos_luna[frame]
        luna_plot.center = (lx, ly)
        
        cx_vals = [p[0] for p in pos_cohete[:frame]]
        cy_vals = [p[1] for p in pos_cohete[:frame]]
        trayectoria.set_data(cx_vals, cy_vals)
        
        cohete_punto.set_data([pos_cohete[frame][0]], [pos_cohete[frame][1]])
        return luna_plot, trayectoria, cohete_punto

    ani = FuncAnimation(fig, update, frames=len(pos_cohete), blit=True, interval=1)

    # 3. Guardar la animación como GIF
    nombre_salida = nombre_archivo.replace('.dat', '.gif')
    print(f"-> Generando {nombre_salida}... ({len(pos_cohete)} frames)")
    
    try:
        ani.save(nombre_salida, writer='ffmpeg', fps=40)
    except Exception as e:
        print(f"   Aviso: No se pudo usar ffmpeg ({e}). Usando motor alternativo 'pillow'...")
        ani.save(nombre_salida, writer='pillow', fps=30)
    
    plt.close() 


def graficar_estabilidad_jacobi():
    """Lee los datos de energía jacobi_*.dat y guarda el panel comparativo en un PNG."""
    archivos_jacobi = sorted(glob.glob("jacobi_*.dat"))
    
    if not archivos_jacobi:
        print("\n[Error] No se encontraron archivos 'jacobi_*.dat'. ¿Compilaste y ejecutaste el C++ modificado?")
        return

    print("\n-> Generando gráfica comparativa de la Integral de Jacobi...")
    fig, axs = plt.subplots(2, 2, figsize=(14, 10))
    fig.suptitle("Conservación de la Integral de Jacobi ($H'$)\nPrecisión Numérica del Método Runge-Kutta 4", fontsize=15, fontweight='bold')
    axs = axs.flatten()

    for i, arc in enumerate(archivos_jacobi):
        if i >= 4: break 
        
        try:
            # Estructura de jacobi_*.dat: paso, H_actual
            datos = np.loadtxt(arc)
            pasos = datos[:, 0]
            h_jacobi = datos[:, 1]
            
            # Variación absoluta respecto al inicio (debería ser casi 0)
            desviacion_h = h_jacobi - h_jacobi[0]

            axs[i].plot(pasos, desviacion_h, color='crimson', lw=1.2, label=f'Caso {i}')
            axs[i].set_title(f"Estabilidad del Sistema - Caso {i}", fontsize=12)
            axs[i].set_xlabel("Paso de Simulación")
            axs[i].set_ylabel("$H' - H'_0$")
            axs[i].grid(True, linestyle='--', alpha=0.6)
            axs[i].ticklabel_format(style='sci', axis='y', scilimits=(0,0))
            axs[i].legend(loc='upper right')
        except Exception as e:
            print(f"   Error al procesar el archivo {arc}: {e}")

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    
    # Guardamos el gráfico final de estabilidad
    plt.savefig("estabilidad_jacobi.png", dpi=300)
    print("-> ¡Gráfica de conservación guardada con éxito como 'estabilidad_jacobi.png'!")


# =========================================================================
# --- BLOQUE DE EJECUCIÓN PRINCIPAL ---
# =========================================================================
if __name__ == "__main__":
    print("=== INICIANDO PROCESAMIENTO DE DATOS ===")
    
    # Fase 1: Encontrar y renderizar las animaciones GIF
    archivos_cohete = sorted(glob.glob("cohete_*.dat"))
    if archivos_cohete:
        print(f"Se han detectado {len(archivos_cohete)} archivos de trayectorias.")
        for arc in archivos_cohete:
            crear_y_guardar_animacion(arc)
    else:
        print("[Aviso] No se encontraron archivos 'cohete_*.dat'.")

    # Fase 2: Encontrar los datos de energía y hacer el plot de Jacobi
    graficar_estabilidad_jacobi()
    
    print("\n=== PROCESO COMPLETADO EN SU TOTALIDAD ===")
    print("Ya tienes listos en tu carpeta los 4 GIFs y el archivo 'estabilidad_jacobi.png'.")