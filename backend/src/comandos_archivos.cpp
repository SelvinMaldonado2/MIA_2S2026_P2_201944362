#include "comandos_archivos.h"
#include "comandos_admin.h" // por allocateInode, allocateBlock
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

std::vector<std::string> separarPath(std::string path) {
    std::vector<std::string> carpetas;
    std::string temp = "";
    for (char c : path) {
        if (c == '/') {
            if (!temp.empty()) { carpetas.push_back(temp); temp = ""; }
        } else { temp += c; }
    }
    if (!temp.empty()) carpetas.push_back(temp);
    return carpetas;
}

int buscarInodoPorNombre(std::fstream& archivo, SuperBlock& sb, int inodoActual, const std::string& nombreBuscado) {
    Inode inodo;
    archivo.seekg(sb.s_inode_start + (inodoActual * sizeof(Inode)), std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&inodo), sizeof(Inode));

    for (int i = 0; i < 12; i++) {
        if (inodo.i_block[i] != -1) {
            FolderBlock fb;
            archivo.seekg(sb.s_block_start + (inodo.i_block[i] * sizeof(FolderBlock)), std::ios::beg);
            archivo.read(reinterpret_cast<char*>(&fb), sizeof(FolderBlock));
            
            for (int j = 0; j < 4; j++) {
                if (fb.b_content[j].b_inodo != -1) {
                    if (std::string(fb.b_content[j].b_name) == nombreBuscado) {
                        return fb.b_content[j].b_inodo;
                    }
                }
            }
        }
    }
    return -1;
}

int buscarDirectorio(std::fstream& archivo, SuperBlock& sb, std::string path) {
    std::vector<std::string> carpetas = separarPath(path);
    int inodoActual = 0; // Raiz
    
    for (const std::string& carpeta : carpetas) {
        int encontrado = buscarInodoPorNombre(archivo, sb, inodoActual, carpeta);
        if (encontrado == -1) return -1;
        inodoActual = encontrado;
    }
    return inodoActual;
}

int crearEntradaCarpeta(std::fstream& archivo, SuperBlock& sb, int inodoPadre, std::string nombre, int tipo, int inodoApuntado) {
    Inode padre;
    archivo.seekg(sb.s_inode_start + (inodoPadre * sizeof(Inode)), std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&padre), sizeof(Inode));

    bool agregado = false;
    for (int i = 0; i < 12; i++) {
        if (padre.i_block[i] != -1) {
            FolderBlock fb;
            archivo.seekg(sb.s_block_start + (padre.i_block[i] * sizeof(FolderBlock)), std::ios::beg);
            archivo.read(reinterpret_cast<char*>(&fb), sizeof(FolderBlock));
            
            for (int j = 0; j < 4; j++) {
                if (fb.b_content[j].b_inodo == -1) {
                    std::strncpy(fb.b_content[j].b_name, nombre.c_str(), 11);
                    fb.b_content[j].b_inodo = inodoApuntado;
                    archivo.seekp(sb.s_block_start + (padre.i_block[i] * sizeof(FolderBlock)), std::ios::beg);
                    archivo.write(reinterpret_cast<char*>(&fb), sizeof(FolderBlock));
                    agregado = true;
                    break;
                }
            }
        } else {
            // Asignar nuevo bloque de carpeta
            int bIdx = allocateBlock(archivo, sb, 0);
            if (bIdx == -1) return -1;
            padre.i_block[i] = bIdx;
            
            FolderBlock fb;
            for(int j=0; j<4; j++) fb.b_content[j].b_inodo = -1;
            std::strncpy(fb.b_content[0].b_name, nombre.c_str(), 11);
            fb.b_content[0].b_inodo = inodoApuntado;
            
            archivo.seekp(sb.s_block_start + (bIdx * sizeof(FolderBlock)), std::ios::beg);
            archivo.write(reinterpret_cast<char*>(&fb), sizeof(FolderBlock));
            agregado = true;
        }
        if (agregado) break;
    }
    
    if(agregado) {
        archivo.seekp(sb.s_inode_start + (inodoPadre * sizeof(Inode)), std::ios::beg);
        archivo.write(reinterpret_cast<char*>(&padre), sizeof(Inode));
        return 0; // Exito
    }
    return -1; // No cupo en 12 bloques directos (simplificado)
}

