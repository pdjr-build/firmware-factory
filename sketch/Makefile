.PHONE: build

build: sketch.ino config.cfg
	cat config.cfg | utils/interpolate sketch.ino
	rm config.cfg
	arduino --upload sketch.ino

config.cfg:
	utils/get-config hardware firmware product > config.cfg
