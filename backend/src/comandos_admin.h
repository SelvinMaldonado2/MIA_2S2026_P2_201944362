#ifndef COMANDOS_ADMIN_H
#define COMANDOS_ADMIN_H

#include <string>
#include <vector>
#include "utils.h"

std::string ejecutarLogin(const std::vector<Token>& parametros);
std::string ejecutarLogout(const std::vector<Token>& parametros);
std::string ejecutarMkusr(const std::vector<Token>& parametros);
std::string ejecutarRmusr(const std::vector<Token>& parametros);
std::string ejecutarMkfile(const std::vector<Token>& parametros);

int allocateBlock(std::fstream& archivo, SuperBlock& sb, int part_start);
int allocateInode(std::fstream& archivo, SuperBlock& sb, int part_start);
void rewriteUsersTxt(std::fstream& archivo, SuperBlock& sb, std::string content);

std::string ejecutarMkgrp(const std::vector<Token>& parametros);
std::string ejecutarRmgrp(const std::vector<Token>& parametros);
std::string ejecutarChgrp(const std::vector<Token>& parametros);

#endif // COMANDOS_ADMIN_H
