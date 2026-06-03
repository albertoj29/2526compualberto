# ==============================================================================
# PIPELINE DE ANÁLISIS Y GRAFICACIÓN - DINÁMICA MOLECULAR (LENNARD-JONES)
# Diseñado para procesar múltiples simulaciones en lotes de forma genérica
# Modificado para exportar animaciones en formato GIF de alta calidad para Jupyter
# ==============================================================================

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import os
from scipy.stats import linregress # Requerido para el ajuste de la ecuación de estado

# De forma análoga a como lo hicimos en el script de C++, ahora primero vamos a definir algunas
# constantes y funciones auxiliares para luego poder hacer un análisis completo de los distintos
# apartados, generando gráficas y simulaciones

# ==============================================================================
# CONSTANTES FÍSICAS DEL SISTEMA (Deben coincidir con las de C++)
# ==============================================================================
m = 1.0       # Masa reducida de las partículas
kB = 1.0      # Constante de Boltzmann en unidades reducidas
N = 100       # Número total de átomos
L = 22.0      # Longitud de la caja cuadrada

# ==============================================================================
# 1. FUNCIÓN DE ANÁLISIS PARA CASOS ESTÁNDAR (Isótropos con módulo fijo)
# Esta función procesa de forma automatizada los archivos .dat de una simulación estándar, 
# generando gráficas de evolución temporal de energía e histogramas de equilibrio contrastados 
# con las curvas teóricas de Maxwell-Boltzmann y Gauss.
# ==============================================================================
def analizar_caso_estandar(prefijo, titulo_graficas):
    """
    Procesa de forma automatizada los archivos .dat de una simulación estándar.
    Genera gráficas de evolución temporal de energía e histogramas de equilibrio
    contrastados con las curvas teóricas de Maxwell-Boltzmann y Gauss.
    """
    print(f"\n[INFO] Analizando: {titulo_graficas} (Prefijo: '{prefijo}')")
    
    # Verificación de existencia de archivos antes de operar
    if not os.path.exists(f'{prefijo}energias.dat') or not os.path.exists(f'{prefijo}velocidades.dat'):
        print(f"[ERROR] No se encontraron los archivos para el prefijo '{prefijo}'. Saltando caso.")
        return None

    # --------------------------------------------------------------------------
    # 1.1 Procesamiento de Energías
    # --------------------------------------------------------------------------
    datos_energia = np.loadtxt(f'{prefijo}energias.dat')
    t_e = datos_energia[:, 0]
    E_cin = datos_energia[:, 1]
    E_pot = datos_energia[:, 2]
    E_tot = datos_energia[:, 3]
    
    # --------------------------------------------------------------------------
    # 1.2 Análisis Estadístico de Velocidades
    # --------------------------------------------------------------------------
    datos_vel = np.loadtxt(f'{prefijo}velocidades.dat')
    t_v = datos_vel[:, 0]
    vx = datos_vel[:, 1]
    vy = datos_vel[:, 2]
    v_mod = datos_vel[:, 3]
    
    # Extraemos específicamente los datos para t=0 (Para el histograma inicial)
    filtro_t0 = (t_v == 0.0)
    v_mod_t0 = v_mod[filtro_t0]

    # Filtro temporal estricto para asegurar que el gas ya se ha termalizado
    filtro_eq = (t_v >= 100.0) & (t_v <= 250.0)
    vx_eq = vx[filtro_eq]
    vy_eq = vy[filtro_eq]
    v_mod_eq = v_mod[filtro_eq]
    
    # TEOREMA DE EQUIPARTICIÓN EN 2D: <E_cin> = 2 * (1/2 * kB * T) -> T = <v^2> * m / (2 * kB)
    T_calculada = 0.5 * np.mean(vx_eq**2 + vy_eq**2) * m / kB
    print(f"   -> Temperatura termodinámica de equilibrio obtenida: T = {T_calculada:.4f}")

    # ==========================================================================
    # CREACIÓN DE LA FIGURA COMBINADA (Múltiples subplots)
    # ==========================================================================
    fig = plt.figure(figsize=(12, 12))
    fig.suptitle(f'Resultados de la Simulación: {titulo_graficas}\nTemperatura de equilibrio: T = {T_calculada:.4f}', fontsize=14, fontweight='bold')

    # --------------------------------------------------------------------------
    # 1.3 Histograma del Módulo de la Velocidad a t=0 (Pico Delta de Dirac)
    # --------------------------------------------------------------------------
    ax1 = plt.subplot(3, 2, 1)
    ax1.hist(v_mod_t0, bins=20, color='#9467bd', edgecolor='black', density=True, alpha=0.8)
    ax1.set_title('Distribución de Módulos |v| a t=0', fontsize=11, fontweight='bold')
    ax1.set_xlabel('Módulo de la Velocidad |v|', fontsize=10)
    ax1.set_ylabel('Frecuencia', fontsize=10)
    ax1.grid(True, linestyle=':', alpha=0.5)

    # --------------------------------------------------------------------------
    # 1.4 Histograma del Módulo de la Velocidad en Equilibrio (Maxwell)
    # --------------------------------------------------------------------------
    ax2 = plt.subplot(3, 2, 2)
    ax2.hist(v_mod_eq, bins=45, color='#a1c9f4', edgecolor='black', density=True, alpha=0.8, label='Simulación (MD)')
    
    # Curva teórica analítica de Maxwell-Boltzmann para dos dimensiones
    v_eje = np.linspace(0, np.max(v_mod_eq) * 1.15, 250)
    f_maxwell = (m * v_eje / (kB * T_calculada)) * np.exp(-m * v_eje**2 / (2 * kB * T_calculada))
    
    ax2.plot(v_eje, f_maxwell, color='#d62728', linewidth=2.5, label=f'Teoría MB (T={T_calculada:.2f})')
    ax2.set_title('Distribución de Módulos |v| en Equilibrio', fontsize=11, fontweight='bold')
    ax2.set_xlabel('Módulo de la Velocidad |v|', fontsize=10)
    ax2.set_ylabel('Densidad de Probabilidad', fontsize=10)
    ax2.legend(shadow=True)
    ax2.grid(True, linestyle=':', alpha=0.5)

    # --------------------------------------------------------------------------
    # 1.5 Histograma Estadístico de la Componente Vx (Perfil Gaussiano)
    # --------------------------------------------------------------------------
    ax3 = plt.subplot(3, 2, 3)
    ax3.hist(vx_eq, bins=45, color='#8de5a1', edgecolor='black', density=True, alpha=0.8, label='Simulación vx')
    
    # Curva Gaussiana teórica centrada en cero
    vx_eje = np.linspace(np.min(vx_eq) * 1.2, np.max(vx_eq) * 1.2, 250)
    f_gaussX = np.sqrt(m / (2 * np.pi * kB * T_calculada)) * np.exp(-m * vx_eje**2 / (2 * kB * T_calculada))
    
    ax3.plot(vx_eje, f_gaussX, color='#2ca02c', linewidth=2.5, label=f'Gauss teórica')
    ax3.set_title('Componente Intrínseca vx en Equilibrio', fontsize=11, fontweight='bold')
    ax3.set_xlabel('Velocidad vx', fontsize=10)
    ax3.set_ylabel('Densidad de Probabilidad', fontsize=10)
    ax3.legend(shadow=True)
    ax3.grid(True, linestyle=':', alpha=0.5)

    # --------------------------------------------------------------------------
    # 1.6 Histograma Estadístico de la Componente Vy (Perfil Gaussiano)
    # --------------------------------------------------------------------------
    ax4 = plt.subplot(3, 2, 4)
    ax4.hist(vy_eq, bins=45, color='#ffb3b3', edgecolor='black', density=True, alpha=0.8, label='Simulación vy')
    
    # Curva Gaussiana teórica centrada en cero
    vy_eje = np.linspace(np.min(vy_eq) * 1.2, np.max(vy_eq) * 1.2, 250)
    f_gaussY = np.sqrt(m / (2 * np.pi * kB * T_calculada)) * np.exp(-m * vy_eje**2 / (2 * kB * T_calculada))
    
    ax4.plot(vy_eje, f_gaussY, color='#cc0000', linewidth=2.5, label=f'Gauss teórica')
    ax4.set_title('Componente Intrínseca vy en Equilibrio', fontsize=11, fontweight='bold')
    ax4.set_xlabel('Velocidad vy', fontsize=10)
    ax4.set_ylabel('Densidad de Probabilidad', fontsize=10)
    ax4.legend(shadow=True)
    ax4.grid(True, linestyle=':', alpha=0.5)

    # --------------------------------------------------------------------------
    # 1.1 (Continuación) Graficación de Energías (Ocupa el ancho completo)
    # --------------------------------------------------------------------------
    ax5 = plt.subplot(3, 2, (5, 6)) # Usamos los subplots 5 y 6 combinados
    ax5.plot(t_e, E_cin, label='E. Cinética', color='#1f77b4', alpha=0.8)
    ax5.plot(t_e, E_pot, label='E. Potencial', color='#ff7f0e', alpha=0.8)
    ax5.plot(t_e, E_tot, label='E. Total (Verlet)', color='#2ca02c', linewidth=2)
    ax5.set_title('Conservación y Relajación de la Energía', fontsize=12, fontweight='bold')
    ax5.set_xlabel('Tiempo Termodinámico (t)', fontsize=10)
    ax5.set_ylabel('Energía del Sistema', fontsize=10)
    ax5.legend(loc='best', frameon=True, shadow=True)
    ax5.grid(True, linestyle='--', alpha=0.6)

    # Guardado de la imagen unificada
    plt.tight_layout(rect=[0, 0.03, 1, 0.95]) # Ajuste para que el título global no se corte
    nombre_archivo = f'{prefijo}resultados_combinados.png'
    plt.savefig(nombre_archivo, dpi=300)
    plt.close()
    
    return T_calculada

