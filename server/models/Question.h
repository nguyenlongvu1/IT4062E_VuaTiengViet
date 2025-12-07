#pragma once
#include <string>

struct Question {
    int id = 0;
    std::string text;
    std::string optionA, optionB, optionC, optionD;
    std::string correctAnswer;
    std::string category;
    // removed: type, difficulty
};
