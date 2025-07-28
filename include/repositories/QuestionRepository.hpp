#pragma once

#include "models/Question.hpp"

#include <QList>
#include <optional>

class QuestionRepository {
public:
    static int create(const Question& q);
    static bool update(const Question& q);
    static bool remove(int id);
    static std::optional<Question> get(int id);
    static QList<Question> byQuiz(int quizId);
};
