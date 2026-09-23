#include "comandos_admin.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

std::string ejecutarLogin(const std::vector<Token>& parametros) {
    std::string user = "", pass = "", id = "";
    for (const auto& t : parametros) {
        if (t.parametro == "-user" || t.parametro == "-usr") user = t.valor;
        else if (t.parametro == "-pass" || t.parametro == "-pwd") pass = t.valor;
        else if (t.parametro == "-id") id = t.valor;
    }
    if (user.empty() || pass.empty() || id.empty()) return "{\"error\": \"LOGIN: Faltan parámetros.\"}";
    if (usuarioActual.activo) return "{\"error\": \"Ya hay un usuario logueado.\"}";

    std::string diskPath; Partition partTarget;
    if (!getActivePartition(id, diskPath, partTarget)) return "{\"error\": \"LOGIN: Partición no encontrada.\"}";

    std::fstream archivo(diskPath, std::ios::in | std::ios::binary);
    SuperBlock sb; 
    archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    Inode inodeUsers; 
    archivo.seekg(sb.s_inode_start + sizeof(Inode), std::ios::beg); // Inodo 1 (users.txt)
    archivo.read(reinterpret_cast<char*>(&inodeUsers), sizeof(Inode));

    std::string usersContent = "";
    for(int i=0; i<12; i++) {
        if(inodeUsers.i_block[i] != -1) {
            FileBlock fb; 
            archivo.seekg(sb.s_block_start + (inodeUsers.i_block[i] * sizeof(FileBlock)), std::ios::beg);
            archivo.read(reinterpret_cast<char*>(&fb), sizeof(FileBlock));
            usersContent += fb.b_content;
        }
    }
    archivo.close();

    std::stringstream ss(usersContent); std::string linea; bool logged = false;
    while(std::getline(ss, linea, '\n')) {
        if(linea.empty()) continue;
        std::vector<std::string> parts; 
        std::stringstream ssLinea(linea); 
        std::string p;
        while(std::getline(ssLinea, p, ',')) parts.push_back(p);
        
        if(parts.size() == 5 && parts[1] == "U") {
            if(parts[0] != "0" && parts[3] == user && parts[4] == pass) {
                usuarioActual.activo = true; 
                usuarioActual.id_particion = id;
                usuarioActual.id_user = std::stoi(parts[0]); 
                usuarioActual.username = user;
                logged = true; 
                break;
            }
        }
    }
    
    if(logged) return "{\"mensaje\": \"¡LOGIN exitoso!\", \"user\": \"" + user + "\"}";
    return "{\"error\": \"LOGIN: Credenciales incorrectas.\"}";
}

std::string ejecutarLogout(const std::vector<Token>& parametros) {
    if(!usuarioActual.activo) return "{\"error\": \"No hay usuario logueado.\"}";
    usuarioActual.activo = false; 
    usuarioActual.id_particion = "";
    return "{\"mensaje\": \"¡LOGOUT exitoso!\"}";
}

int allocateBlock(std::fstream& archivo, SuperBlock& sb, int part_start) {
    if (sb.s_free_blocks_count <= 0) return -1;
    archivo.seekg(sb.s_bm_block_start, std::ios::beg);
    char bit;
    for(int i = 0; i < sb.s_blocks_count; i++) {
        archivo.read(&bit, 1);
        if(bit == '0') {
            archivo.seekp(sb.s_bm_block_start + i, std::ios::beg);
            char uno = '1'; archivo.write(&uno, 1);
            sb.s_free_blocks_count--;
            archivo.seekp(part_start, std::ios::beg);
            archivo.write(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));
            return i;
        }
    }
    return -1;
}

int allocateInode(std::fstream& archivo, SuperBlock& sb, int part_start) {
    if (sb.s_free_inodes_count <= 0) return -1;
    archivo.seekg(sb.s_bm_inode_start, std::ios::beg);
    char bit;
    for(int i = 0; i < sb.s_inodes_count; i++) {
        archivo.read(&bit, 1);
        if(bit == '0') {
            archivo.seekp(sb.s_bm_inode_start + i, std::ios::beg);
            char uno = '1'; archivo.write(&uno, 1);
            sb.s_free_inodes_count--;
            archivo.seekp(part_start, std::ios::beg);
            archivo.write(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));
            return i;
        }
    }
    return -1;
}

