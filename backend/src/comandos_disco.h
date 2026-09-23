#ifndef COMANDOS_DISCO_H
#define COMANDOS_DISCO_H

#include <string>
#include <vector>
#include "utils.h"

std::string ejecutarMkdisk(const std::vector<Token>& parametros);
std::string ejecutarRmdisk(const std::vector<Token>& parametros);
std::string ejecutarFdisk(const std::vector<Token>& parametros);
std::string ejecutarMount(const std::vector<Token>& parametros);
std::string ejecutarMounted(const std::vector<Token>& parametros);
std::string ejecutarUnmount(const std::vector<Token>& parametros);

#endif // COMANDOS_DISCO_H
