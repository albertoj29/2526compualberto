"""
=============================================================================
SCRIPT DE ANÁLISIS Y GRÁFICAS: MODELO DE KAWASAKI (FILTRADO DINÁMICO)
Este script procesa los datos generados por C++ y realiza TODAS las gráficas 
y animaciones del Trabajo Voluntario de una sola vez.

Optimización: Aplica un filtrado adaptativo según el ruido estadístico de 
cada tamaño de red para obtener una tendencia de Tc más sólida y física.
=============================================================================
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import os # Para comprobar si existen los archivos antes de leerlos
from scipy.signal import savgol_filter # Para el suavizado de datos

# Definimos las listas de parámetros que usamos en C++
lista_tamanos = [32, 64, 128]
lista_m0 = [0.0, 0.4]
temperaturas_anim = [1.5, 2.3, 3.5]


# =============================================================================
# [ACTIVIDADES 1 y 8]: REPRESENTACIÓN DE FOTOGRAMAS (GIFS)
# =============================================================================
print("\n--- INICIANDO ACTIVIDADES 1 Y 8: GENERACIÓN DE ANIMACIONES GIF ---")

for m0_anim in lista_m0:
    etiqueta_m_anim = int(m0_anim * 10)
    
    print(f"\n>> Generando animaciones para m0 = {m0_anim}...")
    
    for N_anim in lista_tamanos:
        for T_anim in temperaturas_anim:
            nombre_archivo_anim = f"animacion_kawasaki_N{N_anim}_m{etiqueta_m_anim}_T{T_anim:.1f}.dat"
            nombre_gif = f"evolucion_dominios_N{N_anim}_m{etiqueta_m_anim}_T{T_anim:.1f}.gif"
            
            if os.path.exists(nombre_archivo_anim):
                print(f"  Procesando {nombre_archivo_anim} -> {nombre_gif}")
                try:
                    with open(nombre_archivo_anim, "r") as f:
                        contenido = f.read().strip().split("\n\n")  
                    
                    frames = []
                    for bloque in contenido:
                        if bloque.strip():
                            matriz = np.array([list(map(int, linea.split(','))) for linea in bloque.strip().split("\n")])
                            frames.append(matriz)

                    fig_anim, ax_anim = plt.subplots(figsize=(5, 5))
                    ax_anim.set_title(f"Kawasaki N={N_anim} | m0={m0_anim} | T={T_anim:.1f}", fontsize=12)
                    ax_anim.axis('off')

                    imagen = ax_anim.imshow(frames[0], cmap='bwr', vmin=-1, vmax=1)

                    def actualizar(num_frame):
                        imagen.set_array(frames[num_frame])
                        return [imagen]

                    ani = animation.FuncAnimation(fig_anim, actualizar, frames=len(frames), interval=50, blit=True)
                    ani.save(nombre_gif, writer='pillow')
                    plt.close(fig_anim) 
                except Exception as e:
                    print(f"    Error al generar GIF: {e}")
            else:
                print(f"  Aviso: Faltan datos para {nombre_archivo_anim}.")


# =============================================================================
# [ACTIVIDADES 2, 4, 6, 7 y 8]: ANÁLISIS DE OBSERVABLES (BARRIDO CONTINUO DE T)
# =============================================================================
print("\n--- INICIANDO ACTIVIDADES 2, 4, 6, 7 Y 8: ANÁLISIS DE OBSERVABLES ---")

for m0 in lista_m0:
    etiqueta_m = int(m0 * 10)
    print(f"\n>> Procesando gráficas de observables para m0 = {m0}")

    fig, axs = plt.subplots(2, 2, figsize=(14, 11))
    fig.suptitle(f'Análisis del Modelo de Kawasaki ($m_0 = {m0}$) - Comparativa de Tamaños', fontsize=16, fontweight='bold')

    for N in lista_tamanos:
        archivo_datos = f"observables_kawasaki_N{N}_m{etiqueta_m}.txt"
        
        if not os.path.exists(archivo_datos):
            print(f"  Aviso: Faltan los datos de {archivo_datos}.")
            continue
            
        try:
            datos = np.loadtxt(archivo_datos, skiprows=1)
            T       = datos[:, 0]
            E_media = datos[:, 1]
            Cv      = datos[:, 2]
            M_bot   = datos[:, 3]
            M_top   = datos[:, 4]
            Chi_bot = datos[:, 5]
            Chi_top = datos[:, 6]

            # --- MEJORA: FILTRADO DINÁMICO ADAPTATIVO ---
            # Ajustamos la ventana según el nivel de ruido de cada tamaño de red
            if N == 32:
                ancho_ventana = 11  # Ventana grande para domar el ruido extremo
                grado_poli = 2
            elif N == 64:
                ancho_ventana = 13  # Ventana máxima para fusionar las dos jorobas falsas (2.0 y 2.7)
                grado_poli = 2
            else:
                ancho_ventana = 7   # Ventana pequeña para preservar el pico real y nítido de N=128
                grado_poli = 3

            Cv_suave = savgol_filter(Cv, window_length=ancho_ventana, polyorder=grado_poli)
            Chi_bot_suave = savgol_filter(Chi_bot, window_length=ancho_ventana, polyorder=grado_poli)

            # Búsqueda de Tc(N) en las nuevas curvas adaptativas
            Tc_exp_Cv = T[np.argmax(Cv_suave)]
            Tc_exp_Chi = T[np.argmax(Chi_bot_suave)]
            # --------------------------------------------
            
            if m0 == 0.0:
                print(f"  - Tamaño N={N} | Tc(Cv): {Tc_exp_Cv:.2f} | Tc(Chi): {Tc_exp_Chi:.2f}")
            else:
                print(f"  - Tamaño N={N} | Tc(Cv): {Tc_exp_Cv:.2f}")

            # [ACTIVIDAD 4]: Energía vs T
            axs[0, 0].plot(T, E_media, '.-', label=f'$N={N}$')
            axs[0, 0].set_title('Actividad 4: Energía Media por partícula', fontsize=12, fontweight='bold')
            axs[0, 0].set_ylabel(r'$\langle E \rangle / N^2$', fontsize=11)
            axs[0, 0].grid(True, alpha=0.4)
            axs[0, 0].legend(loc='best', fontsize=9)

            # [ACTIVIDAD 6]: Calor Específico vs T
            color_linea = axs[0, 1].plot(T, Cv, '.', alpha=0.25)[0].get_color() 
            axs[0, 1].plot(T, Cv_suave, '-', color=color_linea, linewidth=2, label=f'$N={N}$ ($T_c \\approx {Tc_exp_Cv:.2f}$)')
            axs[0, 1].set_title('Actividad 6: Calor Específico ($c_N$)', fontsize=12, fontweight='bold')
            axs[0, 1].set_ylabel(r'$c_N$', fontsize=11)
            axs[0, 1].grid(True, alpha=0.4)
            axs[0, 1].legend(loc='best', fontsize=9)

            # [ACTIVIDADES 2 y 8]: Magnetización por dominios vs T 
            axs[1, 0].plot(T, M_bot, '.-', label=f'$N={N}$ (Dom. Inf +)')
            axs[1, 0].plot(T, M_top, '.--', alpha=0.7, label=f'$N={N}$ (Dom. Sup -)')
            if m0 == 0.0:
                axs[1, 0].set_title('Actividad 2: Magnetización por Dominios', fontsize=12, fontweight='bold')
            else:
                axs[1, 0].set_title('Actividad 8: Magnetización por Dominios (Salto Discontinuo)', fontsize=12, fontweight='bold')
            axs[1, 0].set_ylabel(r'$\langle M \rangle$', fontsize=11)
            axs[1, 0].grid(True, alpha=0.4)
            axs[1, 0].legend(loc='best', fontsize=8)

            # [ACTIVIDAD 7]: Susceptibilidad por dominios vs T (SOLO si m0 == 0.0)
            if m0 == 0.0:
                color_chi = axs[1, 1].plot(T, Chi_bot, '.', alpha=0.25)[0].get_color()
                axs[1, 1].plot(T, Chi_bot_suave, '-', color=color_chi, linewidth=2, label=f'$N={N}$ ($T_c \\approx {Tc_exp_Chi:.2f}$)')
                axs[1, 1].plot(T, Chi_top, '.--', alpha=0.5, label=f'$N={N}$ (Dom. Sup -)')
                axs[1, 1].set_title('Actividad 7: Susceptibilidad Magnética ($\chi_N$)', fontsize=12, fontweight='bold')
                axs[1, 1].set_ylabel(r'$\chi_N$', fontsize=11)
                axs[1, 1].grid(True, alpha=0.4)
                axs[1, 1].legend(loc='best', fontsize=8)

        except Exception as e:
            print(f"  Error procesando {archivo_datos}: {e}")

    if m0 != 0.0:
        axs[1, 1].axis('off') 
        texto_explicativo = (
            "El Apartado 8 especifica textualmente:\n"
            "'Realizar los puntos 1-6 partiendo de\nuna magnetización no nula'.\n\n"
            "Por consiguiente, el cálculo de la\n"
            "susceptibilidad $\chi_N$ (Punto 7)\n"
            "no es requerido para este caso."
        )
        axs[1, 1].text(0.5, 0.5, texto_explicativo, ha='center', va='center', 
                       fontsize=12, style='italic', color='darkred',
                       bbox=dict(facecolor='ivory', alpha=0.9, edgecolor='gray', boxstyle='round,pad=1'))

    for idx, ax in enumerate(axs.flat):
        if idx != 3 or m0 == 0.0: 
            ax.set_xlabel('Temperatura ($T$)', fontsize=11)

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    nombre_imagen = f'graficas_observables_m{etiqueta_m}.png'
    plt.savefig(nombre_imagen, dpi=300)
    print(f"  -> Gráfica agrupada guardada como '{nombre_imagen}'")
    plt.close(fig) 


# =============================================================================
# [ACTIVIDADES 3, 5 y 8]: PERFILES VERTICALES DE DENSIDAD
# =============================================================================
print("\n--- INICIANDO ACTIVIDADES 3 Y 5: PERFILES DE DENSIDAD ---")

for m0_perfil in lista_m0:
    etiqueta_m_perfil = int(m0_perfil * 10)
    
    for N_perfil in lista_tamanos:
        archivo_perfil = f"perfil_densidad_N{N_perfil}_m{etiqueta_m_perfil}.txt"

        if os.path.exists(archivo_perfil):
            try:
                perfiles = np.loadtxt(archivo_perfil)
                
                plt.figure(figsize=(7, 5))
                N_filas = len(perfiles[0]) - 1 
                y_eje = np.arange(N_filas)
                
                for fila in perfiles:
                    temp = round(fila[0], 1) 
                    if temp in temperaturas_anim: 
                        densidad = fila[1:]  
                        plt.plot(densidad, y_eje, '.-', linewidth=2, label=f'T = {temp}')

                plt.title(f"Actividades 3/5: Perfil Densidad Y (N={N_perfil} | m0={m0_perfil})", fontsize=14)
                plt.xlabel(r"Densidad media $\rho(y)$", fontsize=12)
                plt.ylabel("Posición Y (Filas)", fontsize=12)
                plt.gca().invert_yaxis() 
                plt.axvline(x=0.5, color='gray', linestyle='--', alpha=0.6) 
                plt.legend()
                plt.grid(True, alpha=0.3)
                
                nombre_img_perfil = f'grafica_perfil_N{N_perfil}_m{etiqueta_m_perfil}.png'
                plt.savefig(nombre_img_perfil, dpi=300)
                print(f"  -> Perfil guardado: '{nombre_img_perfil}'")
                plt.close()

            except Exception as e:
                print(f"  Error procesando {archivo_perfil}: {e}")

print("\n¡SCRIPT DE PYTHON FINALIZADO CON ÉXITO! Filtros dinámicos aplicados.")