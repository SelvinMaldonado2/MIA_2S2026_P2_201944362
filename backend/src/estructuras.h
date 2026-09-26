#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <string>
#include <vector>
#include <ctime>

// =========================================================================
// ESTRUCTURAS DEL SISTEMA DE ARCHIVOS EXT2
// =========================================================================
struct Partition {
    char part_status = '0'; // '0' inactiva, '1' activa
    char part_type = 'P';   // 'P' primaria, 'E' extendida, 'L' lógica
    char part_fit = 'W';    // 'W' Worst, 'F' First, 'B' Best
    int part_start = -1;    // Byte donde inicia
    int part_size = 0;      // Tamaño en bytes
    char part_name[16] = ""; // Nombre de la partición
};

struct MBR {
    int mbr_tamano = 0;
    time_t mbr_fecha_creacion;
    int mbr_dsk_signature = 0;
    char mbr_dsk_fit = 'F';
    Partition mbr_partitions[4]; // Arreglo de 4 particiones
};

struct SuperBlock {
    int s_filesystem_type = 2;       // EXT2 (2)
    int s_inodes_count = 0;          // Total de inodos
    int s_blocks_count = 0;          // Total de bloques
    int s_free_blocks_count = 0;     // Bloques libres
    int s_free_inodes_count = 0;     // Inodos libres
    time_t s_mtime = 0;              // Última fecha de montaje
    time_t s_umtime = 0;             // Última fecha de desmontaje
    int s_mnt_count = 0;             // Número de veces montada
    int s_magic = 0xEF53;            // Número mágico para identificar EXT2
    int s_inode_size = 128;          // Tamaño del inodo
    int s_block_size = 64;           // Tamaño del bloque
    int s_first_ino = 0;             // Primer inodo libre
    int s_first_block = 0;           // Primer bloque libre
    int s_bm_inode_start = 0;        // Inicio bitmap de inodos
    int s_bm_block_start = 0;        // Inicio bitmap de bloques
    int s_inode_start = 0;           // Inicio tabla de inodos
    int s_block_start = 0;           // Inicio tabla de bloques
};

struct Inode {
    int i_uid = 1;                   // UID del usuario dueño
    int i_gid = 1;                   // GID del grupo
    int i_size = 0;                  // Tamaño del archivo en bytes
    time_t i_atime = 0;              // Último acceso
    time_t i_ctime = 0;              // Creación
    time_t i_mtime = 0;              // Modificación
    int i_block[15] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // Punteros
    char i_type = '0';               // '0' archivo, '1' carpeta
    int i_perm = 664;                // Permisos
};

struct Content {
    char b_name[12] = "";
    int b_inodo = -1;
};

struct FolderBlock {
    Content b_content[4];
};

struct FileBlock {
    char b_content[64] = "";
};

struct PointerBlock {
    int b_pointers[16] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
};

struct Journaling {
    char j_operation[20] = ""; // Operacion (mkdir, mkfile, etc)
    char j_path[150] = "";     // Ruta afectada
    char j_content[100] = "";  // Contenido o detalle
    time_t j_date = 0;         // Fecha y hora
    char j_type = '0';         // '0' Carpeta, '1' Archivo
};

struct LoggedUser {
    bool activo = false;
    std::string id_particion = "";
    int id_user = -1;
    int id_group = -1;
    std::string username = "";
};

// Extern declarations to share global state among files
extern LoggedUser usuarioActual;

struct ParticionMontada {
    std::string id;
    std::string path;
    std::string nombre;
};

extern std::vector<ParticionMontada> listaParticionesMontadas;

#endif // ESTRUCTURAS_H
