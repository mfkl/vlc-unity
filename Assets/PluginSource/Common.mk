TARGET = VLCUnityPlugin

ifeq ($(PLATFORM), win)
SRCS = RenderingPlugin.cpp RenderAPI.cpp RenderAPI_D3D11.cpp Log.cpp
else
SRCS = RenderingPlugin.cpp RenderAPI.cpp RenderAPI_Android.cpp RenderAPI_OpenGLBase.cpp RenderAPI_OpenGLEGL.cpp Log.cpp
endif

OBJS = $(SRCS:.cpp=.o)

CXXFLAGS = -O2 -fPIC -fdebug-prefix-map='/mnt/c/'='c:/' -Wall -I./include/ -I/usr/lib/jvm/default-java/include/ -I/usr/lib/jvm/default-java/include/linux/

ifeq ($(ARCH), x86_64)
LIB=/mnt/d/vlc-4.0.0-dev/sdk/lib
else
LIB=/mnt/d/vlc-4.0.0-dev-x86/sdk/lib
endif
LDFLAGS = -static-libgcc -static-libstdc++ -shared -Wl,-pdb= -L$(LIB)

ifeq ($(PLATFORM), win)
LIBS = -lvlc -ld3d11 -ld3dcompiler_47 -ldxgi
else
LIBS = -lvlc -llibdl -lGLESv2
endif

ifeq ($(PLATFORM), win)
BIN_PREFIX = x86_64-w64-mingw32
COMPILEFLAG = -m64
endif

ifeq ($(PLATFORM), win)
OUTPUT = $(TARGET).dll
else
OUTPUT = $(TARGET).so
endif

CXX = g++ -DUNITY_ANDROID=1
STRIP = $(BIN_PREFIX)-strip

all: $(OUTPUT)

copy: $(OUTPUT)
ifndef NOSTRIP
	$(STRIP) $(OUTPUT)
endif
	cp -f $(OUTPUT) ../Assets/$(OUTPUT)

clean:
	rm -f $(OUTPUT) $(OBJS)

$(OUTPUT): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(OUTPUT) $(OBJS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(COMPILEFLAG) -c -o $@ $<