#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h> // Để dùng fcntl set non-blocking
#include <errno.h>

class ClientSocket {
private:
    int sock_fd;
    std::string server_ip;
    int server_port;
    bool connected;

public:
    ClientSocket() : sock_fd(-1), connected(false) {}

    ~ClientSocket() {
        Close();
    }

    // Kết nối đến Server
    bool Connect(const std::string& ip, int port) {
        this->server_ip = ip;
        this->server_port = port;

        // 1. Tạo Socket TCP
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            std::cerr << "[ERR] Failed to create socket\n";
            return false;
        }

        // 2. Cấu hình địa chỉ Server
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr)); // Xóa sạch rác bộ nhớ
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port); // Host to Network Short
        
        // Chuyển đổi IP từ string sang binary
        if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "[ERR] Invalid address/ Address not supported\n";
            return false;
        }

        // 3. Kết nối (Blocking connect - Chấp nhận chờ lúc đầu)
        if (::connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "[ERR] Connection Failed\n";
            return false;
        }

        // 4. QUAN TRỌNG: Chuyển Socket sang chế độ Non-blocking
        // Để khi gọi recv() trong vòng lặp game, nó không làm treo màn hình
        int flags = fcntl(sock_fd, F_GETFL, 0);
        fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

        connected = true;
        std::cout << "[INFO] Connected to " << ip << ":" << port << "\n";
        return true;
    }

    // Đóng kết nối
    void Close() {
        if (sock_fd >= 0) {
            close(sock_fd);
            sock_fd = -1;
            connected = false;
        }
    }

    // Gửi dữ liệu (Binary)
    // Trả về true nếu gửi thành công
    bool Send(const void* data, int len) {
        if (!connected || sock_fd < 0) return false;

        // Gửi toàn bộ dữ liệu (MSG_NOSIGNAL để tránh crash nếu server ngắt kết nối)
        int sent = send(sock_fd, data, len, MSG_NOSIGNAL);
        
        if (sent < 0) {
            std::cerr << "[ERR] Send failed: " << strerror(errno) << "\n";
            connected = false; // Coi như mất kết nối
            return false;
        }
        return true;
    }

    // Nhận dữ liệu (Non-blocking)
    // Trả về: 
    //  > 0: Số byte đọc được
    //  = 0: Không có dữ liệu (EWOULDBLOCK/EAGAIN) hoặc Server đóng (xử lý kỹ sau)
    //  -1: Lỗi
    int Receive(void* buffer, int max_len) {
        if (!connected || sock_fd < 0) return -1;

        // MSG_DONTWAIT: Nếu không có dữ liệu, trả về ngay lập tức chứ không chờ
        int bytes_read = recv(sock_fd, buffer, max_len, MSG_DONTWAIT);

        if (bytes_read > 0) {
            return bytes_read;
        } 
        else if (bytes_read == 0) {
            // Server đóng kết nối
            std::cout << "[INFO] Server closed connection\n";
            Close();
            return -1;
        } 
        else {
            // bytes_read < 0
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // Không có lỗi, chỉ là chưa có dữ liệu mới
                return 0; 
            } else {
                std::cerr << "[ERR] Recv failed: " << strerror(errno) << "\n";
                Close();
                return -1;
            }
        }
    }

    bool IsConnected() const { return connected; }
};