# ==============================================================================
# 2. FUNCIÓN DE ANÁLISIS PARA EL CASO ESPECIAL (Efecto del Viento de Arrastre)
# Esta función analiza el comportamiento colectivo del gas cuando se inyecta un viento direccional en el eje X.
# ==============================================================================
def analizar_caso_viento(prefijo):
    """
    Analiza el comportamiento colectivo del gas cuando se inyecta un viento 
    direccional en el eje X. Aplica estadística de varianzas para aislar el movimiento 
    macroscópico colectivo (Centro de Masas) del verdadero perfil térmico interno.
    """
    print(f"\n[INFO] Analizando: Caso Especial Viento de Arrastre (Prefijo: '{prefijo}')")
    
    if not os.path.exists(f'{prefijo}energias.dat') or not os.path.exists(f'{prefijo}velocidades.dat'):
        print(f"[ERROR] No se encontraron archivos para el caso viento. Saltando.")
        return

    # 2.1 Tratamiento físico de las velocidades retenidas
    datos_vel = np.loadtxt(f'{prefijo}velocidades.dat')
    t_v, vx, vy = datos_vel[:, 0], datos_vel[:, 1], datos_vel[:, 2]
    
    filtro_eq = (t_v >= 100.0) & (t_v <= 250.0)
    vx_eq = vx[filtro_eq]
    vy_eq = vy[filtro_eq]
    
    # CÁLCULO ESENCIAL: Extraemos la velocidad macroscópica de arrastre (Drift Velocity)
    v_viento_neto = np.mean(vx_eq)
    v_drift_y = np.mean(vy_eq) # Debería fluctuar en torno a 0 de forma estadística
    
    # Comentario: La temperatura real del gas NO depende de la velocidad absoluta total,
    # sino exclusivamente de las fluctuaciones moleculares respecto al centro de masas.
    # Por tanto, usamos las VARIANZAS de las componentes de velocidad en lugar de los cuadrados.
    T_viento = 0.5 * (np.var(vx_eq) + np.var(vy_eq)) * m / kB
    print(f"   -> Velocidad macroscópica medida del viento <vx>: {v_viento_neto:.4f}")
    print(f"   -> Temperatura real (Fluctuaciones térmicas puras): T = {T_viento:.4f}")

    # ==========================================================================
    # CREACIÓN DE LA FIGURA COMBINADA (Caso Viento)
    # ==========================================================================
    fig = plt.figure(figsize=(12, 8))
    fig.suptitle(f'Resultados Viento de Arrastre\nTemp. real: T = {T_viento:.4f} | Viento Neto = {v_viento_neto:.2f}', fontsize=14, fontweight='bold')

    # 2.3 Histograma Vx - Gaussiana desplazada debido a la velocidad del viento
    ax1 = plt.subplot(2, 2, 1)
    ax1.hist(vx_eq, bins=45, color='#ffeb9c', edgecolor='black', density=True, alpha=0.8, label='Simulación vx')
    vx_eje = np.linspace(np.min(vx_eq) * 1.2, np.max(vx_eq) * 1.2, 250)
    f_gauss_desplazada = np.sqrt(m / (2 * np.pi * kB * T_viento)) * np.exp(-m * (vx_eje - v_viento_neto)**2 / (2 * kB * T_viento))
    ax1.plot(vx_eje, f_gauss_desplazada, color='#e65c00', linewidth=2.5, label=f'Gauss Centrada en {v_viento_neto:.2f}')
    ax1.set_title('Distribución de vx Desplazada por Efecto del Viento', fontsize=11, fontweight='bold')
    ax1.set_xlabel('Velocidad vx', fontsize=10)
    ax1.set_ylabel('Densidad de Probabilidad', fontsize=10)
    ax1.legend(shadow=True)
    ax1.grid(True, linestyle=':', alpha=0.5)

    # 2.4 Histograma Vy - Perfil Gaussiano normal surgido por colisiones puras
    ax2 = plt.subplot(2, 2, 2)
    ax2.hist(vy_eq, bins=45, color='#ffb3b3', edgecolor='black', density=True, alpha=0.8, label='Simulación vy')
    vy_eje = np.linspace(np.min(vy_eq) * 1.2, np.max(vy_eq) * 1.2, 250)
    f_gauss_Y = np.sqrt(m / (2 * np.pi * kB * T_viento)) * np.exp(-m * vy_eje**2 / (2 * kB * T_viento))
    ax2.plot(vy_eje, f_gauss_Y, color='#cc0000', linewidth=2.5, label=f'Gauss Estándar (Centrada en 0)')
    ax2.set_title('Distribución Termalizada de vy', fontsize=11, fontweight='bold')
    ax2.set_xlabel('Velocidad vy', fontsize=10)
    ax2.set_ylabel('Densidad de Probabilidad', fontsize=10)
    ax2.legend(shadow=True)
    ax2.grid(True, linestyle=':', alpha=0.5)

    # 2.1 Graficación rápida de la evolución energética
    datos_energia = np.loadtxt(f'{prefijo}energias.dat')
    ax3 = plt.subplot(2, 2, (3, 4))
    ax3.plot(datos_energia[:, 0], datos_energia[:, 1], label='E. Cinética', color='#1f77b4')
    ax3.plot(datos_energia[:, 0], datos_energia[:, 2], label='E. Potencial', color='#ff7f0e')
    ax3.plot(datos_energia[:, 0], datos_energia[:, 3], label='E. Total', color='#2ca02c', linewidth=2)
    ax3.set_title('Relajación Energética bajo condiciones de Viento Neto', fontsize=12, fontweight='bold')
    ax3.set_xlabel('Tiempo (t)')
    ax3.set_ylabel('Energía')
    ax3.legend(shadow=True)
    ax3.grid(True, linestyle='--', alpha=0.6)

    # Guardado de la imagen unificada
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.savefig(f'{prefijo}resultados_combinados.png', dpi=300)
    plt.close()

