#include "utils.h"
#include <fstream>
#include <iostream>

std::string limpiarComillas(std::string texto) {
    if (texto.length() >= 2 && texto.front() == '"' && texto.back() == '"') {
        return texto.substr(1, texto.length() - 2);
    }
    return texto;
}

std::string limpiarPath(std::string path) {
    if (path.length() >= 2 && path.front() == '"' && path.back() == '"') {
        path = path.substr(1, path.length() - 2);
    }
    if (path.find_first_not_of(" \t\n\r") != std::string::npos) {
        path.erase(0, path.find_first_not_of(" \t\n\r"));
        path.erase(path.find_last_not_of(" \t\n\r") + 1);
    }
    return path;
}

std::vector<std::string> tokenizarLinea(const std::string& linea) {
    std::vector<std::string> tokens;
    std::string actual = "";
    bool dentroDeComillas = false;

    for (char c : linea) {
        if (c == '"') {
            dentroDeComillas = !dentroDeComillas;
            actual += c;
        } else if (c == ' ' && !dentroDeComillas) {
            if (!actual.empty()) {
                tokens.push_back(actual);
                actual = "";
            }
        } else {
            actual += c;
        }
    }
    if (!actual.empty()) tokens.push_back(actual);
    return tokens;
}

bool getActivePartition(std::string id, std::string& diskPath, Partition& partTarget) {
    for (const auto& pm : listaParticionesMontadas) {
        if (pm.id == id) { diskPath = pm.path; break; }
    }
    if (diskPath.empty()) return false;
    
    std::ifstream archivo(diskPath, std::ios::in | std::ios::binary);
    if (!archivo.is_open()) return false;
    
    MBR mbr; 
    archivo.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    archivo.close();
    
    for (int i = 0; i < 4; i++) {
        for (const auto& pm : listaParticionesMontadas) {
            if (pm.id == id && pm.nombre == std::string(mbr.mbr_partitions[i].part_name)) {
                partTarget = mbr.mbr_partitions[i];
                return true;
            }
        }
    }
    return false;
}
