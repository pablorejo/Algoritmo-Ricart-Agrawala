#!/bin/bash

# Obtener los IDs de los procesos en ejecución de tipo "pagos"
pids=$(pgrep -f "pagos")

if [ -z "$pids" ]; then
  echo "No hay procesos en ejecución de tipo 'pagos'."
else
  echo "Procesos encontrados:"
  echo "$pids"

  # Eliminar los procesos
  echo "Eliminando procesos..."
  kill $pids

  echo "Procesos eliminados."
fi