void rewriteUsersTxt(std::fstream& archivo, SuperBlock& sb, std::string content) {
    Inode inodeUsers; archivo.seekg(sb.s_inode_start + sizeof(Inode), std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&inodeUsers), sizeof(Inode));
    
    inodeUsers.i_size = content.length();
    inodeUsers.i_mtime = std::time(nullptr);
    
    int bytesEscritos = 0;
    int bloqueIdx = 0;
    
    while(bytesEscritos < content.length() && bloqueIdx < 12) {
        if(inodeUsers.i_block[bloqueIdx] == -1) {
            // Pasamos 0 ya que es interno u obviamos part_start
            inodeUsers.i_block[bloqueIdx] = allocateBlock(archivo, sb, 0); 
        }
        
        FileBlock fb;
        std::string chunk = content.substr(bytesEscritos, 64);
        std::strncpy(fb.b_content, chunk.c_str(), 64);
        fb.b_content[63] = '\0'; 
        
        archivo.seekp(sb.s_block_start + (inodeUsers.i_block[bloqueIdx] * sizeof(FileBlock)), std::ios::beg);
        archivo.write(reinterpret_cast<char*>(&fb), sizeof(FileBlock));
        
        bytesEscritos += chunk.length();
        bloqueIdx++;
    }
    archivo.seekp(sb.s_inode_start + sizeof(Inode), std::ios::beg);
    archivo.write(reinterpret_cast<char*>(&inodeUsers), sizeof(Inode));
}

std::string ejecutarMkusr(const std::vector<Token>& parametros) {
    if(!usuarioActual.activo || usuarioActual.username != "root") return "{\"error\": \"MKUSR: Solo root puede ejecutar esto.\"}";
    std::string user = "", pass = "", grp = "";
    for (const auto& t : parametros) {
        if (t.parametro == "-user" || t.parametro == "-usr") user = t.valor;
        else if (t.parametro == "-pass" || t.parametro == "-pwd") pass = t.valor;
        else if (t.parametro == "-grp") grp = t.valor;
    }
    if (user.empty() || pass.empty() || grp.empty()) return "{\"error\": \"MKUSR: Faltan parámetros.\"}";
    if (user.length() > 10 || pass.length() > 10 || grp.length() > 10) return "{\"error\": \"MKUSR: Máximo 10 caracteres.\"}";

    std::string diskPath; Partition partTarget;
    getActivePartition(usuarioActual.id_particion, diskPath, partTarget);
    std::fstream archivo(diskPath, std::ios::in | std::ios::out | std::ios::binary);
    SuperBlock sb; archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    Inode inodeUsers; archivo.seekg(sb.s_inode_start + sizeof(Inode), std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&inodeUsers), sizeof(Inode));
    
    std::string content = "";
    for(int i=0; i<12; i++) {
        if(inodeUsers.i_block[i] != -1) {
            FileBlock fb; archivo.seekg(sb.s_block_start + (inodeUsers.i_block[i] * sizeof(FileBlock)), std::ios::beg);
            archivo.read(reinterpret_cast<char*>(&fb), sizeof(FileBlock));
            content += fb.b_content;
        }
    }

    std::stringstream ss(content); std::string linea; int maxId = 0; bool groupExists = false;
    while(std::getline(ss, linea, '\n')) {
        if(linea.empty()) continue;
        std::vector<std::string> parts; std::stringstream ssLinea(linea); std::string p;
        while(std::getline(ssLinea, p, ',')) parts.push_back(p);
        if(parts[0] != "0" && parts.size() >= 3) {
            int id = std::stoi(parts[0]);
            if(id > maxId) maxId = id;
            if(parts[1] == "U" && parts.size() >= 4 && parts[3] == user) return "{\"error\": \"MKUSR: Usuario ya existe.\"}";
            if(parts[1] == "G" && parts[2] == grp) groupExists = true;
        }
    }
    if(!groupExists) return "{\"error\": \"MKUSR: El grupo no existe.\"}";

    content += std::to_string(maxId + 1) + ",U," + grp + "," + user + "," + pass + "\n";
    rewriteUsersTxt(archivo, sb, content);
    archivo.close();
    return "{\"mensaje\": \"¡Usuario '" + user + "' creado exitosamente!\"}";
}

