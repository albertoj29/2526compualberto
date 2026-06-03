import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys
import gc

# ==========================================
# 2. Generador de datos (Lectura eficiente)
# ==========================================
def generar_fotogramas(filename):
    try:
        with open(filename, 'r') as file:
            lineas_acumuladas = []
            for linea in file:
                if linea.strip():
                    lineas_acumuladas.append(linea)
                elif lineas_acumuladas:
                    fotograma = np.loadtxt(lineas_acumuladas, delimiter=',')
                    yield fotograma
                    lineas_acumuladas = []
            if lineas_acumuladas:
                yield np.loadtxt(lineas_acumuladas, delimiter=',')
    except FileNotFoundError:
        print(f"Error: No se encontró '{filename}'. Ejecuta primero tu programa C++.")
        sys.exit(1)

# Envolvemos todo en una función para procesar cada vídeo individualmente
def procesar_animacion(archivo_datos, nombre_video):
    print(f"\nProcesando {archivo_datos}...")
    
    # Averiguamos N leyendo el primer frame
    generador = generar_fotogramas(archivo_datos)
    primer_fotograma = next(generador)
    N_puntos = int(primer_fotograma[-1, 0])
    
    x_min, x_max = 0, N_puntos
    # Posición de la barrera (basada en tu C++: entre 2N/5 y 3N/5)
    barrera_inicio, barrera_fin = int(2*N_puntos/5), int(3*N_puntos/5)

    # --- NUEVO: Extraer lambda del nombre del archivo para ajustar la altura visual ---
    try:
        lambda_val = float(archivo_datos.split('_L')[-1].replace('.dat', ''))
    except Exception:
        lambda_val = 0.5 # Por si acaso un archivo no sigue el patrón

    # Asignamos una altura proporcional en el gráfico (de 0.0 a 1.0) según el valor de lambda
    if lambda_val <= 0.3:
        altura_visual = 0.2   # Barrera baja
    elif lambda_val <= 0.5:
        altura_visual = 0.35  # Barrera media-baja
    elif lambda_val <= 1.0:
        altura_visual = 0.6   # Barrera media-alta
    else:
        altura_visual = 1.0   # Muro infranqueable (λ = 10.0)

    # ==========================================
    # 3. Configuración del Gráfico (Matplotlib)
    # ==========================================
    fig, ax = plt.subplots(figsize=(10, 6))
    ax.set_xlim(x_min, x_max)

    # Ajuste automático del eje Y: 
    # Si normalizaste a 1.0, la altura es ~0.015. Si no, es ~0.1
    # LO HEMOS AJUSTADO A 0.06 PARA QUE SE VEA GRANDE AL ESTAR NORMALIZADA
    ax.set_ylim(-0.002, 0.06) 

    ax.set_xlabel("Posición (puntos de red j)")
    ax.set_ylabel("Densidad de Probabilidad $|\Phi|^2$")
    ax.set_title(f"Evolución de la Partícula Cuántica ({archivo_datos})")

    # Elementos visuales
    linea_onda, = ax.plot([], [], lw=2, color='#1f77b4', label='Paquete de ondas')
    
    # MODIFICACIÓN AQUÍ: Ahora 'ymax' usa la altura_visual calculada y muestra el valor en la leyenda
    ax.axvspan(barrera_inicio, barrera_fin, ymin=0, ymax=altura_visual, 
               color='#ff7f0e', alpha=0.3, label=f'Barrera de Potencial ($\lambda$ = {lambda_val})')
    
    texto_paso = ax.text(0.02, 0.95, '', transform=ax.transAxes, fontweight='bold')
    ax.legend(loc='upper right')

    # ==========================================
    # 4. Funciones de Animación (Corregidas)
    # ==========================================
    def init():
        linea_onda.set_data([], [])
        texto_paso.set_text('')
        return linea_onda, texto_paso

    # La función update ahora solo recibe un argumento para evitar el TypeError
    def update(datos_fotograma):
        posiciones_j = datos_fotograma[:, 0]
        probabilities = datos_fotograma[:, 1]
        
        linea_onda.set_data(posiciones_j, probabilities)
        texto_paso.set_text('Simulando evolución...')
        
        return linea_onda, texto_paso

    # ==========================================
    # 5. Ejecución y Guardado
    # ==========================================
    # Creamos la animación
    # HEMOS SUBIDO EL INTERVALO A 100 PARA QUE VAYA MÁS LENTO
    ani = animation.FuncAnimation(
        fig, 
        update, 
        frames=generar_fotogramas(archivo_datos),
        init_func=init, 
        blit=True, 
        interval=100, 
        repeat=False
    )

    print(f"El archivo se guardará como: {nombre_video}")

    # Guardar el archivo (requiere ffmpeg instalado)
    try:
        # fps=15 para que el video resultante sea más pausado y se vea mejor
        ani.save(nombre_video, writer='ffmpeg', fps=15, dpi=120)
        print("¡Video guardado con éxito!")
    except Exception as e:
        print(f"\nNo se pudo guardar el video automáticamente: {e}")
        print("Nota: Para guardar .mp4 necesitas instalar ffmpeg (sudo apt install ffmpeg)")

    plt.close(fig) # Cerramos la figura para liberar memoria antes de hacer el siguiente video
    gc.collect()

# ==========================================
# 1. Configuración de Archivos y Parámetros
# ==========================================
# Aquí definimos la lista de todos los vídeos a renderizar automáticamente
lista_experimentos = [
    ("datos_N500_L0.5.dat", "video_N500_L0.5.mp4"),
    ("datos_N2000_L0.5.dat", "video_N2000_L0.5.mp4"),
    ("datos_N500_L0.3.dat", "video_N500_L0.3.mp4"),
    ("datos_N500_L10.0.dat", "video_N500_L10.0.mp4"),
    ("datos_N1000_L1.0.dat", "video_N1000_L1.0.mp4")
]

# Ejecutar el lote de simulaciones
print("--- INICIANDO RENDERIZADO EN LOTE ---")
for archivo, video in lista_experimentos:
    procesar_animacion(archivo, video)
print("\n--- TODOS LOS VÍDEOS HAN SIDO GENERADOS ---")