arduino-cli compile -v --fqbn esp32:esp32:esp32 main.ino \
	&& arduino-cli upload --fqbn esp32:esp32:esp32 -p $1 main.ino \
	&& screen $1 115200