# ==============================================================================
# 3. GENERADOR AUTOMÁTICO DE ANIMACIÓN (Evolución espacial del Gas en GIF)
# ==============================================================================
# MODIFICACIÓN: Se cambian los parámetros para exportar en formato GIF de forma nativa
# optimizando la visualización directa dentro de reportes de Jupyter Notebook.
def generar_video_sistema(prefijo_base, nombre_gif_salida, N_particulas=100, L_caja=22.0):
    """
    Renderiza de forma optimizada la evolución dinámica de los átomos en la caja.
    Genera un archivo GIF animado fluido y ligero usando Pillow de forma directa.
    """
    print(f"\n[INFO] Generando renderización dinámica desde '{prefijo_base}posParticulas.dat'...")
    
    # En los apartados del 4 al 7 el archivo generado por el C++ termina en 'evolucion.dat'
    # Hacemos una comprobación inteligente para abrir el archivo correcto según el apartado.
    archivo_lectura = f'{prefijo_base}posParticulas.dat'
    if not os.path.exists(archivo_lectura):
        archivo_lectura = f'{prefijo_base}evolucion.dat'
        
    if not os.path.exists(archivo_lectura):
        print(f"[ERROR] Archivo de posiciones ausente ({prefijo_base}). Imposible animar.")
        return

    with open(archivo_lectura, 'r') as f:
        lineas = f.readlines()

    fotogramas_totales = []
    fotograma_en_construccion = []
    
    for linea in lineas:
        linea = linea.strip()
        if linea:
            fotograma_en_construccion.append([float(c) for c in linea.split(',')])
            if len(fotograma_en_construccion) == N_particulas:
                fotogramas_totales.append(fotograma_en_construccion)
                fotograma_en_construccion = []

    coords = np.array(fotogramas_totales)
    
    # Submuestreo adaptativo para acelerar el renderizado sin perder fluidez.
    # En los apartados 4-7 guardamos cada 500 pasos, por lo que no es necesario recortar tanto.
    paso_submuestreo = 40 if "posParticulas" in archivo_lectura else 1
    coords_reducidas = coords[::paso_submuestreo] 

    fig, ax = plt.subplots(figsize=(5.5, 5.5))
    ax.set_xlim(0, L_caja)
    ax.set_ylim(0, L_caja)
    ax.set_aspect('equal')
    ax.set_title("Evolución Dinámica Temporal del Gas", fontsize=11, fontweight='bold')
    
    # Dibujado inicial estético de los átomos como esferas rígidas virtuales
    scat = ax.scatter([], [], c='#4363d8', edgecolors='black', s=45, alpha=0.85)
    
    def init():
        scat.set_offsets(np.empty((0, 2)))
        return scat,
        
    def update(frame):
        scat.set_offsets(coords_reducidas[frame])
        return scat,
    
    # Configuramos la animación
    ani = animation.FuncAnimation(fig, update, frames=len(coords_reducidas), init_func=init, blit=True, interval=40)
    
    # MODIFICACIÓN CLAVE: Cambiamos el guardado tradicional a un encoder nativo de GIFs (Pillow)
    try:
        escritor = animation.PillowWriter(fps=24, metadata=dict(artist='MD_Simulation'))
        ani.save(nombre_gif_salida, writer=escritor)
        print(f"   -> ¡Éxito! Animación guardada correctamente como '{nombre_gif_salida}'")
    except Exception as e:
        print(f"[ERROR] No se pudo guardar como GIF usando pillow: {e}")
        print("[CONSEJO] Asegúrate de tener instalada la librería 'pillow' en tu entorno python.")
        
    plt.close()

