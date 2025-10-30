# Compiler settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lcryptopp

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Output
TARGET = $(BUILD_DIR)/messageu

# Source files
SRCS = $(SRC_DIR)/main.cc \
       $(SRC_DIR)/client.cc \
       $(SRC_DIR)/protocol.cc \
       $(SRC_DIR)/message.cc \
       $(SRC_DIR)/AESWrapper.cpp \
       $(SRC_DIR)/Base64Wrapper.cpp \
       $(SRC_DIR)/RSAPrivateWrapper.cpp \
       $(SRC_DIR)/RSAPublicWrapper.cpp

# Object files (in build directory)
OBJS = $(BUILD_DIR)/main.o \
       $(BUILD_DIR)/client.o \
       $(BUILD_DIR)/protocol.o \
       $(BUILD_DIR)/message.o \
       $(BUILD_DIR)/AESWrapper.o \
       $(BUILD_DIR)/Base64Wrapper.o \
       $(BUILD_DIR)/RSAPrivateWrapper.o \
       $(BUILD_DIR)/RSAPublicWrapper.o

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compile source files
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cc $(INCLUDE_DIR)/client.h
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cc -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/client.o: $(SRC_DIR)/client.cc $(INCLUDE_DIR)/client.h $(INCLUDE_DIR)/protocol.h $(INCLUDE_DIR)/message.h
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/client.cc -o $(BUILD_DIR)/client.o

$(BUILD_DIR)/protocol.o: $(SRC_DIR)/protocol.cc $(INCLUDE_DIR)/protocol.h
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/protocol.cc -o $(BUILD_DIR)/protocol.o

$(BUILD_DIR)/message.o: $(SRC_DIR)/message.cc $(INCLUDE_DIR)/message.h
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/message.cc -o $(BUILD_DIR)/message.o

$(BUILD_DIR)/AESWrapper.o: $(SRC_DIR)/AESWrapper.cpp $(INCLUDE_DIR)/crypto/AESWrapper.h
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/AESWrapper.cpp -o $(BUILD_DIR)/AESWrapper.o

$(BUILD_DIR)/Base64Wrapper.o: $(SRC_DIR)/Base64Wrapper.cpp $(INCLUDE_DIR)/crypto/Base64Wrapper.h
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/Base64Wrapper.cpp -o $(BUILD_DIR)/Base64Wrapper.o

$(BUILD_DIR)/RSAPrivateWrapper.o: $(SRC_DIR)/RSAPrivateWrapper.cpp $(INCLUDE_DIR)/crypto/RSAPrivateWrapper.h
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/RSAPrivateWrapper.cpp -o $(BUILD_DIR)/RSAPrivateWrapper.o

$(BUILD_DIR)/RSAPublicWrapper.o: $(SRC_DIR)/RSAPublicWrapper.cpp $(INCLUDE_DIR)/crypto/RSAPublicWrapper.h
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/RSAPublicWrapper.cpp -o $(BUILD_DIR)/RSAPublicWrapper.o

# Clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm -f my.info

.PHONY: all clean
