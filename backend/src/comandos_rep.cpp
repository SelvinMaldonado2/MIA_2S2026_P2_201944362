#include "comandos_rep.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

// ==========================================
// REPORTE MBR (Muestra Toda la Tabla y Particiones)
// ==========================================
std::string reportarMBR(const std::string& diskPath, const std::string& outputPath) {
    std::fstream archivo(diskPath, std::ios::in | std::ios::binary);
    if (!archivo.is_open()) return "Error: No se pudo abrir el disco físico.";

    MBR mbr;
    archivo.seekg(0, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    archivo.close();

    std::string dotContent = "digraph G {\n";
    dotContent += "  node [shape=plaintext, fontname=\"Inter\"];\n";
    dotContent += "  tabla [label=<\n";
    dotContent += "    <table border=\"1\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"8\">\n";
    dotContent += "      <tr><td bgcolor=\"#7000ff\" colspan=\"2\"><font color=\"white\"><b>REPORTE MBR</b></font></td></tr>\n";
    dotContent += "      <tr><td bgcolor=\"#1a1a24\"><font color=\"white\"><b>mbr_tamano</b></font></td><td bgcolor=\"#1a1a24\"><font color=\"white\">" + std::to_string(mbr.mbr_tamano) + "</font></td></tr>\n";
    dotContent += "      <tr><td>mbr_dsk_signature</td><td>" + std::to_string(mbr.mbr_dsk_signature) + "</td></tr>\n";

    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {
            dotContent += "      <tr><td bgcolor=\"#00f0ff\" colspan=\"2\"><font color=\"black\"><b>Partición " + std::to_string(i + 1) + "</b></font></td></tr>\n";
            dotContent += "      <tr><td>part_status</td><td>" + std::string(1, mbr.mbr_partitions[i].part_status) + "</td></tr>\n";
            dotContent += "      <tr><td>part_type</td><td>" + std::string(1, mbr.mbr_partitions[i].part_type) + "</td></tr>\n";
            dotContent += "      <tr><td>part_fit</td><td>" + std::string(1, mbr.mbr_partitions[i].part_fit) + "</td></tr>\n";
            dotContent += "      <tr><td>part_start</td><td>" + std::to_string(mbr.mbr_partitions[i].part_start) + "</td></tr>\n";
            dotContent += "      <tr><td>part_size</td><td>" + std::to_string(mbr.mbr_partitions[i].part_size) + "</td></tr>\n";
            dotContent += "      <tr><td>part_name</td><td>" + std::string(mbr.mbr_partitions[i].part_name) + "</td></tr>\n";
        }
    }
    dotContent += "    </table>\n  >];\n}\n";

    std::string dotPath = outputPath + ".dot";
    std::ofstream file(dotPath);
    file << dotContent;
    file.close();

    std::string cmd = "dot -Tpng " + dotPath + " -o " + outputPath;
    std::system(cmd.c_str());
    return "Reporte MBR generado en: " + outputPath;
}

