#pragma once
#include <optional>
#include <vector>
#include "../models/Question.h"

class QuestionDAO {
public:
    static std::optional<Question> findById(int questionId);
    static int createQuestion(const Question &q);
    static std::vector<Question> getRandomQuestions(const std::string &category, int count);
};
