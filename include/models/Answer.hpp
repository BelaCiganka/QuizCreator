#pragma once

#include <QString>

struct Answer {
    int id = -1;
    int questionId = -1;
    QString text;
    bool isCorrect = false;
};
