# Projet-Drone

Instruction pour lancer les test:

-Avoir télecharger et build le SDK3 de Parrot (UNIX): https://developer.parrot.com/docs/SDK3/#how-to-build-the-sdk

-Placer le dossier du test dans: chemin/du/SDK/packages/Samples

-Etre dans le dir du SDK pour les commandes suivantes(emplacement du fichier build.sh):

  -Compiler le code avec: ./build.sh -p arsdk-native -A <Nom_du_test>

  -Executer le code avec: ./out/arsdk-native/staging/native-wrapper.sh ./out/arsdk-native/staging/usr/bin/<Nom_du_test>
