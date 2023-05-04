#!/bin/bash

# Contador de segmentos eliminados
eliminados=0

# Obtener los IDs de los segmentos de memoria compartida con permisos 660
ids=$(ipcs -m | grep 660 | awk '{print $2}')

# Comprobar si se encontraron IDs
if [[ -z "$ids" ]]; then
  echo "No se encontraron segmentos de memoria compartida con permisos 660."
  exit 1
fi

# Eliminar los segmentos de memoria compartida correspondientes a los IDs
for id in $ids; do
  ipcrm -m "$id"
  eliminados=$((eliminados + 1))
done

echo "Se eliminaron $eliminados segmentos de memoria compartida con permisos 660 correctamente."
