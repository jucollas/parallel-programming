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
              src/base/imageFile.cpp \
              src/images/pgmFile.cpp \
              src/images/ppmFile.cpp \
              src/pixel/grayPixel.cpp \
              src/pixel/RGBPixel.cpp \
              src/filters.cpp \
              src/utils.cpp

# -----------------------------
# Ejecutables
# -----------------------------
# 1. filterer
FILTERER_SRCS = apps/filterer.cpp $(COMMON_SRCS) src/base/processor.cpp  src/engines/sequentialEngine.cpp
FILTERER_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(FILTERER_SRCS))
FILTERER_BIN = $(BINDIR)/filterer

# 2. pth_filterer
PTH_SRCS = apps/pth_filterer.cpp $(COMMON_SRCS) src/base/processor.cpp  src/engines/multithreadEngine.cpp
PTH_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(PTH_SRCS))
PTH_BIN = $(BINDIR)/pth_filterer

# 3. omp_filterer
OMP_SRCS = apps/omp_filterer.cpp $(COMMON_SRCS) src/engines/sequentialEngine.cpp
OMP_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(OMP_SRCS))
OMP_BIN = $(BINDIR)/omp_filterer

# -----------------------------
# Reglas principales
# -----------------------------
all: filterer pth_filterer omp_filterer

filterer: $(FILTERER_BIN)
pth_filterer: $(PTH_BIN)
omp_filterer: $(OMP_BIN)

$(FILTERER_BIN): $(FILTERER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(PTH_BIN): $(PTH_OBJS)
	$(CXX) $(CXXFLAGS) -pthread -o $@ $^

$(OMP_BIN): $(OMP_OBJS)
	$(CXX) $(CXXFLAGS) -fopenmp -o $@ $^

# -----------------------------
# Regla genérica para objetos
# -----------------------------
# Todos los archivos de omp_filterer usan -fopenmp
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
ifneq ($(filter %$<,$(OMP_SRCS)),)
	$(CXX) $(CXXFLAGS) -fopenmp -c $< -o $@
else
	$(CXX) $(CXXFLAGS) -c $< -o $@
endif

# -----------------------------
# Limpiar
# -----------------------------
.PHONY: all clean filterer pth_filterer omp_filterer

clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/*