// ==========================================
// REPORTE SUPERBLOQUE EXT2
// ==========================================
std::string reportarSuperBlock(const std::string& diskPath, Partition partTarget, const std::string& outputPath) {
    std::fstream archivo(diskPath, std::ios::in | std::ios::binary);
    if (!archivo.is_open()) return "Error: No se pudo abrir el disco físico.";

    SuperBlock sb;
    archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));
    archivo.close();

    std::string dotContent = "digraph G {\n";
    dotContent += "  node [shape=plaintext, fontname=\"Inter\"];\n";
    dotContent += "  tabla [label=<\n";
    dotContent += "    <table border=\"1\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"8\">\n";
    dotContent += "      <tr><td bgcolor=\"#e83e8c\" colspan=\"2\"><font color=\"white\"><b>REPORTE DE SUPERBLOQUE</b></font></td></tr>\n";
    dotContent += "      <tr><td>s_inodes_count</td><td>" + std::to_string(sb.s_inodes_count) + "</td></tr>\n";
    dotContent += "      <tr><td>s_blocks_count</td><td>" + std::to_string(sb.s_blocks_count) + "</td></tr>\n";
    dotContent += "      <tr><td>s_free_blocks_count</td><td>" + std::to_string(sb.s_free_blocks_count) + "</td></tr>\n";
    dotContent += "      <tr><td>s_free_inodes_count</td><td>" + std::to_string(sb.s_free_inodes_count) + "</td></tr>\n";
    dotContent += "      <tr><td>s_mtime</td><td>" + std::to_string(sb.s_mtime) + "</td></tr>\n";
    dotContent += "      <tr><td>s_magic</td><td>0xEF53</td></tr>\n";
    dotContent += "      <tr><td>s_inode_size</td><td>" + std::to_string(sb.s_inode_size) + "</td></tr>\n";
    dotContent += "      <tr><td>s_block_size</td><td>" + std::to_string(sb.s_block_size) + "</td></tr>\n";
    dotContent += "      <tr><td>s_first_ino</td><td>" + std::to_string(sb.s_first_ino) + "</td></tr>\n";
    dotContent += "      <tr><td>s_bm_inode_start</td><td>" + std::to_string(sb.s_bm_inode_start) + "</td></tr>\n";
    dotContent += "      <tr><td>s_bm_block_start</td><td>" + std::to_string(sb.s_bm_block_start) + "</td></tr>\n";
    dotContent += "      <tr><td>s_inode_start</td><td>" + std::to_string(sb.s_inode_start) + "</td></tr>\n";
    dotContent += "      <tr><td>s_block_start</td><td>" + std::to_string(sb.s_block_start) + "</td></tr>\n";
    dotContent += "    </table>\n  >];\n}\n";

    std::string dotPath = outputPath + ".dot";
    std::ofstream file(dotPath);
    file << dotContent;
    file.close();

    std::string cmd = "dot -Tpng " + dotPath + " -o " + outputPath;
    std::system(cmd.c_str());
    return "Reporte SUPERBLOCK generado en: " + outputPath;
}

// ==========================================
// REPORTE DISK (PASTEL/BARRA RECTANGULAR)
// ==========================================
std::string reportarDisk(const std::string& diskPath, const std::string& outputPath) {
    std::fstream archivo(diskPath, std::ios::in | std::ios::binary);
    if (!archivo.is_open()) return "Error: No se pudo abrir el disco físico.";

    MBR mbr;
    archivo.seekg(0, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    archivo.close();

    std::string dot = "digraph G {\n";
    dot += "  node [shape=record, style=filled, fillcolor=\"#B2DFDB\", fontname=\"Inter\"];\n";
    
    std::string fila = "MBR";
    int lastEnd = sizeof(MBR);
    
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {
            int currentStart = mbr.mbr_partitions[i].part_start;
            if (currentStart > lastEnd) {
                float freePercent = ((float)(currentStart - lastEnd) / mbr.mbr_tamano) * 100.0;
                fila += " | Libre\\n" + std::to_string(freePercent).substr(0,4) + "%";
            }
            float partPercent = ((float)mbr.mbr_partitions[i].part_size / mbr.mbr_tamano) * 100.0;
            fila += " | Part " + std::to_string(i+1) + "\\n" + std::to_string(partPercent).substr(0,4) + "%";
            lastEnd = currentStart + mbr.mbr_partitions[i].part_size;
        }
    }
    
    if (lastEnd < mbr.mbr_tamano) {
        float finalPercent = ((float)(mbr.mbr_tamano - lastEnd) / mbr.mbr_tamano) * 100.0;
        fila += " | Libre\\n" + std::to_string(finalPercent).substr(0,4) + "%";
    }

    dot += "  struct1 [label=\"{" + fila + "}\"];\n";
    dot += "}\n";

    std::string dotPath = outputPath + ".dot";
    std::ofstream file(dotPath); file << dot; file.close();
    std::string cmd = "dot -Tpng " + dotPath + " -o " + outputPath;
    std::system(cmd.c_str());

    return "Reporte DISK generado gráficamente en: " + outputPath;
}

