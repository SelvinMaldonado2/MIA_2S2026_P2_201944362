#include "comandos_ext2.h"
#include <iostream>
#include <fstream>
#include <cstring>

std::string ejecutarMkfs(const std::vector<Token>& parametros) {
    std::string id = "";
    std::string type = "full";

    for (const auto& token : parametros) {
        if (token.parametro == "-id") {
            id = token.valor;
        } else if (token.parametro == "-type" || token.parametro == "-t") {
            type = token.valor;
            for (char &c : type) c = tolower(c);
        } else if (token.parametro == "-fs") {
            // Ignorar para ext2
        }
    }

    if (id.empty()) return "{\"error\": \"MKFS: El parámetro -id es obligatorio.\"}";

    ParticionMontada* particionEncontrada = nullptr;
    for (auto& pm : listaParticionesMontadas) {
        if (pm.id == id) {
            particionEncontrada = &pm;
            break;
        }
    }

    if (!particionEncontrada) {
        return "{\"error\": \"MKFS: No se encontró ninguna partición montada con el ID: " + id + "\"}";
    }

    std::fstream archivo(particionEncontrada->path, std::ios::in | std::ios::out | std::ios::binary);
    if (!archivo.is_open()) {
        return "{\"error\": \"MKFS: No se pudo abrir el archivo del disco.\"}";
    }

    MBR mbr;
    archivo.seekg(0, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));

    Partition partTarget;
    bool encontradaEnMbr = false;
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {
            std::string pName(mbr.mbr_partitions[i].part_name);
            if (pName == particionEncontrada->nombre) {
                partTarget = mbr.mbr_partitions[i];
                encontradaEnMbr = true;
                break;
            }
        }
    }

    if (!encontradaEnMbr) {
        archivo.close();
        return "{\"error\": \"MKFS: La partición montada ya no existe en el MBR del disco.\"}";
    }

    int n = (partTarget.part_size - sizeof(SuperBlock)) / (4 + sizeof(Inode) + 3 * 64);
    if (n <= 0) {
        archivo.close();
        return "{\"error\": \"MKFS: La partición es demasiado pequeña para formatear EXT2.\"}";
    }

    int n_inodos = n;
    int n_bloques = 3 * n;

    SuperBlock sb;
    sb.s_inodes_count = n_inodos;
    sb.s_blocks_count = n_bloques;
    sb.s_free_blocks_count = n_bloques - 2;
    sb.s_free_inodes_count = n_inodos - 2;
    sb.s_mtime = std::time(nullptr);
    sb.s_magic = 0xEF53;
    sb.s_inode_size = sizeof(Inode);
    sb.s_block_size = sizeof(FolderBlock);
    sb.s_first_ino = 2; // Inodo 0 y 1 ocupados
    sb.s_first_block = 2; // Bloque 0 y 1 ocupados

    sb.s_bm_inode_start = partTarget.part_start + sizeof(SuperBlock);
    sb.s_bm_block_start = sb.s_bm_inode_start + n_inodos;
    sb.s_inode_start = sb.s_bm_block_start + n_bloques;
    sb.s_block_start = sb.s_inode_start + (n_inodos * sizeof(Inode));

    archivo.seekp(partTarget.part_start, std::ios::beg);
    archivo.write(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    // Formatear bitmaps con ceros
    char cero = '0';
    archivo.seekp(sb.s_bm_inode_start, std::ios::beg);
    for (int i = 0; i < n_inodos; i++) archivo.write(&cero, 1);
    archivo.seekp(sb.s_bm_block_start, std::ios::beg);
    for (int i = 0; i < n_bloques; i++) archivo.write(&cero, 1);

    // Ocupar los primeros dos Inodos y Bloques (Raíz y Users.txt)
    char uno = '1';
    archivo.seekp(sb.s_bm_inode_start, std::ios::beg);
    archivo.write(&uno, 1); archivo.write(&uno, 1);
    archivo.seekp(sb.s_bm_block_start, std::ios::beg);
    archivo.write(&uno, 1); archivo.write(&uno, 1);

    // Crear Inodo 0 - Directorio Raíz
    Inode inodeRaiz;
    inodeRaiz.i_uid = 1; inodeRaiz.i_gid = 1;
    inodeRaiz.i_size = 0;
    inodeRaiz.i_ctime = std::time(nullptr); inodeRaiz.i_mtime = std::time(nullptr);
    inodeRaiz.i_type = '1'; // Carpeta
    inodeRaiz.i_perm = 664;
    inodeRaiz.i_block[0] = 0; // Apunta al Bloque 0

    // Bloque 0 - Directorio Raíz
    FolderBlock fbRaiz;
    std::strncpy(fbRaiz.b_content[0].b_name, ".", 11); fbRaiz.b_content[0].b_inodo = 0;
    std::strncpy(fbRaiz.b_content[1].b_name, "..", 11); fbRaiz.b_content[1].b_inodo = 0;
    std::strncpy(fbRaiz.b_content[2].b_name, "users.txt", 11); fbRaiz.b_content[2].b_inodo = 1;

    // Crear Inodo 1 - Archivo users.txt
    std::string usersStr = "1,G,root\n1,U,root,root,123\n";
    Inode inodeUsers;
    inodeUsers.i_uid = 1; inodeUsers.i_gid = 1;
    inodeUsers.i_size = usersStr.length();
    inodeUsers.i_ctime = std::time(nullptr); inodeUsers.i_mtime = std::time(nullptr);
    inodeUsers.i_type = '0'; // Archivo
    inodeUsers.i_perm = 664;
    inodeUsers.i_block[0] = 1; // Apunta al Bloque 1

    // Bloque 1 - Contenido de users.txt
    FileBlock fbUsers;
    std::strncpy(fbUsers.b_content, usersStr.c_str(), sizeof(fbUsers.b_content) - 1);

    // Escribir Inodos
    archivo.seekp(sb.s_inode_start, std::ios::beg);
    archivo.write(reinterpret_cast<char*>(&inodeRaiz), sizeof(Inode));
    archivo.write(reinterpret_cast<char*>(&inodeUsers), sizeof(Inode));

    // Escribir Bloques
    archivo.seekp(sb.s_block_start, std::ios::beg);
    archivo.write(reinterpret_cast<char*>(&fbRaiz), sizeof(FolderBlock));
    archivo.write(reinterpret_cast<char*>(&fbUsers), sizeof(FileBlock));

    archivo.close();

    std::cout << "-> Partición con ID " << id << " formateada exitosamente con EXT2." << std::endl;
    return "{\n  \"mensaje\": \"¡Partición formateada exitosamente con EXT2 (MKFS)!\",\n  \"id\": \"" + id + "\",\n  \"inodos\": " + std::to_string(n_inodos) + ",\n  \"bloques\": " + std::to_string(n_bloques) + "\n}";
}
