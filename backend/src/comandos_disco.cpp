#include "comandos_disco.h"
#include "estructuras.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

LoggedUser usuarioActual;
std::vector<ParticionMontada> listaParticionesMontadas;

std::string ejecutarMkdisk(const std::vector<Token>& parametros) {
    int size = 0;
    std::string unit = "m";
    std::string path = "";

    for (const auto& token : parametros) {
        if (token.parametro == "-size") {
            try { size = std::stoi(token.valor); } catch (...) { return "{\"error\": \"-size debe ser un número entero.\"}"; }
        } else if (token.parametro == "-unit" || token.parametro == "-u") {
            unit = token.valor;
            for (char &c : unit) c = tolower(c);
        } else if (token.parametro == "-path") {
            path = limpiarPath(token.valor);
        }
    }

    if (size <= 0) return "{\"error\": \"MKDISK: -size es obligatorio y mayor a 0.\"}";
    if (path.empty()) return "{\"error\": \"MKDISK: -path es obligatorio.\"}";

    try {
        fs::path filePath(path);
        fs::path parentDir = filePath.parent_path();
        if (!parentDir.empty() && !fs::exists(parentDir)) fs::create_directories(parentDir);
    } catch (...) {
        return "{\"error\": \"No se pudieron crear las carpetas en Linux.\"}";
    }

    int total_bytes = (unit == "m") ? size * 1024 * 1024 : size * 1024;

    std::ofstream archivo(path, std::ios::out | std::ios::binary);
    if (!archivo.is_open()) return "{\"error\": \"No se pudo crear el archivo en la ruta indicada.\"}";

    char buffer[1024] = {0};
    for (int i = 0; i < (total_bytes / 1024); i++) {
        archivo.write(buffer, 1024);
    }

    MBR nuevo_mbr;
    nuevo_mbr.mbr_tamano = total_bytes;
    nuevo_mbr.mbr_fecha_creacion = std::time(nullptr);
    nuevo_mbr.mbr_dsk_signature = std::rand() % 10000;
    
    archivo.seekp(0, std::ios::beg);
    archivo.write(reinterpret_cast<char*>(&nuevo_mbr), sizeof(MBR));
    archivo.close();

    std::cout << "-> MBR escrito correctamente en: " << path << std::endl;
    return "{\n  \"mensaje\": \"¡Disco creado exitosamente con su MBR!\",\n  \"path\": \"" + path + "\",\n  \"bytes\": " + std::to_string(total_bytes) + "\n}";
}

std::string ejecutarRmdisk(const std::vector<Token>& parametros) {
    std::string path = "";
    for (const auto& token : parametros) {
        if (token.parametro == "-path") path = limpiarPath(token.valor);
    }

    if (path.empty()) return "{\"error\": \"RMDISK: el parámetro -path es obligatorio.\"}";

    if (fs::exists(path)) {
        fs::remove(path);
        std::cout << "-> Disco eliminado: " << path << std::endl;
        return "{\"mensaje\": \"¡Disco eliminado exitosamente de Linux!\", \"path\": \"" + path + "\"}";
    } else {
        return "{\"error\": \"El disco no existe en la ruta indicada.\"}";
    }
}

std::string ejecutarFdisk(const std::vector<Token>& parametros) {
    int size = 0;
    std::string unit = "k";
    std::string path = "";
    std::string type = "p";
    std::string fit = "w";
    std::string name = "";

    for (const auto& token : parametros) {
        if (token.parametro == "-size") {
            try { size = std::stoi(token.valor); } catch (...) { return "{\"error\": \"-size debe ser un número entero.\"}"; }
        } else if (token.parametro == "-unit" || token.parametro == "-u") {
            unit = token.valor;
            for (char &c : unit) c = tolower(c);
        } else if (token.parametro == "-path") {
            path = limpiarPath(token.valor);
        } else if (token.parametro == "-type" || token.parametro == "-t") {
            type = token.valor;
            for (char &c : type) c = tolower(c);
        } else if (token.parametro == "-fit" || token.parametro == "-f") {
            fit = token.valor;
            for (char &c : fit) c = tolower(c);
        } else if (token.parametro == "-name") {
            name = token.valor;
        }
    }

    if (size <= 0) return "{\"error\": \"FDISK: -size es obligatorio y mayor a 0.\"}";
    if (path.empty()) return "{\"error\": \"FDISK: -path es obligatorio.\"}";
    if (name.empty()) return "{\"error\": \"FDISK: -name es obligatorio.\"}";

    if (!fs::exists(path)) {
        return "{\"error\": \"El disco especificado en -path no existe.\"}";
    }

    int bytes_particion = size;
    if (unit == "m") bytes_particion = size * 1024 * 1024;
    else if (unit == "k") bytes_particion = size * 1024;

    std::fstream archivo(path, std::ios::in | std::ios::out | std::ios::binary);
    if (!archivo.is_open()) {
        return "{\"error\": \"No se pudo abrir el archivo del disco para escribir la partición.\"}";
    }

    MBR mbr;
    archivo.seekg(0, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));

    int indiceLibre = -1;
    int contadorPrimariasExtendidas = 0;
    int byteSiguiente = sizeof(MBR);

    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {
            if (mbr.mbr_partitions[i].part_type == 'P' || mbr.mbr_partitions[i].part_type == 'E') {
                contadorPrimariasExtendidas++;
            }
            int finParticion = mbr.mbr_partitions[i].part_start + mbr.mbr_partitions[i].part_size;
            if (finParticion > byteSiguiente) {
                byteSiguiente = finParticion;
            }
        } else {
            if (indiceLibre == -1) indiceLibre = i;
        }
    }

    if (indiceLibre == -1) {
        archivo.close();
        return "{\"error\": \"FDISK: Ya existen 4 particiones en el MBR.\"}";
    }

    char tipoParticion = 'P';
    if (type == "e") {
        tipoParticion = 'E';
        for (int i = 0; i < 4; i++) {
            if (mbr.mbr_partitions[i].part_status == '1' && mbr.mbr_partitions[i].part_type == 'E') {
                archivo.close();
                return "{\"error\": \"FDISK: Ya existe una partición extendida.\"}";
            }
        }
    } else if (type == "l") {
        tipoParticion = 'L';
    }

    if (byteSiguiente + bytes_particion > mbr.mbr_tamano) {
        archivo.close();
        return "{\"error\": \"FDISK: No hay suficiente espacio en el disco.\"}";
    }

    Partition nuevaPart;
    nuevaPart.part_status = '1';
    nuevaPart.part_type = tipoParticion;
    nuevaPart.part_fit = (fit == "bf") ? 'B' : ((fit == "wf") ? 'W' : 'F');
    nuevaPart.part_start = byteSiguiente;
    nuevaPart.part_size = bytes_particion;
    std::strncpy(nuevaPart.part_name, name.c_str(), sizeof(nuevaPart.part_name));
    nuevaPart.part_name[sizeof(nuevaPart.part_name) - 1] = '\0';

    mbr.mbr_partitions[indiceLibre] = nuevaPart;

    archivo.seekp(0, std::ios::beg);
    archivo.write(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    archivo.close();

    std::cout << "-> Partición '" << name << "' creada con éxito." << std::endl;
    return "{\n  \"mensaje\": \"¡Partición creada y guardada en el MBR exitosamente!\",\n  \"path\": \"" + path + "\",\n  \"inicio\": " + std::to_string(byteSiguiente) + ",\n  \"tamanio_bytes\": " + std::to_string(bytes_particion) + "\n}";
}

