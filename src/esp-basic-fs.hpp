#pragma once

#include <LittleFS.h>
#ifdef ARDUINO_ARCH_ESP32
#include <esp_littlefs.h>
#endif
#include <MD5Builder.h>

// #define BASIC_FS_DEBUG
// debug printing macros
// clang-format off
#ifdef BASIC_FS_DEBUG
#define DEBUG_PRINTER Serial
#define BASIC_FS_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
#define BASIC_FS_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#define BASIC_FS_PRINTF(...) { DEBUG_PRINTER.printf(__VA_ARGS__); }
#else
#define BASIC_FS_PRINT(...) {}
#define BASIC_FS_PRINTLN(...) {}
#define BASIC_FS_PRINTF(...) {}
#endif
// clang-format on

#define FILE_SYSTEM LittleFS

class BasicFS {

  public:
	enum ReadFileError {
		read_ok,
		read_not_exists,
		read_error,
	};

	BasicFS();

	bool setup(bool formatOnFail = false);
	String fileName(String filename);
	String fileMd5(String filename);
	bool exists(String filename);
	uint8_t readFile(String filename, File& file);
	bool writeFile(String& data, String filename, File& file);
	bool appendFile(String& data, String filename, File& file);
	bool renameFile(String filename, String newFilename);

	friend class BasicConfig;
	friend class BasicWebServer;
	friend class BasicLogs;

  private:
	bool _fsStarted;
};

extern BasicFS filesystem;