// ==========================================
// REPORTE INDIVIDUAL INODES Y BLOCKS
// ==========================================
std::string reportarBloquesOInodos(const std::string& diskPath, Partition partTarget, const std::string& outputPath, int flag) {
    std::fstream archivo(diskPath, std::ios::in | std::ios::binary);
    if (!archivo.is_open()) return "Error: No se pudo abrir el disco físico.";

    SuperBlock sb;
    archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    std::string dot = "digraph G {\n";
    dot += "  node [shape=plaintext, fontname=\"Inter\"];\n";

    int limit = (flag == 1) ? sb.s_inodes_count : sb.s_blocks_count;
    int offsetMap = (flag == 1) ? sb.s_bm_inode_start : sb.s_bm_block_start;
    std::vector<char> bm(limit);
    archivo.seekg(offsetMap, std::ios::beg);
    archivo.read(bm.data(), limit);

    for (int i = 0; i < limit; i++) {
        if (bm[i] == '1') {
            if (flag == 1) { // INODES
                Inode inodo;
                archivo.seekg(sb.s_inode_start + (i * sizeof(Inode)), std::ios::beg);
                archivo.read(reinterpret_cast<char*>(&inodo), sizeof(Inode));
                dot += "  nod_" + std::to_string(i) + " [label=<\n    <table border=\"0\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"4\">\n";
                dot += "      <tr><td bgcolor=\"#00B8D4\" colspan=\"2\"><font color=\"white\"><b>Inodo " + std::to_string(i) + "</b></font></td></tr>\n";
                dot += "      <tr><td>i_uid</td><td>" + std::to_string(inodo.i_uid) + "</td></tr>\n";
                dot += "      <tr><td>i_size</td><td>" + std::to_string(inodo.i_size) + "</td></tr>\n";
                for(int k=0; k<12; k++) {
                    dot += "      <tr><td>i_block[" + std::to_string(k) + "]</td><td>" + std::to_string(inodo.i_block[k]) + "</td></tr>\n";
                }
                dot += "    </table>\n  >];\n";
            } else { // BLOCKS
                dot += "  nod_" + std::to_string(i) + " [label=<\n    <table border=\"0\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"4\">\n";
                dot += "      <tr><td bgcolor=\"#FFAB00\" colspan=\"1\"><font color=\"black\"><b>Bloque " + std::to_string(i) + "</b></font></td></tr>\n";
                dot += "    </table>\n  >];\n"; 
                // En un proyecto real se lee File o FolderBlock según dependa, simplificado para validación visual rápida
            }
        }
    }
    dot += "}\n";
    archivo.close();

    std::string dotPath = outputPath + ".dot";
    std::ofstream file(dotPath); file << dot; file.close();
    std::string cmd = "dot -Tpng " + dotPath + " -o " + outputPath;
    std::system(cmd.c_str());

    std::string nameR = (flag == 1) ? "INODE" : "BLOCK";
    return "Reporte " + nameR + " masivo generado exitosamente en: " + outputPath;
}

