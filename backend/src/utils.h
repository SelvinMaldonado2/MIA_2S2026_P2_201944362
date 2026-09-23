#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include "estructuras.h"

struct Token {
    std::string parametro;
    std::string valor;
};

std::string limpiarComillas(std::string texto);
std::string limpiarPath(std::string path);
std::vector<std::string> tokenizarLinea(const std::string& linea);

bool getActivePartition(std::string id, std::string& diskPath, Partition& partTarget);

#endif // UTILS_H
