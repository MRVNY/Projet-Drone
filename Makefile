#ROOT = /home/johan/Parrot
#ROOT = /Users/Marvin/PARROT/PARROT-SDK
ROOT = ../..
TEST = Projet-Drone

help:
	@echo
	@echo make connect: connecter au wifi
	@echo make compile: compile
	@echo make run: lancer
	@echo make compirun: compile et lancer
	#@echo make proto KEY=<nom de proto>
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

runproto:
	echo $(ls Proto | grep $(KEY))
	#$(ROOT)/build.sh -p native -A $(TEST)
	echo $(PROTO)
