#ifndef UTILS_H
#define UTILS_H

#include <string>

// Función para verificar si un string termina en un sufijo
bool endsWith(const std::string& str, const std::string& suffix);

// Otra función útil (ejemplo: clamp para limitar valores)
int clamp(int value, int minVal, int maxVal);

#endif // UTILS_H
