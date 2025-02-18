#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <map>
#include <string>
#pragma comment(lib, "ws2_32.lib")

struct GameSession {
    std::string player1Move;
    std::string player2Move;
    sockaddr_in player1Addr;
    sockaddr_in player2Addr;
    bool player1Ready = false;
    bool player2Ready = false;
};

std::map<int, GameSession> sessions;
int sessionId = 0;

std::string determineWinner(const std::string& move1, const std::string& move2) {
    if (move1 == move2) return "Empate";

    // Definir las reglas del juego
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

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        recvfrom(serverSocket, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientLen);
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);

        std::string move(buffer);
        std::cout << "Jugador conectado desde " << clientIp << std::endl;
        std::cout << "Jugador hizo jugada: " << move << std::endl;

        if (sessions[sessionId].player1Ready && sessions[sessionId].player2Ready) {
            sessionId++;
        }

        GameSession& game = sessions[sessionId];
        if (!game.player1Ready) {
            game.player1Move = move;
            game.player1Addr = clientAddr;
            game.player1Ready = true;
            std::cout << "Jugador 1 listo con jugada: " << move << std::endl;
        } else {
            game.player2Move = move;
            game.player2Addr = clientAddr;
            game.player2Ready = true;
            std::cout << "Jugador 2 listo con jugada: " << move << std::endl;
        }

        if (game.player1Ready && game.player2Ready) {
            std::string result = determineWinner(game.player1Move, game.player2Move);
            sendto(serverSocket, result.c_str(), result.size(), 0, (sockaddr*)&game.player1Addr, clientLen);
            sendto(serverSocket, result.c_str(), result.size(), 0, (sockaddr*)&game.player2Addr, clientLen);
            std::cout << "Partida finalizada. Resultado: " << result << std::endl;

            game.player1Ready = false;
            game.player2Ready = false;
            sessions.erase(sessionId);
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
