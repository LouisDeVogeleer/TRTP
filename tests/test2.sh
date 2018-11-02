#!/bin/bash

# cleanup d'un test précédent
rm -f tests/copy_CNP3.pdf

# On lance le receiver et capture sa sortie standard
./receiver -f tests/copy_CNP3.pdf :: 3102 2>receiver.log  &
receiver_pid=$!

cleanup()
{
    kill -9 $receiver_pid
    kill -9 $link_pid
    exit 0
}
trap cleanup SIGINT  # Kill les process en arrière plan en cas de ^-C

# On démarre le transfert
if ! ./sender -f tests/CNP3.pdf ::1 3102 2>sender.log; then
  echo "Crash du sender!"
  cat sender.log
  err=1  # On enregistre l'erreur
fi

sleep 10 # On attend 10 seconde que le receiver finisse

if kill -0 $receiver_pid &> /dev/null ; then
  echo "Le receiver ne s'est pas arreté à la fin du transfert!"
  kill -9 $receiver_pid
  err=1
else  # On teste la valeur de retour du receiver
  if ! wait $receiver_pid ; then
    echo "Crash du receiver!"
    cat receiver.log
    err=1
  fi
fi

# On arrête le simulateur de lien
kill -9 $link_pid &> /dev/null

# On vérifie que le transfert s'est bien déroulé
if [[ "$(md5sum tests/CNP3.pdf | awk '{print $1}')" != "$(md5sum tests/copy_CNP3.pdf | awk '{print $1}')" ]]; then
  echo "Le transfert a corrompu le fichier!"
  echo "Diff binaire des deux fichiers: (attendu vs produit)"
  #diff -C 9 <(od -Ax -t x1z tests/CNP3.pdf) <(od -Ax -t x1z tests/copy_CNP3.pdf)
  exit 1
else
  echo "Le transfert est réussi!"
  exit 0  # En cas d'erreurs avant, on renvoie le code d'erreur
fi