# ==============================================================================
# == NUEVAS FUNCIONES DE ANÁLISIS PARA LOS APARTADOS 3, 4, 5, 6, 7 Y 8 ========
# ==============================================================================

# --------------------------------------------------------------------------
# 5. ANÁLISIS DEL APARTADO 3: Ley de los Gases Ideales y Ecuación de Estado
# Esta función lee los pares de (Temperatura, Presión) generados por C++, y realiza un ajuste
# --------------------------------------------------------------------------
def analizar_ap3_ecuacion_estado(archivo_datos="ap3_ecuacion_estado.dat"):
    """
    Lee los pares de (Temperatura, Presión) generados por C++, realiza un ajuste
    lineal por mínimos cuadrados y grafica la Ecuación de Estado macroscópica.
    """
    print(f"\n[INFO] Analizando Apartado 3: Ecuación de Estado desde '{archivo_datos}'")
    if not os.path.exists(archivo_datos):
        print(f"[ERROR] No existe el archivo '{archivo_datos}'.")
        return

    datos = np.loadtxt(archivo_datos)
    temperaturas = datos[:, 0]
    presiones = datos[:, 1]

    # Regresión lineal para evaluar el comportamiento PV = N*kB*T
    pendiente, intercepto, r_value, p_value, std_err = linregress(temperaturas, presiones)
    
    plt.figure(figsize=(7, 5))
    plt.scatter(temperaturas, presiones, color='#1f77b4', s=60, zorder=3, label="Datos de Simulación (MD)")
    
    # Generamos la recta del ajuste
    T_eje = np.linspace(np.min(temperaturas)*0.8, np.max(temperaturas)*1.1, 100)
    P_ajuste = pendiente * T_eje + intercepto
    
    plt.plot(T_eje, P_ajuste, color='#d62728', linestyle='--', linewidth=2, 
             label=f'Ajuste Lineal (P = {pendiente:.4f}·T + {intercepto:.3f})\n$R^2$ = {r_value**2:.4f}')
    
    plt.title('Ecuación de Estado: Presión vs Temperatura', fontsize=12, fontweight='bold')
    plt.xlabel('Temperatura Termodinámica ($T$)', fontsize=11)
    plt.ylabel('Presión del Sistema ($P$)', fontsize=11)
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.legend(loc='best', shadow=True)
    plt.tight_layout()
    plt.savefig('ap3_ajuste_ecuacion_estado.png', dpi=300)
    plt.close()
    print("   -> Gráfica de la Ecuación de Estado guardada como 'ap3_ajuste_ecuacion_estado.png'")

