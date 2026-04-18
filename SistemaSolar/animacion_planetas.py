# ================================================================================
# ANIMACION SISTEMA SOLAR (MODIFICADO FINAL CORREGIDO)
# ================================================================================

from matplotlib import pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.patches import Circle
import numpy as np

# Parámetros
file_in = "data/evolucion.dat"
interval = 50
show_trail = True
trail_width = 1.5
save_to_file = True

planet_names = [
    "Sol", "Mercurio", "Venus", "Tierra", "Marte",
    "Júpiter", "Saturno", "Urano", "Neptuno", "Plutón"
]

# Leer datos
with open(file_in, "r") as f:
    data_str = f.read()

frames_data = []
for frame_data_str in data_str.split("\n\n"):
    frame_data = []
    for planet_pos_str in frame_data_str.split("\n"):
        planet_pos = np.fromstring(planet_pos_str, sep=",")
        if planet_pos.size > 0:
            frame_data.append(planet_pos)
    if len(frame_data) > 0:
        frames_data.append(frame_data)

nplanets = len(frames_data[0])

colors = plt.cm.tab10(np.linspace(0, 1, nplanets))

# ============================
# FUNCIÓN GENERAL
# ============================
def crear_animacion(indices_planetas, titulo, xlim, ylim, nombre_archivo):

    fig, ax = plt.subplots(figsize=(8, 8))

    ax.axis("equal")
    ax.set_xlim(xlim)
    ax.set_ylim(ylim)
    ax.set_xlabel("X (UA)")
    ax.set_ylabel("Y (UA)")
    ax.set_title(titulo)
    ax.grid(True, alpha=0.3)

    planet_points = []
    planet_trails = []

    # Inicializar planetas
    for i in indices_planetas:
        x, y = frames_data[0][i]

        planet = Circle((x, y), 0.15, color=colors[i], ec='black')
        ax.add_artist(planet)
        planet_points.append(planet)

        if show_trail:
            trail, = ax.plot(x, y, "-", linewidth=trail_width,
                             color=colors[i], alpha=0.7)
            planet_trails.append(trail)

    # LEYENDA
    handles = [
        plt.Line2D([0], [0], marker='o', color='w',
                   markerfacecolor=colors[i],
                   label=planet_names[i],
                   markersize=8)
        for i in indices_planetas
    ]
    ax.legend(handles=handles, loc="upper right")

    #  Reducimos frames (MUY IMPORTANTE)
    step = 5
    frames_reducidos = list(range(0, len(frames_data), step))

    # UPDATE
    def update(frame_idx):
        frame = frames_reducidos[frame_idx]

        for j, i in enumerate(indices_planetas):
            x, y = frames_data[frame][i]
            planet_points[j].center = (x, y)

            if show_trail:
                xs_old, ys_old = planet_trails[j].get_data()
                xs_new = np.append(xs_old, x)
                ys_new = np.append(ys_old, y)
                planet_trails[j].set_data(xs_new, ys_new)

        return planet_points + planet_trails

    def init():
        if show_trail:
            for trail in planet_trails:
                trail.set_data([], [])
        return planet_points + planet_trails

    anim = FuncAnimation(
        fig, update, init_func=init,
        frames=len(frames_reducidos),
        interval=interval, blit=True
    )

    if save_to_file:
        print(f"Guardando '{nombre_archivo}'...")
        anim.save(nombre_archivo, dpi=150)
        print("¡Hecho!")
    else:
        plt.show()

    plt.close(fig)  # 🔥 CLAVE para que la segunda animación funcione


# ============================
# ANIMACIONES
# ============================

#  INTERIOR
crear_animacion(
    indices_planetas=[0,1,2,3,4],
    titulo="Sistema Solar Interior",
    xlim=(-2, 2),
    ylim=(-2, 2),
    nombre_archivo="interior.mp4"
)

#  EXTERIOR
crear_animacion(
    indices_planetas=[0,5,6,7,8,9],
    titulo="Sistema Solar Exterior",
    xlim=(-50, 50),
    ylim=(-50, 50),
    nombre_archivo="exterior.mp4"
)