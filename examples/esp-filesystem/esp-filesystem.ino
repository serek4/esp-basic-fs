#include <esp-basic-fs.h>

void setup() {
	Serial.begin(115200);
	Serial.println("");
	filesystem.setup(true);
}

void loop() {
	delay(10);
}
