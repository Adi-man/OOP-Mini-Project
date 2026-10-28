#include "httplib.h"
#include <iostream>
#include <vector>
#include <string>
#include <functional>

// --- Core OOP Model ---
enum class Role { ADMINISTRATOR, DOCTOR, RECEPTIONIST, NONE };

std::string roleToString(Role r) {
    switch(r) {
        case Role::ADMINISTRATOR: return "admin";
        case Role::DOCTOR: return "doctor";
        case Role::RECEPTIONIST: return "receptionist";
        default: return "none";
    }
}

class User {
private:
    std::string staffId;
    std::size_t passwordHash;
    Role role;

public:
    User(std::string id, std::string password, Role r) : staffId(id), role(r) {
        std::hash<std::string> hasher;
        passwordHash = hasher(password);
    }
    bool verify(std::string id, std::string password) const {
        std::hash<std::string> hasher;
        return (staffId == id && passwordHash == hasher(password));
    }
    Role getRole() const { return role; }
    std::string getId() const { return staffId; }
};

class HealthcareSystem {
private:
    std::vector<User> users;
public:
    HealthcareSystem() {
        users.push_back(User("admin001", "Admin@123", Role::ADMINISTRATOR));
        users.push_back(User("doctor001", "Doc@123", Role::DOCTOR));
        users.push_back(User("doctor002", "Doc@123", Role::DOCTOR)); // Added missing doctor account
        users.push_back(User("doctor003", "Doc@123", Role::DOCTOR)); // Added missing doctor account
        users.push_back(User("desk001", "Desk@123", Role::RECEPTIONIST));
    }

    User* authenticate(const std::string& id, const std::string& password) {
        for (auto& u : users) {
            if (u.verify(id, password)) return &u;
        }
        return nullptr;
    }
};

// --- HTTP Server Setup ---
int main() {
    httplib::Server svr;
    HealthcareSystem system;

    // 1. Serve the frontend HTML/CSS/JS files from the "public" directory
    auto ret = svr.set_mount_point("/", "../public");
    if (!ret) {
        std::cerr << "Error: The 'public' directory does not exist or cannot be read.\n";
    }

    // 2. API Endpoint: Login
    svr.Post("/api/login", [&](const httplib::Request& req, httplib::Response& res) {
        // Parse incoming form data from JavaScript
        std::string userid = req.get_param_value("userid");
        std::string password = req.get_param_value("password");

        std::cout << "[SERVER] Login attempt for user: " << userid << "\n";

        User* authenticatedUser = system.authenticate(userid, password);

        res.set_header("Content-Type", "application/json");
        
        if (authenticatedUser != nullptr) {
            // Manual JSON construction to avoid external dependencies
            std::string roleStr = roleToString(authenticatedUser->getRole());
            std::string jsonResponse = "{\"status\":\"success\",\"role\":\"" + roleStr + "\"}";
            res.set_content(jsonResponse, "application/json");
            std::cout << "[AUDIT LOG] ALLOW | User: " << userid << " authenticated successfully.\n";
        } else {
            res.set_content("{\"status\":\"denied\",\"message\":\"Invalid credentials\"}", "application/json");
            std::cout << "[AUDIT LOG] DENY | User: " << userid << " failed authentication.\n";
        }
    });

    std::cout << "=================================================\n";
    std::cout << " MediShield C++ Backend Server Running!\n";
    std::cout << " Open your browser to: http://localhost:8080/index.html\n";
    std::cout << "=================================================\n";
    
    svr.listen("localhost", 8080);
    return 0;
}