# --------------------------------------------------------------------------
# 6. ANÁLISIS DEL APARTADO 6: Monitoreo de la Fusión Brusca y cálculo del MSD
# Esta función sirve para hacer un análisis detallado de la evolución térmica 
# y estructural durante el proceso de fusión brusca,
# --------------------------------------------------------------------------
def analizar_ap6_fusion(prefijo="ap6_fusion_"):
    """
    Grafica la evolución de la temperatura y el Desplazamiento Cuadrático Medio (MSD)
    para identificar los instantes exactos de cambio de fase causados por los reescalados térmicos.
    """
    print(f"\n[INFO] Analizando Apartado 6: Fusión Estructural y MSD ('{prefijo}')")
    archivo = f"{prefijo}propiedades.dat"
    if not os.path.exists(archivo):
        print(f"[ERROR] Archivo '{archivo}' no encontrado.")
        return

    datos = np.loadtxt(archivo)
    t = datos[:, 0]
    T_inst = datos[:, 1]
    msd = datos[:, 2]

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 7), sharex=True)
    
    # Subplot 1: Temperatura instantánea mostrando las discontinuidades (saltos de 1.5x)
    ax1.plot(t, T_inst, color='#e65c00', linewidth=1.8, label='Temperatura Instantánea')
    # Marcamos los tiempos clave del guión donde ocurren los choques térmicos
    tiempos_choque = [180.0, 270.0, 300.0, 400.0]
    for tc in tiempos_choque:
        ax1.axvline(x=tc, color='red', linestyle=':', alpha=0.7, label='Reescalado térmico (1.5x)' if tc==180.0 else "")
    ax1.set_ylabel('Temperatura ($T$)', fontsize=11)
    ax1.set_title('Evolución Térmica y Difusión (MSD) en la Fusión', fontsize=13, fontweight='bold')
    ax1.grid(True, linestyle=':', alpha=0.6)
    ax1.legend(shadow=True)

    # Subplot 2: Evolución temporal del MSD (El cambio de pendiente delata la licuación)
    ax2.plot(t, msd, color='#2ca02c', linewidth=2, label='MSD del Sistema')
    ax2.set_xlabel('Tiempo ($t$)', fontsize=11)
    ax2.set_ylabel('Desplazamiento Cuadrático Medio (MSD)', fontsize=11)
    ax2.grid(True, linestyle=':', alpha=0.6)
    ax2.legend(shadow=True, loc='upper left')

    plt.tight_layout()
    plt.savefig(f'{prefijo}perfil_fusion_msd.png', dpi=300)
    plt.close()
    print(f"   -> Gráfica de fusión guardada como '{prefijo}perfil_fusion_msd.png'")

