# ================================================================================
# GRAFICAR MOMENTO ANGULAR (UNA SOLA GRÁFICA)
# ================================================================================

import matplotlib.pyplot as plt
import numpy as np

# Leer datos
data = []
with open('data/momento_angular.dat', 'r') as f:
    for line in f:
        if line.strip():
            data.append([float(x) for x in line.split()])

nbodies = 10

times = []
angular_momenta = [[] for _ in range(nbodies)]
total_angular_momentum = []

for row in data:
    times.append(row[0])
    for i in range(nbodies):
        angular_momenta[i].append(row[1 + i])
    total_angular_momentum.append(row[1 + nbodies])

bodies = ['Sol', 'Mercurio', 'Venus', 'Tierra', 'Marte',
          'Júpiter', 'Saturno', 'Urano', 'Neptuno', 'Plutón']

# =========================
# UNA SOLA GRÁFICA
# =========================
plt.figure(figsize=(14, 7))

# Momentos individuales
for i in range(nbodies):
    plt.plot(times, angular_momenta[i], label=bodies[i], linewidth=1)

# Momento total (en la misma gráfica)
plt.plot(times, total_angular_momentum,
         'r--', linewidth=3, label='L_total')

plt.xlabel('Tiempo (unidades adimensionales)')
plt.ylabel('Momento angular')
plt.title('Momento angular individual y total')
plt.legend()
plt.grid(True)

plt.tight_layout()
plt.savefig('momento_angular.png', dpi=300)
plt.show()