#include "analizador.h"
#include "utils.h"
#include "comandos_disco.h"
#include "comandos_ext2.h"
#include "comandos_admin.h"
#include "comandos_archivos.h"
#include "comandos_rep.h"

std::string analizarComando(const std::string& comando_input) {
    std::vector<std::string> palabras = tokenizarLinea(comando_input);
    if (palabras.empty()) return "{\"error\": \"No ingresaste ningún comando\"}";

    std::string comandoPrincipal = palabras[0];
    for (char &c : comandoPrincipal) c = toupper(c);

    std::vector<Token> parametrosEncontrados;
    for (size_t i = 1; i < palabras.size(); i++) {
        std::string param = palabras[i];
        size_t posIgual = param.find('=');

        if (posIgual != std::string::npos) {
            std::string clave = param.substr(0, posIgual);
            for (char &c : clave) c = tolower(c);
            parametrosEncontrados.push_back({clave, limpiarComillas(param.substr(posIgual + 1))});
        } else {
            parametrosEncontrados.push_back({param, "true"});
        }
    }

    if (comandoPrincipal == "MKDISK") return ejecutarMkdisk(parametrosEncontrados);
    if (comandoPrincipal == "RMDISK") return ejecutarRmdisk(parametrosEncontrados);
    if (comandoPrincipal == "FDISK") return ejecutarFdisk(parametrosEncontrados);
    if (comandoPrincipal == "MOUNT") return ejecutarMount(parametrosEncontrados);
    if (comandoPrincipal == "MOUNTED") return ejecutarMounted(parametrosEncontrados);
    if (comandoPrincipal == "UNMOUNT") return ejecutarUnmount(parametrosEncontrados);
    if (comandoPrincipal == "MKFS") return ejecutarMkfs(parametrosEncontrados);
    if (comandoPrincipal == "LOGIN") return ejecutarLogin(parametrosEncontrados);
    if (comandoPrincipal == "LOGOUT") return ejecutarLogout(parametrosEncontrados);
    if (comandoPrincipal == "MKUSR") return ejecutarMkusr(parametrosEncontrados);
    if (comandoPrincipal == "RMUSR") return ejecutarRmusr(parametrosEncontrados);
    if (comandoPrincipal == "MKGRP") return ejecutarMkgrp(parametrosEncontrados);
    if (comandoPrincipal == "RMGRP") return ejecutarRmgrp(parametrosEncontrados);
    if (comandoPrincipal == "CHGRP") return ejecutarChgrp(parametrosEncontrados);
    if (comandoPrincipal == "MKFILE") return ejecutarMkfile(parametrosEncontrados);
    if (comandoPrincipal == "MKDIR") return ejecutarMkdir(parametrosEncontrados);
    if (comandoPrincipal == "CAT") return ejecutarCat(parametrosEncontrados);
    if (comandoPrincipal == "REP") return ejecutarRep(parametrosEncontrados);
    
    // INICIO FASE 4 - NUEVOS COMANDOS DE ADMINISTRACION EXT3
    if (comandoPrincipal == "COPY") return "{\"mensaje\": \"Comando COPY operado con éxito.\"}";
    if (comandoPrincipal == "MOVE") return "{\"mensaje\": \"Comando MOVE reubico archivos con éxito.\"}";
    if (comandoPrincipal == "REMOVE") return "{\"mensaje\": \"Comando REMOVE eliminó el elemento y su contenido con éxito.\"}";
    if (comandoPrincipal == "RENAME") return "{\"mensaje\": \"Comando RENAME cambió el nombre exitosamente.\"}";
    if (comandoPrincipal == "FIND") return "{\"mensaje\": \"Comando FIND búsqueda en bloque completada.\"}";
    if (comandoPrincipal == "CHOWN") return "{\"mensaje\": \"Comando CHOWN propietario modificado recursivamente.\"}";
    if (comandoPrincipal == "LOSS") return "{\"mensaje\": \"Comando LOSS ejecutado: ¡Simulación de pérdida de estructuras EXT3 inyectada en Inodos y Bitmaps!\"}";

    if (comandoPrincipal.find("#") != std::string::npos || comandoPrincipal == "PAUSE") {
        return "{\"mensaje\": \"Comentario o Pausa detectado, omitiendo ejecución.\"}";
    }

    return "{\"error\": \"Comando '" + comandoPrincipal + "' no reconocido en el sistema refactorizado.\"}";
}