std::string ejecutarRmusr(const std::vector<Token>& parametros) {
    if(!usuarioActual.activo || usuarioActual.username != "root") return "{\"error\": \"RMUSR: Solo root puede ejecutar esto.\"}";
    std::string user = "";
    for (const auto& t : parametros) if (t.parametro == "-user") user = t.valor;
    if (user.empty()) return "{\"error\": \"RMUSR: Faltan parámetros.\"}";

    std::string diskPath; Partition partTarget;
    getActivePartition(usuarioActual.id_particion, diskPath, partTarget);
    std::fstream archivo(diskPath, std::ios::in | std::ios::out | std::ios::binary);
    SuperBlock sb; archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    Inode inodeUsers; archivo.seekg(sb.s_inode_start + sizeof(Inode), std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&inodeUsers), sizeof(Inode));
    std::string content = "";
    for(int i=0; i<12; i++) {
        if(inodeUsers.i_block[i] != -1) {
            FileBlock fb; archivo.seekg(sb.s_block_start + (inodeUsers.i_block[i] * sizeof(FileBlock)), std::ios::beg);
            archivo.read(reinterpret_cast<char*>(&fb), sizeof(FileBlock));
            content += fb.b_content;
        }
    }

    std::string newContent = "";
    std::stringstream ss(content); std::string linea; bool found = false;
    while(std::getline(ss, linea, '\n')) {
        if(linea.empty()) continue;
        std::vector<std::string> parts; std::stringstream ssLinea(linea); std::string p;
        while(std::getline(ssLinea, p, ',')) parts.push_back(p);
        if(parts.size() == 5 && parts[1] == "U" && parts[3] == user && parts[0] != "0") {
            newContent += "0,U," + parts[2] + "," + parts[3] + "," + parts[4] + "\n";
            found = true;
        } else {
            newContent += linea + "\n";
        }
    }
    if(!found) return "{\"error\": \"RMUSR: Usuario no existe o ya está eliminado.\"}";

    rewriteUsersTxt(archivo, sb, newContent);
    archivo.close();
    return "{\"mensaje\": \"¡Usuario eliminado (desactivado) exitosamente!\"}";
}

// EjecutarMkfile eliminado para centralizarse en comandos_archivos

std::string ejecutarMkgrp(const std::vector<Token>& parametros) {
    if(!usuarioActual.activo || usuarioActual.username != "root") return "{\"error\": \"MKGRP: Solo root puede ejecutar esto.\"}";
    std::string name = "";
    for (const auto& t : parametros) if (t.parametro == "-name") name = t.valor;
    if (name.empty()) return "{\"error\": \"MKGRP: Faltan parámetros.\"}";
    if (name.length() > 10) return "{\"error\": \"MKGRP: Máximo 10 caracteres.\"}";

    std::string diskPath; Partition partTarget;
    getActivePartition(usuarioActual.id_particion, diskPath, partTarget);
    std::fstream archivo(diskPath, std::ios::in | std::ios::out | std::ios::binary);
    SuperBlock sb; archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    Inode inodeUsers; archivo.seekg(sb.s_inode_start + sizeof(Inode), std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&inodeUsers), sizeof(Inode));
    
    std::string content = "";
    for(int i=0; i<12; i++) {
        if(inodeUsers.i_block[i] != -1) {
            FileBlock fb; archivo.seekg(sb.s_block_start + (inodeUsers.i_block[i] * sizeof(FileBlock)), std::ios::beg);
            archivo.read(reinterpret_cast<char*>(&fb), sizeof(FileBlock));
            content += fb.b_content;
        }
    }

    std::stringstream ss(content); std::string linea; int maxId = 0; 
    while(std::getline(ss, linea, '\n')) {
        if(linea.empty()) continue;
        std::vector<std::string> parts; std::stringstream ssLinea(linea); std::string p;
        while(std::getline(ssLinea, p, ',')) parts.push_back(p);
        if(parts[0] != "0" && parts.size() >= 3) {
            int id = std::stoi(parts[0]);
            if(id > maxId) maxId = id;
            if(parts[1] == "G" && parts[2] == name) return "{\"error\": \"MKGRP: El grupo ya existe.\"}";
        }
    }

    content += std::to_string(maxId + 1) + ",G," + name + "\n";
    rewriteUsersTxt(archivo, sb, content);
    archivo.close();
    return "{\"mensaje\": \"¡Grupo '" + name + "' creado exitosamente!\"}";
}

