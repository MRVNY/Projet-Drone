#ROOT = /home/johan/Parrot
#ROOT = /Users/Marvin/PARROT/PARROT-SDK
ROOT = ../..
TEST = Projet-Drone

help:
	@echo
	@echo make connect: connecter au wifi
	@echo make compile: compiler
	@echo make run: lancer
	@echo make compirun: compiler et lancer
	@echo make watch: Tester proto watchdog
	@echo

connect:
	@bash Pilotage/connect.sh

compile:
	$(ROOT)/build.sh -p native -A $(TEST)

run:
	$(ROOT)/out/arsdk-native/staging/native-wrapper.sh $(ROOT)/out/arsdk-native/staging/usr/bin/$(TEST)

compirun:
	$(ROOT)/build.sh -p native -A $(TEST)
	$(ROOT)/out/arsdk-native/staging/native-wrapper.sh $(ROOT)/out/arsdk-native/staging/usr/bin/$(TEST)
	#$(ROOT)/out/arsdk-native/staging/native-darwin-wrapper.sh $(ROOT)/out/arsdk-native/staging/usr/bin/$(TEST)

doc:
	cd src/Rapports; for i in *.tex; do pdflatex $i;done

watch:
	gcc -pthread -o ./Proto/Pilotage/Test_Watchdog/Test_Watchdog ./Proto/Pilotage/Test_Watchdog/Test_Watchdog.c
	./Proto/Pilotage/Test_Watchdog/Test_Watchdog
