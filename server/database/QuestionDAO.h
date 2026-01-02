#pragma once
#include <optional>
#include "../models/Question.h"

class QuestionDAO {
public:
    static std::optional<Question> findById(int questionId);
    static int createQuestion(const Question &q);
};
