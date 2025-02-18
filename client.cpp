#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    // Configurar timeout para recvfrom()
    int timeout = 5000; // 5 segundos
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    char buffer[1024];
    int serverLen = sizeof(serverAddr);

    // Enviar mensaje de conexión
    std::string joinMessage = "JOIN";
    sendto(clientSocket, joinMessage.c_str(), joinMessage.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    std::cout << "Esperando respuesta del servidor..." << std::endl;

    // Intentar recibir confirmación del servidor
    int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&serverAddr, &serverLen);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Error: No se recibió respuesta del servidor. Verifica que el servidor esté corriendo." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    buffer[bytesReceived] = '\0';
    std::cout << "Servidor: " << buffer << std::endl;

    while (true) { // Bucle para jugar varias rondas
        std::string move;
        std::cout << "Ingrese su jugada (Piedra, Papel, Tijeras, Lagarto o Spock, o 'salir' para salir): ";
        std::getline(std::cin, move);

        // Permitir salir del juego
        if (move == "salir") {
            std::cout << "Saliendo del juego..." << std::endl;
            break;
        }

        // Validación de entrada
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
            continue; // Volver a pedir la jugada
        }

        // Enviar jugada al servidor
        sendto(clientSocket, move.c_str(), move.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

        // Recibir resultado
        bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&serverAddr, &serverLen);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error: No se recibió respuesta del servidor después de enviar la jugada." << std::endl;
            continue; // Volver a intentarlo en la siguiente iteración
        }

        buffer[bytesReceived] = '\0';
        std::cout << "Resultado: " << buffer << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
