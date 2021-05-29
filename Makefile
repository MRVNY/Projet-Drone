ROOT = /home/johan/Parrot
#ROOT = /Users/Marvin/PARROT/PARROT-SDK
ROOT = ../..
TEST = Projet-Drone

help:
	@echo
	@echo make connect: connecter au wifi
	@echo make compile: compile
	@echo make run: lancer
	@echo make compirun: compile et lancer
	@echo make rapports: generer les pdf des rapports
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

rapports:
	pdflatex --jobname=RapportPartiel Doc/Rapports/RapportPartiel/Doc1.tex
	pdflatex --jobname=RapportFinal Doc/Rapports/RapportFinal/Doc1.tex