std::string ejecutarMount(const std::vector<Token>& parametros) {
    std::string path = "";
    std::string name = "";

    for (const auto& token : parametros) {
        if (token.parametro == "-path") {
            path = limpiarPath(token.valor);
        } else if (token.parametro == "-name") {
            name = token.valor;
        }
    }

    if (path.empty()) return "{\"error\": \"MOUNT: El parámetro -path es obligatorio.\"}";
    if (name.empty()) return "{\"error\": \"MOUNT: El parámetro -name es obligatorio.\"}";

    if (!fs::exists(path)) {
        return "{\"error\": \"MOUNT: El disco especificado no existe.\"}";
    }

    std::ifstream archivo(path, std::ios::in | std::ios::binary);
    if (!archivo.is_open()) {
        return "{\"error\": \"MOUNT: No se pudo abrir el archivo del disco.\"}";
    }

    MBR mbr;
    archivo.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    archivo.close();

    int indiceParticion = -1;
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {
            std::string partName(mbr.mbr_partitions[i].part_name);
            if (partName == name) {
                indiceParticion = i;
                break;
            }
        }
    }

    if (indiceParticion == -1) {
        return "{\"error\": \"MOUNT: La partición '" + name + "' no existe en este disco.\"}";
    }

    for (const auto& pm : listaParticionesMontadas) {
        if (pm.path == path && pm.nombre == name) {
            return "{\"error\": \"MOUNT: La partición ya se encuentra montada con el ID: " + pm.id + "\"}";
        }
    }

    // ID Generado (Asumiendo convenciones basicas de la practica 36+letra+num)
    std::string idGenerado = "36" + std::string(1, 'a' + (listaParticionesMontadas.size() % 26)) + std::to_string(indiceParticion + 1);
    listaParticionesMontadas.push_back({idGenerado, path, name});

    std::cout << "-> Partición montada con éxito. ID: " << idGenerado << std::endl;
    return "{\n  \"mensaje\": \"¡Partición montada exitosamente!\",\n  \"id\": \"" + idGenerado + "\",\n  \"path\": \"" + path + "\",\n  \"nombre\": \"" + name + "\"\n}";
}

std::string ejecutarUnmount(const std::vector<Token>& parametros) {
    std::string id = "";

    for (const auto& token : parametros) {
        if (token.parametro == "-id") {
            id = token.valor;
        }
    }

    if (id.empty()) {
        return "{\"error\": \"UNMOUNT: El parámetro -id es obligatorio.\"}";
    }

    bool encontrado = false;
    for (auto it = listaParticionesMontadas.begin(); it != listaParticionesMontadas.end(); ++it) {
        if (it->id == id) {
            listaParticionesMontadas.erase(it);
            encontrado = true;
            break;
        }
    }

    if (!encontrado) {
        return "{\"error\": \"UNMOUNT: No se encontró ninguna partición montada con el ID: " + id + "\"}";
    }

    std::cout << "-> Partición con ID " << id << " desmontada exitosamente." << std::endl;
    return "{\n  \"mensaje\": \"¡Partición desmontada exitosamente!\",\n  \"id\": \"" + id + "\"\n}";
}

std::string ejecutarMounted(const std::vector<Token>& parametros) {
    if (listaParticionesMontadas.empty()) {
        return "{\"mensaje\": \"MOUNTED: No hay particiones montadas actualmente.\"}";
    }
    std::string listado = "--- LISADO DE PARTICIONES MONTADAS ---\\n";
    for(const auto& m : listaParticionesMontadas) {
        listado += "ID: [" + m.id + "] | Disco: " + m.path + " | Part: " + m.nombre + "\\n";
    }
    return "{\"mensaje\": \"" + listado + "\"}";
}
