TARGET = VLCUnityPlugin

SRCS = RenderingPlugin.cpp RenderAPI.cpp RenderAPI_D3D11.cpp Log.cpp

OBJS = $(SRCS:.cpp=.o)

ifeq ($(PLATFORM), uwp)
CPPFLAGS = -DWINAPI_FAMILY=WINAPI_FAMILY_APP -D_UNICODE -DUNICODE -D__MSVCRT_VERSION__=0xE00 -D_WIN32_WINNT=0x0A00
LDFLAGS = -lwindowsapp -shared -Wl,-pdb= -L$(LIB)
CFLAGS = -Wl,-lwindowsapp
CXXFLAGS = -Wl,-lwindowsapp -I./sdk/include/
EXTRA_CRUNTIME = vcruntime140_app
else
CXXFLAGS = -O3 -g -gcodeview -fdebug-prefix-map='/mnt/c/'='c:/' -Wall -I./sdk/include/ -DNDEBUG
LDFLAGS = -static-libgcc -static-libstdc++ -shared -Wl,-pdb= -L$(LIB)
endif

LIB=./sdk/lib/

LIBS = -lvlc -ld3d11 -ld3dcompiler_47 -ldxgi

ifeq ($(ARCH), x86_64)
BIN_PREFIX = x86_64-w64-mingw32
COMPILEFLAG = m64
else
BIN_PREFIX = i686-w64-mingw32
COMPILEFLAG = m32
endif

ifeq ($(TRIAL), 1)
CXXFLAGS += -DSHOW_WATERMARK
endif

OUTPUT = $(TARGET).dll

ifeq ($(PLATFORM), uwp)
WINDOWS_BINARIES = Assets/VLCUnity/Plugins/UWP
else
WINDOWS_BINARIES = Assets/VLCUnity/Plugins/x86_64
endif
CXX = $(BIN_PREFIX)-c++
STRIP = $(BIN_PREFIX)-strip

all: $(OUTPUT)

copy: $(OUTPUT)
ifndef NOSTRIP
	$(STRIP) $(OUTPUT)
endif
	cp -f $(OUTPUT) ../$(WINDOWS_BINARIES)/$(OUTPUT)

clean:
	rm -f $(OUTPUT) $(OBJS)

$(OUTPUT): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(OUTPUT) $(OBJS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -$(COMPILEFLAG) -c -o $@ $<