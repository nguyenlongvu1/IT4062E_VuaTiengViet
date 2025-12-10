#ifndef PROTOCOL_H
#define PROTOCOL_H

// Các lệnh (Commands) dựa theo Slide 16
#define CMD_LOGIN       "LOGIN"
#define CMD_LOGIN_OK    "LOGIN_OK"
#define CMD_LOGIN_FAIL  "LOGIN_FAIL"

#define CMD_REGISTER    "REGISTER"      // Bổ sung cho chức năng Đăng ký
#define CMD_REG_OK      "REGISTER_OK"
#define CMD_REG_FAIL    "REG_FAIL"

// Format bản tin (Slide 15): Text-based TLV
// COMMAND: [CMD]\nLENGTH: [LEN]\n[PAYLOAD]

#endif // PROTOCOL_H