# --------------------------------------------------------------------------
# 7. ANÁLISIS DEL APARTADO 7: Calentamiento Cuasiestático y Temperatura Crítica
# Esta función analiza el experimento de calentamiento lento continuo para localizar de forma
# --------------------------------------------------------------------------
def analizar_ap7_calentamiento_lento(prefijo="ap7_critico_"):
    """
    Analiza el experimento de calentamiento lento continuo para localizar de forma
    precisa la temperatura crítica de transición de fase mediante la distancia media de pares.
    """
    print(f"\n[INFO] Analizando Apartado 7: Localización de Temperatura Crítica ('{prefijo}')")
    archivo = f"{prefijo}propiedades.dat"
    if not os.path.exists(archivo):
        print(f"[ERROR] Archivo '{archivo}' no encontrado.")
        return

    datos = np.loadtxt(archivo)
    t = datos[:, 0]
    T_inst = datos[:, 1]
    dist_pares = datos[:, 2]

    fig, ax1 = plt.subplots(figsize=(9, 5))

    # Graficamos la temperatura en el eje principal izquierdo
    color = '#d62728'
    ax1.set_xlabel('Tiempo de Simulación ($t$)', fontsize=11)
    ax1.set_ylabel('Temperatura del Sistema ($T$)', color=color, fontsize=11)
    ax1.plot(t, T_inst, color=color, alpha=0.8, linewidth=1.5, label='Temperatura (Calentamiento lento)')
    ax1.tick_params(axis='y', labelcolor=color)
    ax1.grid(True, linestyle=':', alpha=0.5)

    # Creamos un eje gemelo derecho para superponer el observable de estructura espacial
    ax2 = ax1.twinx()  
    color ='#1f77b4'
    ax2.set_ylabel('Alejamiento Cuadrático Medio de Pares', color=color, fontsize=11)
    ax2.plot(t, dist_pares, color=color, linewidth=2, label='Estructura de Pares')
    ax2.tick_params(axis='y', labelcolor=color)

    plt.title('Calentamiento Cuasiestático: Búsqueda de la Temperatura Crítica', fontsize=12, fontweight='bold')
    fig.tight_layout()
    plt.savefig(f'{prefijo}analisis_temperatura_critica.png', dpi=300)
    plt.close()
    print(f"   -> Gráfica de temperatura crítica guardada como '{prefijo}analisis_temperatura_critica.png'")

