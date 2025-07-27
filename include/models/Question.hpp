#pragma once

#include <QString>

enum class QuestionType : int {
    TrueFalse = 0,
    MultipleChoice = 1,
    ShortAnswer = 2
};

struct Question {
    int id = -1;
    int quizId = -1;
    QuestionType type = QuestionType::MultipleChoice;
    QString text;
    QString hint;
    int difficulty = 0;
};
