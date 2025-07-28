#include "repositories/ResultRepository.hpp"
#include "data/Database.hpp"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

int ResultRepository::create(const UserResult& r) {
    QSqlQuery q(Database::instance().db());
    q.prepare(R"(
        INSERT INTO user_results(user_id,quiz_id,score,total,finished_at,duration_ms)
        VALUES(:u,:q,:s,:t,datetime('now'),:d)
    )");
    q.bindValue(":u", r.userId);
    q.bindValue(":q", r.quizId);
    q.bindValue(":s", r.score);
    q.bindValue(":t", r.total);
    q.bindValue(":d", 0);  // TODO: add this :)

    if (!q.exec()) {
        qWarning() << q.lastError();
        return -1;
    }
    return q.lastInsertId().toInt();
}

QList<UserResult> ResultRepository::byUser(int userId) {
    QList<UserResult> list;
    QSqlQuery q(Database::instance().db());
    q.prepare("SELECT id,user_id,quiz_id,score,total FROM user_results WHERE user_id=:u ORDER BY id DESC");
    q.bindValue(":u", userId);

    if (!q.exec()) {
        qWarning() << q.lastError();
        return list;
    }

    while (q.next()) {
        UserResult r;
        r.id     = q.value(0).toInt();
        r.userId = q.value(1).toInt();
        r.quizId = q.value(2).toInt();
        r.score  = q.value(3).toInt();
        r.total  = q.value(4).toInt();
        list.push_back(r);
    }
    return list;
}

QList<UserResult> ResultRepository::byQuiz(int quizId) {
    QList<UserResult> list;
    QSqlQuery q(Database::instance().db());
    q.prepare("SELECT id,user_id,quiz_id,score,total FROM user_results WHERE quiz_id=:q ORDER BY id DESC");
    q.bindValue(":q", quizId);

    if (!q.exec()) {
        qWarning() << q.lastError();
        return list;
    }

    while (q.next()) {
        UserResult r;
        r.id     = q.value(0).toInt();
        r.userId = q.value(1).toInt();
        r.quizId = q.value(2).toInt();
        r.score  = q.value(3).toInt();
        r.total  = q.value(4).toInt();
        list.push_back(r);
    }
    return list;
}

QList<UserResult> ResultRepository::leaderboard(int quizId, int limit) {
    QList<UserResult> list;
    QSqlQuery q(Database::instance().db());
    q.prepare(QStringLiteral(
                  "SELECT id,user_id,quiz_id,score,total FROM user_results "
                  "WHERE quiz_id=:q ORDER BY score DESC LIMIT %1").arg(limit));
    q.bindValue(":q", quizId);

    if (!q.exec()) {
        qWarning() << q.lastError();
        return list;
    }

    while (q.next()) {
        UserResult r;
        r.id     = q.value(0).toInt();
        r.userId = q.value(1).toInt();
        r.quizId = q.value(2).toInt();
        r.score  = q.value(3).toInt();
        r.total  = q.value(4).toInt();
        list.push_back(r);
    }
    return list;
}
