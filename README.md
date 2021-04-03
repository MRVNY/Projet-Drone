# Projet-Drone

## Installation de l'API

- Suivre les intructions pour installer le build SDK3 de Parrot (UNIX): https://developer.parrot.com/docs/SDK3/#how-to-build-the-sdk

- Placer ce repertoire dans: ```<chemin du SDK>packages/```


## Lancement du program principal

- Dans le terminal, taper ```make connect``` pour connecter au wifi du drone
- Taper ```make compirun``` pour compiler et lancer le program
- Pour savoir plus de commandes, taper ```make help```

## Lacement des protoypes testables
- Deplacer dans le dir du SDK pour les commandes suivantes(emplacement du fichier build.sh):

  * Compiler le code avec: ```./build.sh -p arsdk-native -A <Nom_du_test>```

  * Executer le code avec: ```./out/arsdk-native/staging/native-wrapper.sh ./out/arsdk-native/staging/usr/bin/<Nom_du_test>```

- Ou simplement, aller dans les repertoires de certains tests, taper ```bash run.sh```
