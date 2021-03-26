ROOT = /home/johan/Parrot
#ROOT = /Users/Marvin/PARROT/PARROT-SDK
TEST = Proto_commun

help:
	@echo
	@echo make connect: connecter au wifi
	@echo make compile: compile
	@echo make run: lancer
	@echo make comprun: compile et lancer
	@echo

connect:
	@bash connect.sh

compile:
	$(ROOT)/build.sh -p native -A $(TEST)

run:
	$(ROOT)/out/arsdk-native/staging/native-wrapper.sh $(ROOT)/out/arsdk-native/staging/usr/bin/$(TEST)

compirun:
	$(ROOT)/build.sh -p native -A $(TEST)
	$(ROOT)/out/arsdk-native/staging/native-wrapper.sh $(ROOT)/out/arsdk-native/staging/usr/bin/$(TEST)


