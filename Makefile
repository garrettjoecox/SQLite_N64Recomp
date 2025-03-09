# Compiler
ZIG = zig c++

# Directories
SRC_DIR = ./src
INCLUDE_DIR = ./include
OFFLINE_BUILD_DIR = offline_build

# Output files
LINUX_OUTPUT = ProxyRecomp_KV.so
WINDOWS_OUTPUT = ProxyRecomp_KV.dll
MACOS_OUTPUT = ProxyRecomp_KV.dylib

# Compiler flags
CXXFLAGS = -shared -fPIC -I $(OFFLINE_BUILD_DIR) -I $(INCLUDE_DIR)

# Targets
all: linux windows macos

linux:
	$(ZIG) -target x86_64-linux $(CXXFLAGS) -o $(LINUX_OUTPUT) $(SRC_DIR)/*

windows:
	$(ZIG) -target x86_64-windows $(CXXFLAGS) -o $(WINDOWS_OUTPUT) $(SRC_DIR)/*

macos:
	$(ZIG) -target aarch64-macos $(CXXFLAGS) -o $(MACOS_OUTPUT) $(SRC_DIR)/*

.PHONY: all linux windows macos