std::string ejecutarMkdir(const std::vector<Token>& parametros) {
    if(!usuarioActual.activo) return "{\"error\": \"MKDIR: Debes iniciar sesión primero.\"}";
    std::string path = ""; bool p_recursivo = false;
    for(const auto& t: parametros) {
        if(t.parametro == "-path") path = limpiarComillas(t.valor);
        else if(t.parametro == "-p") p_recursivo = true;
    }
    if(path.empty()) return "{\"error\": \"MKDIR: Faltan parámetros.\"}";

    std::string diskPath; Partition partTarget;
    if(!getActivePartition(usuarioActual.id_particion, diskPath, partTarget)) return "{\"error\": \"MKDIR: Particion no encontrada.\"}";
    
    std::fstream archivo(diskPath, std::ios::in | std::ios::out | std::ios::binary);
    SuperBlock sb; archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    std::vector<std::string> rutas = separarPath(path);
    int inodoPadre = 0; // Comienza en raiz
    
    for (size_t i = 0; i < rutas.size(); i++) {
        int inodoHijo = buscarInodoPorNombre(archivo, sb, inodoPadre, rutas[i]);
        if (inodoHijo == -1) {
            // Crear si no existe
            if (i < rutas.size() - 1 && !p_recursivo) {
                return "{\"error\": \"MKDIR: Carpeta padre no existe y no se uso -p.\"}";
            }
            int nuevoInodoIdx = allocateInode(archivo, sb, partTarget.part_start);
            if(nuevoInodoIdx == -1) return "{\"error\": \"MKDIR: No hay inodos libres.\"}";
            
            Inode nuevo;
            nuevo.i_uid = usuarioActual.id_user; nuevo.i_gid = usuarioActual.id_group;
            nuevo.i_size = 0; nuevo.i_type = '1'; nuevo.i_perm = 664;
            nuevo.i_ctime = std::time(nullptr); nuevo.i_mtime = std::time(nullptr);
            int nBloque = allocateBlock(archivo, sb, partTarget.part_start);
            nuevo.i_block[0] = nBloque;
            
            FolderBlock fbPropio;
            for(int j=0; j<4; j++) fbPropio.b_content[j].b_inodo = -1;
            std::strncpy(fbPropio.b_content[0].b_name, ".", 11); fbPropio.b_content[0].b_inodo = nuevoInodoIdx;
            std::strncpy(fbPropio.b_content[1].b_name, "..", 11); fbPropio.b_content[1].b_inodo = inodoPadre;
            
            archivo.seekp(sb.s_block_start + (nBloque * sizeof(FolderBlock)), std::ios::beg);
            archivo.write(reinterpret_cast<char*>(&fbPropio), sizeof(FolderBlock));
            
            archivo.seekp(sb.s_inode_start + (nuevoInodoIdx * sizeof(Inode)), std::ios::beg);
            archivo.write(reinterpret_cast<char*>(&nuevo), sizeof(Inode));
            
            crearEntradaCarpeta(archivo, sb, inodoPadre, rutas[i], 1, nuevoInodoIdx);
            inodoPadre = nuevoInodoIdx;
        } else {
            inodoPadre = inodoHijo; // Entrar a esa carpeta
        }
    }

    archivo.seekp(partTarget.part_start, std::ios::beg);
    archivo.write(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));
    archivo.close();
    return "{\"mensaje\": \"¡Directorio " + path + " creado con éxito!\"}";
}

