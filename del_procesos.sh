#!/bin/bash

# Obtener los IDs de los procesos en ejecución de tipo "pagos"
pids=$(pgrep -f "pagos|administracion|recibir|anulaciones|reservas|consultas")

if [ -z "$pids" ]; then
  echo "No hay procesos en ejecución de los tipos 'pagos', 'administracion', 'recibir', 'anulacion' , 'reservas' o 'consultas'."
else
  echo "Procesos encontrados:"
  echo "$pids"

  # Eliminar los procesos
  echo "Eliminando procesos..."
  kill $pids

  echo "Procesos eliminados."
fi
