all:
	cd libs && sudo $(MAKE) install
	cd filesystem && $(MAKE)
	cd memory && $(MAKE)
	cd kernel && $(MAKE)
