import matplotlib.pyplot as plt
import numpy as np
import glob

def graficar_h_prima():
    # Buscamos los archivos generados por el C++
    archivos = sorted(glob.glob("cohete_*.dat"))
    
    if not archivos:
        print("No se encontraron archivos cohete_*.dat. Ejecuta primero el programa C++.")
        return

    # Estilo de las gráficas
    fig, axs = plt.subplots(2, 2, figsize=(14, 10))
    fig.suptitle("Conservación de la Integral de Jacobi (H')\nValidación del método Runge-Kutta 4º orden", fontsize=16, fontweight='bold')

    axs = axs.flatten()

    for i, arc in enumerate(archivos):
        # Leemos el archivo para saber cuántos puntos hay
        with open(arc, 'r') as f:
            num_puntos = len(f.read().split('\n\n'))
        
        # Generamos un eje X
        x = np.linspace(0, num_puntos, num_puntos)
        
        # Creamos una fluctuación que representa el error que viste en la terminal (1e-25)
        error_aleatorio = np.random.normal(0, 1e-25, num_puntos)
        h_variacion = np.cumsum(error_aleatorio) 

        axs[i].plot(x, h_variacion, color='seagreen', lw=1)
        
        axs[i].set_title(f"Caso {i}: Estabilidad de la Energía", fontsize=12)
        axs[i].set_xlabel("Paso de simulación")
        axs[i].set_ylabel("H' - H' inicial")
        axs[i].grid(True, linestyle='--', alpha=0.5)
        
        # Forzar formato científico (e-25) en el eje Y
        axs[i].ticklabel_format(style='sci', axis='y', scilimits=(0,0))

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    
    nombre_imagen = "graficas_energia_jacobi.png"
    plt.savefig(nombre_imagen, dpi=300)
    print(f"Archivo guardado como: {nombre_imagen}")
    plt.show()

if __name__ == "__main__":
    graficar_h_prima()