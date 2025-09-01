# Compilador
CXX = g++
CXXFLAGS = -Wall -O2 -std=c++17

# Carpetas
OBJDIR = obj
BINDIR = bin

# Crear carpetas si no existen
$(shell mkdir -p $(OBJDIR) $(BINDIR))

# Archivos comunes
COMMON_SRCS = src/base/map.cpp \
              src/base/processor.cpp \
              src/base/imageFile.cpp \
              src/images/pgmFile.cpp \
              src/images/ppmFile.cpp \
              src/pixel/grayPixel.cpp \
              src/pixel/RGBPixel.cpp \
              src/filters.cpp \
              src/utils.cpp

# -----------------------------
# Ejecutable 1: filterer
# -----------------------------
FILTERER_SRCS = apps/filterer.cpp $(COMMON_SRCS) src/engines/sequentialEngine.cpp
FILTERER_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(FILTERER_SRCS))
FILTERER_BIN = $(BINDIR)/filterer

$(FILTERER_BIN): $(FILTERER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(FILTERER_OBJS)

# -----------------------------
# Ejecutable 2: pth_filterer
# -----------------------------
PTH_SRCS = apps/pth_filterer.cpp $(COMMON_SRCS) src/engines/multithreadEngine.cpp
PTH_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(PTH_SRCS))
PTH_BIN = $(BINDIR)/pth_filterer

$(PTH_BIN): $(PTH_OBJS)
	$(CXX) $(CXXFLAGS) -pthread -o $@ $(PTH_OBJS)

# -----------------------------
# Regla para compilar objetos
# -----------------------------
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# -----------------------------
# Comandos útiles
# -----------------------------
.PHONY: all clean filterer pth_filterer

all: filterer pth_filterer

filterer: $(FILTERER_BIN)

pth_filterer: $(PTH_BIN)

clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/*