std::string ejecutarMkfile(const std::vector<Token>& parametros) {
    if(!usuarioActual.activo) return "{\"error\": \"MKFILE: Debes iniciar sesión primero.\"}";
    std::string path = "", cont = ""; int size = 0; bool r_recursivo = false;
    for(const auto& t: parametros) {
        if(t.parametro == "-path") path = limpiarComillas(t.valor);
        else if(t.parametro == "-size") size = std::stoi(t.valor);
        else if(t.parametro == "-r") r_recursivo = true;
        // else cont
    }
    if(path.empty()) return "{\"error\": \"MKFILE: Faltan parámetros.\"}";

    std::string diskPath; Partition partTarget;
    if(!getActivePartition(usuarioActual.id_particion, diskPath, partTarget)) return "{\"error\": \"MKFILE: Particion no encontrada.\"}";
    std::fstream archivo(diskPath, std::ios::in | std::ios::out | std::ios::binary);
    SuperBlock sb; archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    size_t lastSlash = path.find_last_of('/');
    std::string parentPath = (lastSlash == 0) ? "/" : path.substr(0, lastSlash);
    std::string fileName = path.substr(lastSlash + 1);

    int inodoPadre = (parentPath == "/") ? 0 : buscarDirectorio(archivo, sb, parentPath);
    if(inodoPadre == -1) {
        if(r_recursivo) {
            // Podríamos crear la carpeta padre, pero asumiremos simplificación para mantenerlo en foco o retornaremos error por falta de abstraccion del mkdir iterativo.
            return "{\"error\": \"MKFILE: Carpeta padre no existe (Se requiere delegar el parse a Mkdir recursivo avanzado).\"}";
        }
        return "{\"error\": \"MKFILE: La carpeta padre no existe.\"}";
    }

    int nuevoInodoIdx = allocateInode(archivo, sb, partTarget.part_start);
    if(nuevoInodoIdx == -1) return "{\"error\": \"MKFILE: No hay inodos libres.\"}";

    Inode nuevoInodo;
    nuevoInodo.i_uid = usuarioActual.id_user; nuevoInodo.i_gid = usuarioActual.id_group;
    nuevoInodo.i_size = size; nuevoInodo.i_type = '0'; nuevoInodo.i_perm = 664;
    nuevoInodo.i_ctime = std::time(nullptr); nuevoInodo.i_mtime = std::time(nullptr);
    
    int numBloques = (size + 63) / 64; 
    if (size == 0) numBloques = 1;

    for(int i = 0; i < numBloques && i < 12; i++) {
        int bIdx = allocateBlock(archivo, sb, partTarget.part_start);
        if(bIdx == -1) break;
        nuevoInodo.i_block[i] = bIdx;
        
        FileBlock fb;
        for(int j=0; j<64; j++) fb.b_content[j] = '0' + ((j+i) % 10); // Llenado de contenido Dummy
        archivo.seekp(sb.s_block_start + (bIdx * sizeof(FileBlock)), std::ios::beg);
        archivo.write(reinterpret_cast<char*>(&fb), sizeof(FileBlock));
    }

    archivo.seekp(sb.s_inode_start + (nuevoInodoIdx * sizeof(Inode)), std::ios::beg);
    archivo.write(reinterpret_cast<char*>(&nuevoInodo), sizeof(Inode));

    crearEntradaCarpeta(archivo, sb, inodoPadre, fileName, 0, nuevoInodoIdx);

    archivo.seekp(partTarget.part_start, std::ios::beg);
    archivo.write(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));
    archivo.close();
    return "{\"mensaje\": \"¡MKFILE " + fileName + " creado exitosamente en " + parentPath + "!\"}";
}

std::string ejecutarCat(const std::vector<Token>& parametros) {
    if(!usuarioActual.activo) return "{\"error\": \"CAT: Debes iniciar sesión primero.\"}";
    std::vector<std::string> archivosALeer;
    for(const auto& t: parametros) {
        if(t.parametro.find("-file") != std::string::npos) {
            archivosALeer.push_back(limpiarComillas(t.valor));
        }
    }
    if(archivosALeer.empty()) return "{\"error\": \"CAT: Falta parámetro fileN.\"}";

    std::string diskPath; Partition partTarget;
    if(!getActivePartition(usuarioActual.id_particion, diskPath, partTarget)) return "{\"error\": \"CAT: Particion no encontrada.\"}";
    std::fstream archivo(diskPath, std::ios::in | std::ios::binary);
    SuperBlock sb; archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    std::string salidaCompleta = "";

    for(const auto& path: archivosALeer) {
        size_t lastSlash = path.find_last_of('/');
        std::string parentPath = (lastSlash == 0) ? "/" : path.substr(0, lastSlash);
        std::string fileName = path.substr(lastSlash + 1);

        int inodoPadre = (parentPath == "/") ? 0 : buscarDirectorio(archivo, sb, parentPath);
        if(inodoPadre == -1) { salidaCompleta += "Archivo NO Existe: " + path + "\n"; continue; }
        
        int inodoFile = buscarInodoPorNombre(archivo, sb, inodoPadre, fileName);
        if(inodoFile == -1) { salidaCompleta += "Archivo NO Existe: " + path + "\n"; continue; }

        Inode inodo;
        archivo.seekg(sb.s_inode_start + (inodoFile * sizeof(Inode)), std::ios::beg);
        archivo.read(reinterpret_cast<char*>(&inodo), sizeof(Inode));

        salidaCompleta += "--- Contenido de " + fileName + " ---\n";
        for (int i = 0; i < 12; i++) {
            if (inodo.i_block[i] != -1) {
                FileBlock fb;
                archivo.seekg(sb.s_block_start + (inodo.i_block[i] * sizeof(FileBlock)), std::ios::beg);
                archivo.read(reinterpret_cast<char*>(&fb), sizeof(FileBlock));
                salidaCompleta += std::string(fb.b_content, 64);
            }
        }
        salidaCompleta += "\n";
    }

    archivo.close();
    return "{\"mensaje\": \"Consulta Cat de Archivos:\n" + salidaCompleta + "\"}";
}
