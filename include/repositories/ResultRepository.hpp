#pragma once

#include "models/UserResult.hpp"

#include <QList>
#include <optional>

class ResultRepository {
public:
    static int  create(const UserResult& r);
    static QList<UserResult> byUser(int userId);
    static QList<UserResult> byQuiz(int quizId);
    static QList<UserResult> leaderboard(int quizId, int limit = 50);
};
