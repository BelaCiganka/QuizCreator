#pragma once

#include <models/Answer.hpp>

#include <QList>
#include <optional>

class AnswerRepository {
public:
    static bool create(Answer& a);
    static bool update(const Answer& a);
    static bool remove(int id);
    static bool removeByQuestion(int questionId);
    static std::optional<Answer> get(int id);
    static QList<Answer> byQuestion(int questionId);
};
