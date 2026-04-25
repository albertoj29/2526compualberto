# PROGRAMA EN PYTHON PARA REALIZAR LAS ANIMACIONES DEL MODELO DE ISING
# PARA SIMULAR EL SISTEMA A DISTINTAS TEMPERATURAS PARA ASÍ VER CUÁL ES LA TEMPERATURA CRÍTICA
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

"""
Recordar que la red es NxN entonces se representan una cuadrícula NxN de espines, donde cada espín puede
ser: espín hacia arriba (+1), que se representa de color blanco, o espín hacia abajo (-1), que se representa 
de color negro
"""

# Las 5 temperaturas que se emplearon en Ising.cpp para generar los archivos de animación
temperaturas = ["1.25", "1.75", "2.25", "2.75", "3.25"]

for T in temperaturas:
    file_in = f"ising_T_{T}.dat"
    file_out = f"animacion_T_{T}.mp4"
    
    print(f"Leyendo datos de {file_in}...")

    try:
        with open(file_in, "r") as f:
            data_str = f.read()

        # Separamos los fotogramas por el doble salto de línea
        frames_data = []
        for frame_str in data_str.strip().split("\n\n"):
            frame = []
            for line in frame_str.split("\n"):
                row = np.fromstring(line, sep=",")
                if row.size > 0:
                    frame.append(row)
            if frame:
                frames_data.append(np.array(frame))

        # Configuramos la "cámara" para ver la red
        fig, ax = plt.subplots()
        im = ax.imshow(frames_data[0], cmap='gray', vmin=-1, vmax=1)
        ax.set_title(f"Modelo de Ising - Temperatura {T}")

        def update(j):
            im.set_array(frames_data[j])
            return [im]

        ani = FuncAnimation(fig, update, frames=len(frames_data), interval=50, blit=True)

        print(f"Generando vídeo: {file_out}...")
        ani.save(file_out, writer='ffmpeg', dpi=100)
        plt.close() 

    except FileNotFoundError:
        print(f"Error: No se encontró el archivo {file_in}.")

print("\n¡Proceso finalizado!")