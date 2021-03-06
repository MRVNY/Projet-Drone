# Projet-Drone

## Instruction pour lancer les tests:

- Avoir télecharger et build le SDK3 de Parrot (UNIX): https://developer.parrot.com/docs/SDK3/#how-to-build-the-sdk

- Placer ce repertoire dans: ```<chemin du SDK>packages/```

- Entrer dans le dir du SDK pour les commandes suivantes(emplacement du fichier build.sh):

  * Compiler le code avec: ```./build.sh -p arsdk-native -A <Nom_du_test>```

  * Executer le code avec: ```./out/arsdk-native/staging/native-wrapper.sh ./out/arsdk-native/staging/usr/bin/<Nom_du_test>```

- Ou simplement, aller dans les repertoires de tests, taper ```bash run.sh```
- Ou encore plus simplement, dans ce repertoire, taper ```bash runProtoPilotage.sh <mots clé du test>``` (ex: ```bash runProtoPilotage.sh Vol```)
