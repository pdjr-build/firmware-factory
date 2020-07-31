.PHONE: build

build: sketch.ino config.cfg
	cat config.cfg | utils/interpolate sketch.ino
	rm *.cfg
	arduino --upload sketch.ino

config.cfg: spudpole.cfg hardware.cfg
	cat spudpole.cfg hardware.cfg | sort | uniq > config.cfg

spudpole.cfg: 
	readlink -f spudpole/
	utils/get-config-value *_ spudpole/ > spudpole.cfg

hardware.cfg:
	readlink -f hardware/
	utils/get-config-value *_ hardware/ > hardware.cfg
	