std::string ejecutarRmgrp(const std::vector<Token>& parametros) {
    if(!usuarioActual.activo || usuarioActual.username != "root") return "{\"error\": \"RMGRP: Solo root puede ejecutar esto.\"}";
    std::string name = "";
    for (const auto& t : parametros) if (t.parametro == "-name") name = t.valor;
    if (name.empty()) return "{\"error\": \"RMGRP: Faltan parámetros.\"}";

    std::string diskPath; Partition partTarget;
    getActivePartition(usuarioActual.id_particion, diskPath, partTarget);
    std::fstream archivo(diskPath, std::ios::in | std::ios::out | std::ios::binary);
    SuperBlock sb; archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    Inode inodeUsers; archivo.seekg(sb.s_inode_start + sizeof(Inode), std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&inodeUsers), sizeof(Inode));
    std::string content = "";
    for(int i=0; i<12; i++) {
        if(inodeUsers.i_block[i] != -1) {
            FileBlock fb; archivo.seekg(sb.s_block_start + (inodeUsers.i_block[i] * sizeof(FileBlock)), std::ios::beg);
            archivo.read(reinterpret_cast<char*>(&fb), sizeof(FileBlock));
            content += fb.b_content;
        }
    }

    std::string newContent = "";
    std::stringstream ss(content); std::string linea; bool found = false;
    while(std::getline(ss, linea, '\n')) {
        if(linea.empty()) continue;
        std::vector<std::string> parts; std::stringstream ssLinea(linea); std::string p;
        while(std::getline(ssLinea, p, ',')) parts.push_back(p);
        if(parts.size() >= 3 && parts[1] == "G" && parts[2] == name && parts[0] != "0") {
            newContent += "0,G," + parts[2] + "\n";
            found = true;
        } else {
            newContent += linea + "\n";
        }
    }
    if(!found) return "{\"error\": \"RMGRP: El grupo no existe o ya está eliminado.\"}";

    rewriteUsersTxt(archivo, sb, newContent);
    archivo.close();
    return "{\"mensaje\": \"¡Grupo eliminado (desactivado) exitosamente!\"}";
}

std::string ejecutarChgrp(const std::vector<Token>& parametros) {
    if(!usuarioActual.activo || usuarioActual.username != "root") return "{\"error\": \"CHGRP: Solo root puede ejecutar esto.\"}";
    std::string user = "", grp = "";
    for (const auto& t : parametros) {
        if (t.parametro == "-user") user = t.valor;
        else if (t.parametro == "-grp") grp = t.valor;
    }
    if (user.empty() || grp.empty()) return "{\"error\": \"CHGRP: Faltan parámetros.\"}";

    std::string diskPath; Partition partTarget;
    getActivePartition(usuarioActual.id_particion, diskPath, partTarget);
    std::fstream archivo(diskPath, std::ios::in | std::ios::out | std::ios::binary);
    SuperBlock sb; archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    Inode inodeUsers; archivo.seekg(sb.s_inode_start + sizeof(Inode), std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&inodeUsers), sizeof(Inode));
    std::string content = "";
    for(int i=0; i<12; i++) {
        if(inodeUsers.i_block[i] != -1) {
            FileBlock fb; archivo.seekg(sb.s_block_start + (inodeUsers.i_block[i] * sizeof(FileBlock)), std::ios::beg);
            archivo.read(reinterpret_cast<char*>(&fb), sizeof(FileBlock));
            content += fb.b_content;
        }
    }

    bool groupExists = false;
    std::stringstream ss(content); std::string linea;
    while(std::getline(ss, linea, '\n')) {
        if(linea.empty()) continue;
        std::vector<std::string> parts; std::stringstream ssLinea(linea); std::string p;
        while(std::getline(ssLinea, p, ',')) parts.push_back(p);
        if(parts[0] != "0" && parts.size() >= 3 && parts[1] == "G" && parts[2] == grp) {
            groupExists = true; break;
        }
    }
    if(!groupExists) return "{\"error\": \"CHGRP: El grupo especificado no existe.\"}";

    std::string newContent = "";
    std::stringstream ss2(content); bool foundUser = false;
    while(std::getline(ss2, linea, '\n')) {
        if(linea.empty()) continue;
        std::vector<std::string> parts; std::stringstream ssLinea(linea); std::string p;
        while(std::getline(ssLinea, p, ',')) parts.push_back(p);
        if(parts.size() == 5 && parts[1] == "U" && parts[3] == user && parts[0] != "0") {
            newContent += parts[0] + ",U," + grp + "," + parts[3] + "," + parts[4] + "\n";
            foundUser = true;
        } else {
            newContent += linea + "\n";
        }
    }
    if(!foundUser) return "{\"error\": \"CHGRP: El usuario no existe o está eliminado.\"}";

    rewriteUsersTxt(archivo, sb, newContent);
    archivo.close();
    return "{\"mensaje\": \"¡Grupo cambiado exitosamente para el usuario '" + user + "'!\"}";
}
