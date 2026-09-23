#ifndef COMANDOS_REP_H
#define COMANDOS_REP_H

#include <string>
#include <vector>
#include "utils.h"

std::string ejecutarRep(const std::vector<Token>& parametros);

// Funciones internas generadoras de DOT
std::string reportarMBR(const std::string& diskPath, const std::string& outputPath);
std::string reportarSuperBlock(const std::string& diskPath, Partition partTarget, const std::string& outputPath);

#endif // COMANDOS_REP_H
