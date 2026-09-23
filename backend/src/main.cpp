#include <iostream>
#include "httplib.h"
#include "analizador.h"
#include "estructuras.h"

int main() {
    std::srand(std::time(nullptr));

    httplib::Server svr;
    svr.Options("/(.*)", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    });

    svr.Post("/api/analizar", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        std::cout << "\n-> Recibido POST Comando: " << req.body << std::endl;
        res.set_content(analizarComando(req.body), "application/json");
    });

    std::cout << "\n============================================\n";
    std::cout << "¡Servidor C++ REFACTORIZADO ExtreamFS montado!" << std::endl;
    std::cout << "Escuchando en: http://0.0.0.0:8080" << std::endl;
    std::cout << "============================================\n\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}