# --------------------------------------------------------------------------
# 8. ANÁLISIS DEL APARTADO 8: Función de Distribución Radial g(r)
# Esta función lee los histogramas crudos de conteos generados en C++ para las fases sólida,
# líquida y gaseosa, les aplica la normalización geométrica para sistemas 2D y los compara 
# en un panel unificado para evidenciar el orden estructural.
# --------------------------------------------------------------------------
def analizar_ap8_g_de_r():
    """
    Lee los histogramas crudos de conteos generados en C++ para las fases sólida,
    líquida y gaseosa, les aplica la normalización geométrica para sistemas 2D 
    y los compara en un panel unificado para evidenciar el orden estructural.
    """
    print("\n[INFO] Analizando Apartado 8: Funciones de Distribución Radial g(r)")
    
    archivos_gr = {
        "Sólido":  {"file": "ap8_gr_solido.dat",  "color": "#1f77b4", "L": 12.0},
        "Líquido": {"file": "ap8_gr_liquido.dat", "color": "#ff7f0e", "L": 12.0},
        "Gas":     {"file": "ap8_gr_gas.dat",     "color": "#2ca02c", "L": 35.0}
    }
    
    # Parámetros estrictos de simulación en C++ para la normalización matemática
    N_8 = 144
    num_bins = 40
    r_min, r_max = 1.0, 3.0
    dr = (r_max - r_min) / num_bins
    # El bucle de muestreo en C++ corre desde el paso 50000 al 150000 cada 100 pasos:
    N_frames_muestreados = (150000 - 50000) / 100 

    plt.figure(figsize=(9, 6))

    for fase, info in archivos_gr.items():
        if not os.path.exists(info["file"]):
            print(f"[ERROR] Archivo {info['file']} no disponible. Saltando fase {fase}.")
            continue
            
        datos = np.loadtxt(info["file"])
        r_centros = datos[:, 0]
        conteos_brutos = datos[:, 1]
        
        # --- NORMALIZACIÓN GEOMÉTRICA EN DOS DIMENSIONES ---
        # Densidad macroscópica del sistema rho = N / Area
        rho = N_8 / (info["L"]**2)
        
        # El área de una corona esférica diferencial en 2D es: dA = 2 * PI * r * dr
        dA = 2.0 * np.pi * r_centros * dr
        
        # Conteo ideal esperado en un Gas Ideal libre de interacciones:
        conteo_ideal = N_8 * rho * dA * N_frames_muestreados
        
        # g(r) adimensional = Conteo Real / Conteo Ideal
        g_r = conteos_brutos / conteo_ideal

        plt.plot(r_centros, g_r, color=info["color"], linewidth=2, label=f'Fase {fase} (L={info["L"]})')

    plt.axhline(y=1.0, color='black', linestyle='--', alpha=0.5, label='Comportamiento Ideal ($g(r) \\rightarrow 1$)')
    plt.title('Función de Distribución Radial $g(r)$ del Gas de Lennard-Jones', fontsize=13, fontweight='bold')
    plt.xlabel('Distancia radial interatómica ($r$)', fontsize=11)
    plt.ylabel('Función de correlación par $g(r)$', fontsize=11)
    plt.xlim(1.0, 3.0)
    plt.ylim(bottom=0.0)
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.legend(shadow=True, fontsize=10)
    plt.tight_layout()
    plt.savefig('ap8_comparativa_g_de_r.png', dpi=300)
    plt.close()
    print("   -> Comparativa estructural g(r) guardada como 'ap8_comparativa_g_de_r.png'")


