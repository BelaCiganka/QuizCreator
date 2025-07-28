#pragma once

#include "models/Quiz.hpp"

#include <QList>
#include <optional>

class QuizRepository {
public:
    static int create(const Quiz& q);  // returns new id
    static bool update(const Quiz& q);
    static bool remove(int id);
    static std::optional<Quiz> get(int id);
    static QList<Quiz> all();
};
