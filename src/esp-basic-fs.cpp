#include "esp-basic-fs.hpp"

BasicFS filesystem;

BasicFS::BasicFS()
    : BasicPlugin::BasicPlugin("fs")
    , _fsStarted(false)
    , _freeSpace(0) {
}

bool BasicFS::setup(bool formatOnFail) {
#ifdef ARDUINO_ARCH_ESP32
	if (esp_littlefs_mounted("spiffs")) {
#elif defined(ARDUINO_ARCH_ESP8266)
	FSInfo FsInfo;
	if (FILE_SYSTEM.info(FsInfo)) {
#endif
		BASIC_FS_PRINTLN("file system already mounted!");
		_fsStarted = true;
	} else {
		while (!FILE_SYSTEM.begin()) {
			BASIC_FS_PRINTLN("file system mount failed!");
			_fsStarted = false;
			if (formatOnFail) {
				BASIC_FS_PRINTLN("formatting file system!");
				FILE_SYSTEM.format();
			} else {
				return _fsStarted;
			}
		}
		BASIC_FS_PRINTLN("file system mounted!");
		_fsStarted = true;
	}
	freeSpace(true);
	return _fsStarted;
}

String BasicFS::fileName(String filename) {
#ifdef ARDUINO_ARCH_ESP32
	if (!filename.startsWith("/")) {
		return "/" + filename;
	}
#endif
	return filename;
}
String BasicFS::fileMd5(String filename) {
	File file;
	if (readFile(fileName(filename), file) == read_ok) {
		MD5Builder md5;
		md5.begin();
		md5.add(file.readString());
		md5.calculate();
		file.close();
		BASIC_FS_PRINTF("file %s md5: %s\n", filename.c_str(), md5.toString().c_str());
		return md5.toString();
	}
	file.close();
	return "";
}
bool BasicFS::exists(String filename) {
	return FILE_SYSTEM.exists(fileName(filename));
}
uint8_t BasicFS::readFile(String filename, File& file) {
	if (!FILE_SYSTEM.exists(fileName(filename))) {
		BASIC_FS_PRINTF("%s not found!\n", filename.c_str());
		return read_not_exists;
	}
	file = FILE_SYSTEM.open(fileName(filename), "r");
	if (!file) {
		BASIC_FS_PRINTF("failed to read %s!\n", filename.c_str());
		file.close();
		return read_error;
	}
	BASIC_FS_PRINTF("file %s read successful!\n", filename.c_str());
	return read_ok;
}
bool BasicFS::writeFile(String& data, String filename, File& file) {
	if (freeSpace(true) <= FS_MIN_FREE_SPACE) {
		BASIC_FS_PRINTF("failed to write %s! no space left on the device\n", filename.c_str());
		return false;
	}
	if (data.length() >= freeSpace()) {
		BASIC_FS_PRINTF("failed to write %s! not enough space left on the device\n", filename.c_str());
		return false;
	}

	file = FILE_SYSTEM.open(fileName(filename), "w");
	if (!file) {
		BASIC_FS_PRINTF("failed to write %s!\n", filename.c_str());
		file.close();
		return false;
	}
	file.print(data);
	file.close();
	freeSpace(true);
	BASIC_FS_PRINTF("file %s write successful!\n", filename.c_str());
	return true;
}

bool BasicFS::appendFile(String& data, String filename, File& file) {
	if (freeSpace(true) <= FS_MIN_FREE_SPACE) {
		BASIC_FS_PRINTF("failed to append %s! no space left on the device\n", filename.c_str());
		return false;
	}
	if (data.length() >= freeSpace()) {
		BASIC_FS_PRINTF("failed to append %s! not enough space left on the device\n", filename.c_str());
		return false;
	}

	bool fileExist = FILE_SYSTEM.exists(fileName(filename));
	if (!fileExist) {
		BASIC_FS_PRINTF("file %s(new) appended successfully!\n", filename.c_str());
		return writeFile(data, filename, file);
	}
	file = FILE_SYSTEM.open(fileName(filename), "a");
	if (!file) {
		BASIC_FS_PRINTF("failed to append %s\n", filename.c_str());
		file.close();
		return false;
	}
	file.print(data);
	file.close();
	freeSpace(true);
	BASIC_FS_PRINTF("file %s appended successfully!\n", filename.c_str());
	return true;
}

bool BasicFS::renameFile(String filename, String newFilename) {
	if (!FILE_SYSTEM.exists(fileName(filename))) {
		BASIC_FS_PRINTF("failed to rename %s, file not found\n", filename.c_str());
		return false;
	}
	if (!FILE_SYSTEM.rename(fileName(filename), fileName(newFilename))) {
		BASIC_FS_PRINTF("failed to rename %s\n", filename.c_str());
		return false;
	}
	BASIC_FS_PRINTF("file %s successfully renamed to %s!\n",
	                filename.c_str(), newFilename.c_str());
	return true;
}

size_t BasicFS::freeSpace(bool update) {
	if (!update) { return _freeSpace; }

#ifdef ARDUINO_ARCH_ESP32
	size_t total, used;
	if (esp_littlefs_info("spiffs", &total, &used) == ESP_OK) {
		_freeSpace = total - used;
	}
#elif defined(ARDUINO_ARCH_ESP8266)
	FSInfo FsInfo;
	if (FILE_SYSTEM.info(FsInfo)) {
		_freeSpace = FsInfo.totalBytes - FsInfo.usedBytes;
	}
#endif
	return _freeSpace;
}
