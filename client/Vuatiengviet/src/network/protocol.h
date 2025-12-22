#ifndef PROTOCOL_H
#define PROTOCOL_H

#define CMD_LOGIN       "LOGIN"
#define CMD_LOGIN_OK    "LOGIN_OK"
#define CMD_LOGIN_FAIL  "LOGIN_FAIL"

#define CMD_REGISTER    "REGISTER"
#define CMD_REG_OK      "REGISTER_OK"

// --- SỬA DÒNG NÀY ---
// Đừng để là "ERROR", hãy đổi thành cái gì đó server KHÔNG bao giờ gửi
// Hoặc đổi thành "REGISTER_FAIL" để tránh nhầm lẫn logic
#define CMD_REG_FAIL    "REGISTER_FAIL" 
#define CMD_LOGOUT "LOGOUT"

#endif // PROTOCOL_H