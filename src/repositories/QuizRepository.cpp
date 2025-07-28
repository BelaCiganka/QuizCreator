#include "repositories/QuizRepository.hpp"
#include "data/Database.hpp"

#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QDebug>

int QuizRepository::create(const Quiz& q) {
    QSqlQuery qu(Database::instance().db());
    qu.prepare("INSERT INTO quizzes(title, description) VALUES (:t, :d)");
    qu.bindValue(":t", q.title);
    qu.bindValue(":d", q.description);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return -1;
    }
    return qu.lastInsertId().toInt();
}

bool QuizRepository::update(const Quiz& q) {
    QSqlQuery qu(Database::instance().db());
    qu.prepare("UPDATE quizzes SET title=:t, description=:d WHERE id=:id");
    qu.bindValue(":t", q.title);
    qu.bindValue(":d", q.description);
    qu.bindValue(":id", q.id);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return false;
    }
    return qu.numRowsAffected() > 0;
}

bool QuizRepository::remove(int id) {
    QSqlQuery qu(Database::instance().db());
    qu.prepare("DELETE FROM quizzes WHERE id=:id");
    qu.bindValue(":id", id);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return false;
    }
    return qu.numRowsAffected() > 0;
}

std::optional<Quiz> QuizRepository::get(int id) {
    QSqlQuery qu(Database::instance().db());
    qu.prepare("SELECT id, title, description FROM quizzes WHERE id=:id");
    qu.bindValue(":id", id);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return std::nullopt;
    }
    if (qu.next()) {
        Quiz q;
        q.id = qu.value(0).toInt();
        q.title = qu.value(1).toString();
        q.description = qu.value(2).toString();
        return q;
    }
    return std::nullopt;
}

QList<Quiz> QuizRepository::all() {
    QList<Quiz> list;
    QSqlQuery qu("SELECT id, title, description FROM quizzes ORDER BY id DESC", Database::instance().db());

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return list;
    }

    while (qu.next()) {
        Quiz q;
        q.id = qu.value(0).toInt();
        q.title = qu.value(1).toString();
        q.description = qu.value(2).toString();
        list.push_back(q);
    }

    return list;
}













