#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <map>
#include <string>

#pragma comment(lib, "ws2_32.lib")

struct GameSession {
    sockaddr_in player1Addr;
    sockaddr_in player2Addr;
    std::string player1Move;
    std::string player2Move;
    bool player1HasMoved = false;
    bool player2HasMoved = false;
    bool hasPlayer2 = false;
};

std::map<int, GameSession> sessions;
int sessionId = 0;

std::string determineWinner(const std::string& move1, const std::string& move2) {
    if (move1 == move2) return "Empate";

    if ((move1 == "Piedra" && (move2 == "Tijeras" || move2 == "Lagarto")) ||
        (move1 == "Papel" && (move2 == "Piedra" || move2 == "Spock")) ||
        (move1 == "Tijeras" && (move2 == "Papel" || move2 == "Lagarto")) ||
        (move1 == "Lagarto" && (move2 == "Papel" || move2 == "Spock")) ||
        (move1 == "Spock" && (move2 == "Piedra" || move2 == "Tijeras"))) {
        return "Gana Jugador 1";
    }

    return "Gana Jugador 2";
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    char buffer[1024];
    sockaddr_in clientAddr;
    int clientLen = sizeof(clientAddr);
    char clientIp[INET_ADDRSTRLEN];

    std::cout << "Servidor UDP iniciado en el puerto 8888..." << std::endl;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        recvfrom(serverSocket, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientLen);
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);

        std::string message(buffer);
        std::cout << "Mensaje recibido de " << clientIp << ": " << message << std::endl;

        if (message == "JOIN") {
            // Buscar una sesión disponible
            if (sessions.find(sessionId) == sessions.end() || sessions[sessionId].hasPlayer2) {
                sessionId++;
                sessions[sessionId] = {};
            }

            GameSession& game = sessions[sessionId];

            if (!game.player1HasMoved) {
                game.player1Addr = clientAddr;
                game.player1HasMoved = true;
                std::string response = "Conectado como Jugador 1. Esperando otro jugador...";
                sendto(serverSocket, response.c_str(), response.size(), 0, (sockaddr*)&clientAddr, clientLen);
            }
            else if (!game.hasPlayer2) {
                game.player2Addr = clientAddr;
                game.hasPlayer2 = true;
                std::string response = "Conectado como Jugador 2. ¡Partida lista! Envía tu jugada.";
                sendto(serverSocket, response.c_str(), response.size(), 0, (sockaddr*)&clientAddr, clientLen);
                std::cout << "Sesión " << sessionId << " lista para jugar." << std::endl;
            }
            continue;
        }

        // Buscar la sesión del jugador
        int currentSession = -1;
        for (auto& pair : sessions) {
            int id = pair.first;
            GameSession& session = pair.second;

            if (memcmp(&session.player1Addr, &clientAddr, sizeof(clientAddr)) == 0) {
                currentSession = id;
                session.player1Move = message;
                session.player1HasMoved = true;
                std::cout << "Jugador 1 de sesión " << id << " ha jugado: " << message << std::endl;
                break;
            }
            else if (session.hasPlayer2 && memcmp(&session.player2Addr, &clientAddr, sizeof(clientAddr)) == 0) {
                currentSession = id;
                session.player2Move = message;
                session.player2HasMoved = true;
                std::cout << "Jugador 2 de sesión " << id << " ha jugado: " << message << std::endl;
                break;
            }
        }

        if (currentSession == -1) {
            std::cerr << "Error: Jugador no asignado a ninguna sesión." << std::endl;
            continue;
        }

        GameSession& game = sessions[currentSession];

        // Asegurar que ambos jugadores han enviado su jugada antes de calcular el resultado
        if (game.player1HasMoved && game.player2HasMoved) {
            std::string result = determineWinner(game.player1Move, game.player2Move);

            sendto(serverSocket, result.c_str(), result.size(), 0, (sockaddr*)&game.player1Addr, clientLen);
            sendto(serverSocket, result.c_str(), result.size(), 0, (sockaddr*)&game.player2Addr, clientLen);
            std::cout << "Sesión " << currentSession << " finalizada. Resultado: " << result << std::endl;

            // Reiniciar la sesión
            game.player1HasMoved = false;
            game.player2HasMoved = false;
            game.player1Move.clear();
            game.player2Move.clear();
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
