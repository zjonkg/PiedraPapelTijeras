#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    char move[1024];
    std::cout << "Ingrese su jugada (Piedra, Papel, Tijeras, Lagarto o Spock): ";
    std::cin >> move;

    // Validación básica de entrada (opcional)
    std::string validMoves[] = { "Piedra", "Papel", "Tijeras", "Lagarto", "Spock" };
    bool isValidMove = false;
    for (const auto& validMove : validMoves) {
        if (move == validMove) {
            isValidMove = true;
            break;
        }
    }

    if (!isValidMove) {
        std::cerr << "Jugada no válida. Por favor, ingrese una jugada correcta." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1; // Salir con error
    }

    sendto(clientSocket, move, strlen(move), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    char buffer[1024];
    int serverLen = sizeof(serverAddr);
    // Recibir respuesta del servidor
    int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&serverAddr, &serverLen);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';  // Asegurar que la cadena termina correctamente
    }

    std::cout << "Resultado: " << buffer << std::endl;

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
