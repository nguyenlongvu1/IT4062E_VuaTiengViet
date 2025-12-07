#include "ClientSocket.h"
#include <iostream>
#include <string>
#include <limits>

class VuaTiengVietClient {
private:
    ClientSocket socket;
    std::string current_token;
    std::string current_user_id;
    std::string current_username;

public:
    VuaTiengVietClient() : socket("127.0.0.1", 8080) {}

    void run() {
        if (!socket.connect()) {
            std::cerr << "Could not connect to server\n";
            return;
        }

        std::cout << "\n=== VuaTiengViet - Game Client ===\n";

        while (true) {
            if (current_token.empty()) {
                show_auth_menu();
            } else {
                show_main_menu();
            }
        }
    }

private:
    void show_auth_menu() {
        std::cout << "\n--- Authentication Menu ---\n";
        std::cout << "1. Login\n";
        std::cout << "2. Register\n";
        std::cout << "3. Exit\n";
        std::cout << "Choose option (1-3): ";

        int choice = 0;
        while (!(std::cin >> choice) || choice < 1 || choice > 3) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid option. Try again (1-3): ";
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1:
                handle_login();
                break;
            case 2:
                handle_register();
                break;
            case 3:
                std::cout << "Goodbye!\n";
                socket.close_connection();
                exit(0);
                break;
        }
    }

    void show_main_menu() {
        std::cout << "\n--- Main Menu ---\n";
        std::cout << "Logged in as: " << current_username << "\n";
        std::cout << "1. Add Friend\n";
        std::cout << "2. View Friends\n";
        std::cout << "3. Friend Requests\n";
        std::cout << "4. Reset Password\n";
        std::cout << "5. Logout\n";
        std::cout << "0. Back\n";
        std::cout << "Choose option (0-5): ";

        int choice = 0;
        while (!(std::cin >> choice) || choice < 0 || choice > 5) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid option. Try again (0-5): ";
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 0:
                break; // Loop back to main menu
            case 1:
                handle_add_friend();
                break;
            case 2:
                handle_view_friends();
                break;
            case 3:
                handle_friend_requests();
                break;
            case 4:
                handle_reset_password();
                break;
            case 5:
                handle_logout();
                break;
        }
    }

    void handle_login() {
        std::cout << "\n--- Login ---\n";
        std::cout << "Username: ";
        std::string username;
        std::getline(std::cin, username);

        std::cout << "Password: ";
        std::string password;
        std::getline(std::cin, password);

        Message msg;
        msg.command = "LOGIN";
        msg.params["username"] = username;
        msg.params["password"] = password;

        if (!socket.send_message(msg)) {
            std::cout << "Failed to send login request\n";
            return;
        }

        Message response = socket.receive_message();
        if (response.command == "LOGIN_OK") {
            current_token = response.params["token"];
            current_user_id = response.params["user_id"];
            current_username = username;
            std::cout << "\n✓ Login successful!\n";
        } else if (response.command == "ERROR") {
            std::cout << "\n✗ Login failed: " << response.params["error_msg"] << "\n";
        } else {
            std::cout << "\n✗ Unexpected response: " << response.command << "\n";
        }
    }

    void handle_register() {
        std::cout << "\n--- Register ---\n";
        std::cout << "Username: ";
        std::string username;
        std::getline(std::cin, username);

        std::cout << "Password: ";
        std::string password;
        std::getline(std::cin, password);

        std::cout << "Confirm Password: ";
        std::string confirm_password;
        std::getline(std::cin, confirm_password);

        if (password != confirm_password) {
            std::cout << "\n✗ Passwords do not match!\n";
            return;
        }

        Message msg;
        msg.command = "REGISTER";
        msg.params["username"] = username;
        msg.params["password"] = password;

        if (!socket.send_message(msg)) {
            std::cout << "Failed to send register request\n";
            return;
        }

        Message response = socket.receive_message();
        if (response.command == "REGISTER_OK") {
            std::cout << "\n✓ Registration successful! You can now login.\n";
        } else if (response.command == "ERROR") {
            std::cout << "\n✗ Registration failed: " << response.params["error_msg"] << "\n";
        } else {
            std::cout << "\n✗ Unexpected response: " << response.command << "\n";
        }
    }

    void handle_reset_password() {
        std::cout << "\n--- Reset Password ---\n";
        std::cout << "Old Password: ";
        std::string old_password;
        std::getline(std::cin, old_password);

        std::cout << "New Password: ";
        std::string new_password;
        std::getline(std::cin, new_password);

        std::cout << "Confirm New Password: ";
        std::string confirm_password;
        std::getline(std::cin, confirm_password);

        if (new_password != confirm_password) {
            std::cout << "\n✗ Passwords do not match!\n";
            return;
        }

        Message msg;
        msg.command = "RESET_PASSWORD";
        msg.params["token"] = current_token;
        msg.params["old_password"] = old_password;
        msg.params["new_password"] = new_password;

        if (!socket.send_message(msg)) {
            std::cout << "Failed to send reset password request\n";
            return;
        }

        Message response = socket.receive_message();
        if (response.command == "RESET_PASSWORD_OK") {
            std::cout << "\n✓ Password reset successful!\n";
        } else if (response.command == "ERROR") {
            std::cout << "\n✗ Password reset failed: " << response.params["error_msg"] << "\n";
        } else {
            std::cout << "\n✗ Unexpected response: " << response.command << "\n";
        }
    }

    void handle_logout() {
        std::cout << "\n--- Logging out ---\n";

        Message msg;
        msg.command = "LOGOUT";
        msg.params["token"] = current_token;

        if (!socket.send_message(msg)) {
            std::cout << "Failed to send logout request\n";
            return;
        }

        Message response = socket.receive_message();
        if (response.command == "LOGOUT_OK") {
            current_token = "";
            current_user_id = "";
            current_username = "";
            std::cout << "\n✓ Logout successful!\n";
        } else if (response.command == "ERROR") {
            std::cout << "\n✗ Logout failed: " << response.params["error_msg"] << "\n";
        } else {
            std::cout << "\n✗ Unexpected response: " << response.command << "\n";
        }
    }

    void handle_add_friend() {
        std::cout << "\n--- Add Friend ---\n";
        std::cout << "Enter 0 to go back\n";

        while (true) {
            std::cout << "Friend's username: ";
            std::string target_username;
            std::getline(std::cin, target_username);

            if (target_username == "0") {
                break;
            }

            if (target_username.empty()) {
                std::cout << "✗ Username cannot be empty\n";
                continue;
            }

            Message msg;
            msg.command = "SEND_FRIEND_REQUEST";
            msg.params["token"] = current_token;
            msg.params["target_username"] = target_username;

            if (!socket.send_message(msg)) {
                std::cout << "✗ Failed to send friend request\n";
                continue;
            }

            Message response = socket.receive_message();
            if (response.command == "FRIEND_REQUEST_SENT") {
                std::cout << "\n✓ Friend request sent to " << target_username << "!\n";
            } else if (response.command == "ERROR") {
                std::cout << "\n✗ Failed: " << response.params["error_msg"] << "\n";
            } else {
                std::cout << "\n✗ Unexpected response: " << response.command << "\n";
            }
        }
    }

    void handle_view_friends() {
        std::cout << "\n--- Your Friends ---\n";

        Message msg;
        msg.command = "LIST_FRIENDS";
        msg.params["token"] = current_token;

        if (!socket.send_message(msg)) {
            std::cout << "✗ Failed to fetch friends list\n";
            return;
        }

        Message response = socket.receive_message();
        if (response.command == "FRIENDS_LIST") {
            int friend_count = std::stoi(response.params["friend_count"]);
            if (friend_count == 0) {
                std::cout << "\nYou have no friends yet.\n";
            } else {
                std::cout << "\nYou have " << friend_count << " friend(s):\n";
                std::string friends_str = response.params["friends"];
                if (!friends_str.empty()) {
                    // Parse and display friends separated by |
                    size_t pos = 0;
                    int idx = 1;
                    while (pos < friends_str.length()) {
                        size_t pipe_pos = friends_str.find('|', pos);
                        if (pipe_pos == std::string::npos) {
                            pipe_pos = friends_str.length();
                        }
                        std::string friend_info = friends_str.substr(pos, pipe_pos - pos);
                        std::cout << "  " << idx << ". " << friend_info << "\n";
                        idx++;
                        pos = pipe_pos + 1;
                    }
                }
            }
        } else if (response.command == "ERROR") {
            std::cout << "\n✗ Error: " << response.params["error_msg"] << "\n";
        }

        std::cout << "Press Enter to continue...\n";
        std::string dummy;
        std::getline(std::cin, dummy);
    }

    void handle_friend_requests() {
        std::cout << "\n--- Friend Requests ---\n";
        std::cout << "Enter 0 to go back\n";

        while (true) {
            Message msg;
            msg.command = "LIST_PENDING_REQUESTS";
            msg.params["token"] = current_token;

            if (!socket.send_message(msg)) {
                std::cout << "✗ Failed to fetch pending requests\n";
                continue;
            }

            Message response = socket.receive_message();
            if (response.command == "PENDING_REQUESTS") {
                int request_count = std::stoi(response.params["request_count"]);
                if (request_count == 0) {
                    std::cout << "\nNo pending friend requests.\n";
                    std::string dummy;
                    std::cout << "Enter 0 to go back: ";
                    std::getline(std::cin, dummy);
                    if (dummy == "0") break;
                    continue;
                }

                std::cout << "\nPending Requests (" << request_count << "):\n";
                std::string requests_str = response.params["requests"];
                std::cout << requests_str << "\n";
                std::cout << "\nEnter username to accept/reject or 0 to back: ";
                std::string username;
                std::getline(std::cin, username);

                if (username == "0") {
                    break;
                }

                if (username.empty()) {
                    continue;
                }

                std::cout << "1. Accept\n2. Reject\nChoose (1-2): ";
                int choice = 0;
                while (!(std::cin >> choice) || choice < 1 || choice > 2) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid option. Try again (1-2): ";
                }
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (choice == 1) {
                    Message req_msg;
                    req_msg.command = "ACCEPT_FRIEND_REQUEST";
                    req_msg.params["token"] = current_token;
                    req_msg.params["from_username"] = username;

                    if (!socket.send_message(req_msg)) {
                        std::cout << "✗ Failed to accept request\n";
                        continue;
                    }

                    Message req_response = socket.receive_message();
                    if (req_response.command == "FRIEND_REQUEST_ACCEPTED") {
                        std::cout << "\n✓ Friend request from " << username << " accepted!\n";
                    } else if (req_response.command == "ERROR") {
                        std::cout << "\n✗ Failed: " << req_response.params["error_msg"] << "\n";
                    }
                } else {
                    Message req_msg;
                    req_msg.command = "REJECT_FRIEND_REQUEST";
                    req_msg.params["token"] = current_token;
                    req_msg.params["from_username"] = username;

                    if (!socket.send_message(req_msg)) {
                        std::cout << "✗ Failed to reject request\n";
                        continue;
                    }

                    Message req_response = socket.receive_message();
                    if (req_response.command == "FRIEND_REQUEST_REJECTED") {
                        std::cout << "\n✓ Friend request from " << username << " rejected!\n";
                    } else if (req_response.command == "ERROR") {
                        std::cout << "\n✗ Failed: " << req_response.params["error_msg"] << "\n";
                    }
                }
            } else if (response.command == "ERROR") {
                std::cout << "\n✗ Error: " << response.params["error_msg"] << "\n";
                break;
            }
        }
    }
};

int main() {
    VuaTiengVietClient client;
    client.run();
    return 0;
}
