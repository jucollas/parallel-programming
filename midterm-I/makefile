# Compilador
CXX = g++
CXXFLAGS = -Wall -O2 -std=c++17

CMPI = mpic++

# Carpetas
OBJDIR = obj
BINDIR = bin

# Crear carpetas si no existen
$(shell mkdir -p $(BINDIR))

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
# Ejecutables y sus fuentes
# -----------------------------
FILTERER_SRCS = apps/filterer.cpp $(COMMON_SRCS) src/base/processor.cpp src/engines/sequentialEngine.cpp
PTH_SRCS     = apps/pth_filterer.cpp $(COMMON_SRCS) src/base/processor.cpp src/engines/multithreadEngine.cpp
OMP_SRCS     = apps/omp_filterer.cpp $(COMMON_SRCS) src/engines/sequentialEngine.cpp
MPI_SRCS     = apps/mpi_filterer.cpp $(COMMON_SRCS) src/base/processor.cpp src/engines/distributedMemoryEngine.cpp

# Función para convertir src/*.cpp a obj/*.o
define SRC_TO_OBJ
$(patsubst %.cpp,$(OBJDIR)/%.o,$(1))
endef

FILTERER_OBJS = $(call SRC_TO_OBJ,$(FILTERER_SRCS))
PTH_OBJS     = $(call SRC_TO_OBJ,$(PTH_SRCS))
OMP_OBJS     = $(call SRC_TO_OBJ,$(OMP_SRCS))
MPI_OBJS     = $(call SRC_TO_OBJ,$(MPI_SRCS))

FILTERER_BIN = $(BINDIR)/filterer
PTH_BIN     = $(BINDIR)/pth_filterer
OMP_BIN     = $(BINDIR)/omp_filterer
MPI_BIN     = $(BINDIR)/mpi_filterer

# -----------------------------
# Reglas principales
# -----------------------------
all: filterer pth_filterer omp_filterer mpi_filterer

filterer: $(FILTERER_BIN)
pth_filterer: $(PTH_BIN)
omp_filterer: $(OMP_BIN)
mpi_filterer: $(MPI_BIN)

# -----------------------------
# Compilación de ejecutables
# -----------------------------
$(FILTERER_BIN): $(FILTERER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(PTH_BIN): $(PTH_OBJS)
	$(CXX) $(CXXFLAGS) -pthread -o $@ $^

$(OMP_BIN): $(OMP_OBJS)
	$(CXX) $(CXXFLAGS) -fopenmp -o $@ $^

$(MPI_BIN): $(MPI_OBJS)
	$(CMPI) -o $@ $^

# -----------------------------
# Regla genérica para objetos
# -----------------------------
# Se crean directorios automáticamente
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
ifeq ($(filter $<,$(MPI_SRCS)),)
ifeq ($(filter $<,$(OMP_SRCS)),)
	$(CXX) $(CXXFLAGS) -c $< -o $@
else
	$(CXX) $(CXXFLAGS) -fopenmp -c $< -o $@
endif
else
	$(CMPI) $(CXXFLAGS) -c $< -o $@
endif



# -----------------------------
# Limpiar
# -----------------------------
.PHONY: all clean filterer pth_filterer omp_filterer mpi_filterer

clean:
	rm -rf $(OBJDIR) $(BINDIR)/*
