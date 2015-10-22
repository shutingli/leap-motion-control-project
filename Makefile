OS := $(shell uname)
ARCH := $(shell uname -m)

ifeq ($(OS), Linux)
  ifeq ($(ARCH), x86_64)
    LEAP_LIBRARY := lib/x64/libLeap.so -Wl,-rpath,lib/x64
  else
    LEAP_LIBRARY := lib/x86/libLeap.so -Wl,-rpath,lib/x86
  endif
else
  # OS X
  LEAP_LIBRARY := lib/libLeap.dylib
endif

control: control.cpp
	$(CXX) -Wall -g -Iinclude control.cpp -o control $(LEAP_LIBRARY)
ifeq ($(OS), Darwin)
	install_name_tool -change @loader_path/libLeap.dylib lib/libLeap.dylib control
endif

clean:
	rm -rf control control.dSYM
