#include "repositories/AnswerRepository.hpp"
#include "data/Database.hpp"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

int AnswerRepository::create(const Answer& a) {
    QSqlQuery qu(Database::instance().db());
    qu.prepare("INSERT INTO answers(question_id, text, is_correct) VALUES(:qid,:text,:c");
    qu.bindValue(":qid", a.questionId);
    qu.bindValue(":text", a.text);
    qu.bindValue(":c", a.isCorrect ? 1 : 0);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return -1;
    }

    return qu.lastInsertId().toInt();
}

bool AnswerRepository::update(const Answer& a) {
    QSqlQuery qu(Database::instance().db());
    qu.prepare("UPDATE answers SET text=:t, is_correct=:c WHERE id=:id");
    qu.bindValue(":t", a.text);
    qu.bindValue(":c", a.isCorrect ? 1 : 0);
    qu.bindValue(":id", a.id);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return false;
    }

    return qu.numRowsAffected() > 0;
}

bool AnswerRepository::remove(int id) {
    QSqlQuery qu(Database::instance().db());
    qu.prepare("DELETE FROM answers WHERE id=:id");
    qu.bindValue(":id", id);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return false;
    }

    return qu.numRowsAffected() > 0;

}

bool AnswerRepository::removeByQuestion(int questionId) {
    QSqlQuery qu(Database::instance().db());
    qu.prepare("DELETE FROM answers WHERE question_id=:q");
    qu.bindValue(":q", questionId);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return false;
    }

    return true;
}

std::optional<Answer> AnswerRepository::get(int id) {
    QSqlQuery qu(Database::instance().db());
    qu.prepare("SELECT id, question_id, text, is_correct FROM answers WHERE id=:id");
    qu.bindValue(":id", id);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return std::nullopt;
    }

    if (qu.next()) {
        Answer a;
        a.id = qu.value(0).toInt();
        a.questionId = qu.value(1).toInt();
        a.text = qu.value(2).toString();
        a.isCorrect = qu.value(3).toInt() == 1;  // Take care ;)
        return a;
    }
    return std::nullopt;
}

QList<Answer> AnswerRepository::byQuestion(int questionId) {
    QList<Answer> list;
    QSqlQuery qu(Database::instance().db());
    qu.prepare("SELECT id, question_id, text, is_correct FROM answers WHERE question_id=:q");
    qu.bindValue(":q", questionId);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return list;
    }

    while (qu.next()) {
        Answer a;
        a.id = qu.value(0).toInt();
        a.questionId = qu.value(1).toInt();
        a.text = qu.value(2).toString();
        a.isCorrect = qu.value(3).toInt() == 1;  // Take care ;)
        list.push_back(a);
    }

    return list;
}














