#ifndef COMANDOS_ARCHIVOS_H
#define COMANDOS_ARCHIVOS_H

#include <string>
#include <vector>
#include "utils.h"

std::string ejecutarMkfile(const std::vector<Token>& parametros);
std::string ejecutarMkdir(const std::vector<Token>& parametros);
std::string ejecutarCat(const std::vector<Token>& parametros);

// Funciones auxiliares para recorrer ext2
int buscarInodoPorNombre(std::fstream& archivo, SuperBlock& sb, int inodoActual, const std::string& nombreBuscado);
std::vector<std::string> separarPath(std::string path);
int buscarDirectorio(std::fstream& archivo, SuperBlock& sb, std::string path);

#endif // COMANDOS_ARCHIVOS_H