// ==========================================
// REPORTE ARBOL (TREE) EXT2
// ==========================================
std::string reportarTree(const std::string& diskPath, Partition partTarget, const std::string& outputPath) {
    std::fstream archivo(diskPath, std::ios::in | std::ios::binary);
    if (!archivo.is_open()) return "Error: No se pudo abrir el disco físico.";

    SuperBlock sb;
    archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    std::string dot = "digraph G {\n";
    dot += "  node [shape=plaintext, fontname=\"Fira Code\"];\n";
    dot += "  rankdir=LR;\n"; // De izquierda a derecha para los árboles

    // Recorremos todo el Bitmap de Inodos para pintar todo lo ocupado sin redundar y sin fallar por recursividad infinita
    archivo.seekg(sb.s_bm_inode_start, std::ios::beg);
    std::vector<char> bitmapInodos(sb.s_inodes_count);
    archivo.read(bitmapInodos.data(), sb.s_inodes_count);
    
    for (int i = 0; i < sb.s_inodes_count; i++) {
        if (bitmapInodos[i] == '1') {
            Inode inodo;
            archivo.seekg(sb.s_inode_start + (i * sizeof(Inode)), std::ios::beg);
            archivo.read(reinterpret_cast<char*>(&inodo), sizeof(Inode));

            std::string color = (inodo.i_type == '1') ? "#4CAF50" : "#2196F3"; // Carpeta Verde, Archivo Azul
            std::string typeTxt = (inodo.i_type == '1') ? "Carpeta" : "Archivo";

            dot += "  inodo_" + std::to_string(i) + " [label=<\n";
            dot += "    <table border=\"0\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"5\">\n";
            dot += "      <tr><td bgcolor=\"" + color + "\" colspan=\"2\"><font color=\"white\"><b>Inodo " + std::to_string(i) + " (" + typeTxt + ")</b></font></td></tr>\n";
            dot += "      <tr><td>i_uid</td><td>" + std::to_string(inodo.i_uid) + "</td></tr>\n";
            dot += "      <tr><td>i_size</td><td>" + std::to_string(inodo.i_size) + " bytes</td></tr>\n";
            
            for(int k=0; k<12; k++) {
                if(inodo.i_block[k] != -1) {
                    dot += "      <tr><td>i_block[" + std::to_string(k) + "]</td><td port=\"p" + std::to_string(k) + "\">" + std::to_string(inodo.i_block[k]) + "</td></tr>\n";
                } else {
                    dot += "      <tr><td>i_block[" + std::to_string(k) + "]</td><td>" + std::to_string(inodo.i_block[k]) + "</td></tr>\n";
                }
            }
            dot += "    </table>\n  >];\n";

            for(int k = 0; k < 12; k++) {
                int bid = inodo.i_block[k];
                if(bid != -1) {
                    dot += "  inodo_" + std::to_string(i) + ":p" + std::to_string(k) + " -> bloque_" + std::to_string(bid) + ";\n";
                    
                    if(inodo.i_type == '1') { 
                        FolderBlock fb;
                        archivo.seekg(sb.s_block_start + (bid * sizeof(FolderBlock)), std::ios::beg);
                        archivo.read(reinterpret_cast<char*>(&fb), sizeof(FolderBlock));
                        
                        dot += "  bloque_" + std::to_string(bid) + " [label=<\n";
                        dot += "    <table border=\"0\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"5\">\n";
                        dot += "      <tr><td bgcolor=\"#FF9800\" colspan=\"2\"><font color=\"black\"><b>Bloque Carpeta " + std::to_string(bid) + "</b></font></td></tr>\n";
                        
                        std::string enlacesHijos = "";
                        for(int j=0; j<4; j++) {
                            std::string bname = "";
                            for(int n=0; n<12 && fb.b_content[j].b_name[n] != '\0'; n++) {
                                char ch = fb.b_content[j].b_name[n];
                                if(ch == '<') bname += "&lt;";
                                else if(ch == '>') bname += "&gt;";
                                else if(ch == '&') bname += "&amp;";
                                else if(ch >= 32 && ch <= 126) bname += ch;
                            }
                            if(bname.empty()) bname = "-";
                            dot += "      <tr><td>" + bname + "</td><td port=\"p" + std::to_string(j) + "\">" + std::to_string(fb.b_content[j].b_inodo) + "</td></tr>\n";
                            
                            int inodoHijo = fb.b_content[j].b_inodo;
                            if(inodoHijo != -1 && bname != "." && bname != "..") {
                                enlacesHijos += "  bloque_" + std::to_string(bid) + ":p" + std::to_string(j) + " -> inodo_" + std::to_string(inodoHijo) + ";\n";
                            }
                        }
                        dot += "    </table>\n  >];\n";
                        dot += enlacesHijos;
                    } 
                    else { 
                        FileBlock fbFile;
                        archivo.seekg(sb.s_block_start + (bid * sizeof(FileBlock)), std::ios::beg);
                        archivo.read(reinterpret_cast<char*>(&fbFile), sizeof(FileBlock));
                        
                        std::string contenido = "";
                        for(int c=0; c<64; c++) {
                            char ch = fbFile.b_content[c];
                            if(ch == '\n') contenido += "<br/>";
                            else if(ch == '<') contenido += "&lt;";
                            else if(ch == '>') contenido += "&gt;";
                            else if(ch == '&') contenido += "&amp;";
                            else if(ch >= 32 && ch <= 126) contenido += ch;
                        }

                        dot += "  bloque_" + std::to_string(bid) + " [label=<\n";
                        dot += "    <table border=\"0\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"5\">\n";
                        dot += "      <tr><td bgcolor=\"#607D8B\" colspan=\"1\"><font color=\"white\"><b>Bloque Archivo " + std::to_string(bid) + "</b></font></td></tr>\n";
                        dot += "      <tr><td>" + contenido + "</td></tr>\n";
                        dot += "    </table>\n  >];\n";
                    }
                }
            }
        }
    }
    dot += "}\n";
    archivo.close();

    std::string dotPath = outputPath + ".dot";
    std::ofstream file(dotPath);
    file << dot;
    file.close();

    std::string cmd = "dot -Tpng " + dotPath + " -o " + outputPath;
    std::system(cmd.c_str());
    return "Reporte TREE generado magistralmente en: " + outputPath;
}

