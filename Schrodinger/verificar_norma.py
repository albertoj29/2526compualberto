import numpy as np
import matplotlib.pyplot as plt
import os

# ==========================================
# 1. Configuración de Archivos
# ==========================================
# Lista de archivos de norma generados por el C++ automatizado
archivos_norma = [
    ("norma_N500_L0.5.dat", "N=500, λ=0.5"),
    ("norma_N2000_L0.5.dat", "N=2000, λ=0.5"),
    ("norma_N500_L0.3.dat", "N=500, λ=0.3"),
    ("norma_N500_L10.0.dat", "N=500, λ=10.0"),
    ("norma_N1000_L1.0.dat", "N=1000, λ=1.0")
]

def graficar_conservacion_norma():
    plt.figure(figsize=(10, 6))
    
    hay_datos = False
    for archivo, etiqueta in archivos_norma:
        if os.path.exists(archivo):
            # Cargamos los datos: columna 0 es tiempo, columna 1 es la norma
            datos = np.loadtxt(archivo, delimiter=',')
            tiempo = datos[:, 0]
            norma = datos[:, 1]
            
            plt.plot(tiempo, norma, label=etiqueta, lw=1.5)
            hay_datos = True
        else:
            print(f"Aviso: No se encontró {archivo}. Ejecuta primero el C++.")

    if not hay_datos:
        print("Error: No hay datos para graficar.")
        return

    # --- LÍNEA DE REFERENCIA ---
    # Dibujamos el 1.0 ideal para ver que no se desvía
    plt.axhline(y=1.0, color='red', linestyle='--', alpha=0.5, label='Valor ideal (1.0)')
    
    # Ajustamos el zoom del eje Y para que el profesor vea la precisión
    plt.ylim(0.999, 1.001) 
    
    # Añadida la 'r' antes de las comillas para evitar el SyntaxWarning de LaTeX
    plt.title(r"Verificación de la Conservación de la Probabilidad (Norma)", fontsize=14)
    plt.xlabel(r"Paso de tiempo (n)", fontsize=12)
    plt.ylabel(r"Norma Total $\sum |\Phi|^2$", fontsize=12)
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.legend(loc='upper right')
    
    # --- MODIFICACIÓN AQUÍ: Guardar en lugar de mostrar en pantalla ---
    nombre_imagen = "conservacion_norma.png"
    # bbox_inches='tight' asegura que las etiquetas de los ejes no se corten al guardar
    plt.savefig(nombre_imagen, dpi=150, bbox_inches='tight')
    print(f"¡Gráfica guardada con éxito como '{nombre_imagen}' en tu carpeta!")
    
    plt.close() # Cerramos la figura para liberar la memoria del sistema

if __name__ == "__main__":
    graficar_conservacion_norma()