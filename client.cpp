#include <iostream>
#include <cstdlib>
#include <atomic>
#include <ctime>
#include <thread>
#include <vector>
#include <chrono>

#define SUCCESS 0
#define INVALID_SOCKET -1
#include <sys/socket.h> // for creating socket, for binding address, for acceptance, for connecting to the server
#include <arpa/inet.h> // convertation from string to struct
#include <unistd.h> // system calls: close socket, send data, receive data.
const char *clear = "clear";

class App {
    private:
        int sock_m = 0;
        std::atomic <bool> allowed_to_send = false;
        sockaddr_in Server_addr {};
    public:
        ~App() {
            allowed_to_send = false;
            close(sock_m);
        }
        void send_to_server(const std::string &message) {
            if (allowed_to_send) {
                if (send(sock_m, message.c_str(), message.length(), 0) == INVALID_SOCKET) {
                    std::cerr << "\nError: failed to send a message\n";
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            else {
                std::cerr << "\nError: failed to connect to the server\n";
                close(sock_m);
                std::exit(-1);
            }
        }
        void init (const std::string &ip, const int &port) {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) {
                return;
            }

            Server_addr.sin_family = AF_INET;
            Server_addr.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &Server_addr.sin_addr);
            sock_m = sock;
            if (connect(sock, (sockaddr*)&Server_addr, sizeof(Server_addr)) == SUCCESS)  {
                allowed_to_send = true;
            }
            else {
                allowed_to_send = false;
            }
        }
        void get_msg() {
            char message[1024];
            while (allowed_to_send) {
                int bytes_received = recv(sock_m, message, sizeof(message)-1, 0);
                if (bytes_received > 0) {
                    message[bytes_received] = '\0';
                    std::cout << message << std::endl;
                    std::cout.flush();
                    break;
                }
                else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }
}instance;

bool is_only_space(std::string str) {
    bool res = true;
    for (char x : str) {
        if (std::isalnum(x)) {
            res = false;
        }
    }
    return res;
}

int main() {
    std::string input = "";
    std::string ip = "";
    int port = 0;
    std::cout << "Enter IP of the server: ";
    std::cin >> ip;
    std::cout << "Enter Port of the server: ";
    std::cin >> port;
    if (!std::cin || port <= 0) {
        std::cerr << "Error: Invalid input" << std::endl;
        return -1;
    }
    std::cin.ignore(); // remove '\n' character from buffer 
    instance.init(ip, port);
    system(clear);
    while (true) {
        std::getline(std::cin, input);
        if (input == "q" || input.empty() || is_only_space(input)) {
            break;
        }
        instance.send_to_server(input);
        instance.get_msg(); 
        std::cout.flush();
    }
    return 0;
}