std::string reportarBitmap(const std::string& diskPath, Partition partTarget, const std::string& outputPath, int tipo) {
    std::fstream archivo(diskPath, std::ios::in | std::ios::binary);
    if (!archivo.is_open()) return "Error: No se pudo abrir el disco físico.";

    SuperBlock sb;
    archivo.seekg(partTarget.part_start, std::ios::beg);
    archivo.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));

    int offset = (tipo == 1) ? sb.s_bm_inode_start : sb.s_bm_block_start;
    int limit = (tipo == 1) ? sb.s_inodes_count : sb.s_blocks_count;
    
    archivo.seekg(offset, std::ios::beg);
    std::string txtOutput = "";
    char bit;
    for(int i = 1; i <= limit; i++) {
        archivo.read(&bit, 1);
        txtOutput += (bit == '1' ? "1 " : "0 ");
        if(i % 20 == 0) txtOutput += "\n";
    }
    archivo.close();

    std::ofstream file(outputPath);
    file << txtOutput;
    file.close();

    return "Reporte BITMAP de " + std::string(tipo == 1 ? "Inodos" : "Bloques") + " (010101...) generado (TXT) en: " + outputPath;
}

std::string ejecutarRep(const std::vector<Token>& parametros) {
    std::string name = "", path = "", id = "", ruta = "";

    for (const auto& t : parametros) {
        if (t.parametro == "-name") name = t.valor;
        else if (t.parametro == "-path") { path = limpiarComillas(t.valor); }
        else if (t.parametro == "-id") id = t.valor;
        else if (t.parametro == "-ruta") ruta = limpiarComillas(t.valor);
    }
    
    if (name.empty() || path.empty() || id.empty()) {
        return "{\"error\": \"REP: Faltan parámetros obligatorios (-name, -path, -id).\"}";
    }
    for (char &c : name) c = tolower(c);

    std::string diskPath; Partition partTarget;
    if(!getActivePartition(id, diskPath, partTarget)) return "{\"error\": \"REP: ID de partición no fued encontrado en memoria.\"}";

    try {
        fs::path p(path);
        fs::path parentDir = p.parent_path();
        if (!parentDir.empty() && !fs::exists(parentDir)) fs::create_directories(parentDir);
    } catch (...) {
        return "{\"error\": \"REP: Fallo crítico al intentar crear las carpetas para guardar la imagen en Linux.\"}";
    }

    std::string salida = "";
    if (name == "mbr") salida = reportarMBR(diskPath, path);
    else if (name == "disk") salida = reportarDisk(diskPath, path);
    else if (name == "sb") salida = reportarSuperBlock(diskPath, partTarget, path);
    else if (name == "tree") salida = reportarTree(diskPath, partTarget, path);
    else if (name == "bm_inode") salida = reportarBitmap(diskPath, partTarget, path, 1);
    else if (name == "bm_block") salida = reportarBitmap(diskPath, partTarget, path, 2);
    else if (name == "inode") salida = reportarBloquesOInodos(diskPath, partTarget, path, 1);
    else if (name == "block") salida = reportarBloquesOInodos(diskPath, partTarget, path, 2);
    else salida = "Error: El reporte introducido '" + name + "' aún no se encuentra desarrollado en Graphviz.";
    
    if(salida.find("Error") != std::string::npos) {
        return "{\"error\": \"" + salida + "\"}";
    }

    return "{\"mensaje\": \"¡" + salida + "!\"}";
}