# ==============================================================================
# 4. LANZADOR PRINCIPAL DEL PIPELINE (Diccionario de Configuración Modular)
# Este bloque ejecuta de forma secuencial y ordenada cada función de análisis para los diferentes apartados.
# ==============================================================================
if __name__ == "__main__":
    print("======================================================================")
    print("           INICIANDO AUTOMATIZACIÓN DE ANÁLISIS CIENTÍFICO             ")
    print("======================================================================")
    
    # ==========================================================================
    # CONFIGURACIÓN FLEXIBLE: Modifica libremente los prefijos de esta lista 
    # para que coincidan con los que ponga en el main() del código de C++.
    # ==========================================================================
    casos_estandar = [
        {"prefijo": "ap1_",      "titulo": "Modulo inicial v = 1.0"},
        {"prefijo": "ap2_v2_",   "titulo": "Modulo inicial v = 2.0"},
        {"prefijo": "ap2_v3_",   "titulo": "Modulo inicial v = 3.0"},
        {"prefijo": "ap2_v4_",   "titulo": "Modulo inicial v = 4.0"}
    ]
    
    prefijo_viento = "ap2_viento_"
    
    # Ejecución secuencial en bloque para los experimentos base e intermedios (Apartado 1 y 2)
    for caso in casos_estandar:
        analizar_caso_estandar(caso["prefijo"], caso["titulo"])
        # MODIFICACIÓN: Cambio de extensión a .gif
        generar_video_sistema(caso["prefijo"], f'{caso["prefijo"]}evolucion_gas.gif')
        
    # Ejecución aislada para el tratamiento del viento no isótropo (Apartado 2)
    analizar_caso_viento(prefijo_viento)
    # MODIFICACIÓN: Cambio de extensión a .gif
    generar_video_sistema(prefijo_viento, f'{prefijo_viento}evolucion_gas.gif')

    # ==========================================================================
    # NUEVO: Bloque ejecutor automatizado para las secciones complejas (Apartados 3 al 8)
    # ==========================================================================
    print("\n======================================================================")
    # Ejecuta el ajuste y graficación de los coeficientes de la ley del gas ideal
    analizar_ap3_ecuacion_estado()
    
    print("\n======================================================================")
    # Procesamiento y renderizado de videos para las relajaciones cristalinas (N=144, L=12.0)
    print("[NUEVO] Renderizando animaciones de relajación de redes (Apartados 4 y 5)...")
    generar_video_sistema("ap4_red_cuadrada_", "ap4_animacion_cristal_cuadrado.gif", N_particulas=144, L_caja=12.0)
    generar_video_sistema("ap5_red_panal_",    "ap5_animacion_cristal_panal.gif",    N_particulas=144, L_caja=12.0)
    generar_video_sistema("ap5_desordenado_",  "ap5_animacion_red_desordenada.gif",  N_particulas=144, L_caja=12.0)

    print("\n======================================================================")
    # Ejecuta el análisis dinámico de la fusión abrupta y graficado del MSD lineal
    analizar_ap6_fusion()
    # Genera opcionalmente el video de la fusión para visualizar cómo se rompe el cristal
    generar_video_sistema("ap6_fusion_", "ap6_animacion_proceso_fusion.gif", N_particulas=144, L_caja=12.0)

    print("\n======================================================================")
    # Analiza el gradiente térmico lento para encontrar la transición de fase estructural
    analizar_ap7_calentamiento_lento()

    print("\n======================================================================")
    # Ejecuta el procesamiento estadístico y comparativa de g(r) para caracterizar el orden
    analizar_ap8_g_de_r()
    
    print("\n======================================================================")
    print(" [PROCESO FINALIZADO] Revisa las gráficas combinadas y las animaciones GIF.")
    print("======================================================================\n")