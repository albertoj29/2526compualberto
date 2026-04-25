# PROGRAMA PARA GENERAR LA GRÁFICA DE MAGNETIZACIÓN VS TEMPERATURA A PARTIR DE LOS DATOS OBTENIDOS EN ISING.CPP
import numpy as np
import matplotlib.pyplot as plt

# Cargamos los datos: T es la primera columna y M la segunda
# unpack=True sirve para que Python asigne cada columna a una variable
try:
    T, M = np.loadtxt('magnetizacion.txt', unpack=True)
except FileNotFoundError:
    print("Error: No se encuentra 'magnetizacion.txt'. Ejecuta primero el programa C++.")
    exit()

# Creamos la figura de la gráfica
plt.figure(figsize=(9, 6))

# Pintamos los datos: 'bo-' significa puntos Azules (Blue), círculos (o) y línea continua (-)
plt.plot(T, M, 'bo-', label='Magnetización medida')

# Dibujamos una línea roja vertical en la Temperatura Crítica teórica (aprox 2.27)
# He tomado esa temperatura como la teórica ya que es la que fue calculada en 1944 por Lars Onsager para el modelo de Ising 2D sin campo magnético externo
# Además mis resultados experimentales de la PARTE 1, donde obtuve las animaciones para distintas temperaturas, corroboran ese valor, ya que vi que era muy cercana a 2.25
plt.axvline(x=2.269, color='red', linestyle='--', label='Temperatura Crítica teórica ($T_c \\approx 2.27$)')

# Títulos y etiquetas de los ejes
plt.title('Magnetización vs Temperatura (Red 32x32)', fontsize=14)
plt.xlabel('Temperatura (T)', fontsize=12)
plt.ylabel('Magnetización Media |M|', fontsize=12)

# Añadimos una rejilla para que se vea mejor y la leyenda
plt.grid(True, alpha=0.3)
plt.legend()

# Guardamos el resultado en una imagen
print("Generando imagen 'grafica_magnetizacion.png'...")
plt.savefig('grafica_magnetizacion.png') # <--- ¡Aquí estaba el fallo! Ahora está en una sola línea
plt.show()