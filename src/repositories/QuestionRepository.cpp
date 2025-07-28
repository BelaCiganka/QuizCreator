#include "repositories/QuestionRepository.hpp"
#include "data/Database.hpp"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

static int toInt(QuestionType t) { return static_cast<int>(t); }
static QuestionType toType(int v) { return static_cast<QuestionType>(v); }

int QuestionRepository::create(const Question &q) {
    QSqlQuery qu(Database::instance().db());
    qu.prepare(R"(
        INSERT INTO questions(quiz_id, type, text, hint, difficulty)
        VALUES(:quiz,:type,:text,:hint,:diff)
    )");
    qu.bindValue(":quiz", q.quizId);
    qu.bindValue(":type", toInt(q.type));
    qu.bindValue(":text", q.text);
    qu.bindValue(":hint", q.hint);
    qu.bindValue(":diff", q.difficulty);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return -1;
    }
    return qu.lastInsertId().toInt();
}

bool QuestionRepository::update(const Question &q)
{
    QSqlQuery qu(Database::instance().db());
    qu.prepare(R"(
        UPDATE questions SET quiz_id=:quiz, type=:type, text=:text, hind=:hint, difficulty=:diff
        WHERE id=:id
)");
    qu.bindValue(":quiz", q.quizId);
    qu.bindValue(":type", toInt(q.type));
    qu.bindValue(":text", q.text);
    qu.bindValue(":hint", q.hint);
    qu.bindValue(":diff", q.difficulty);
    qu.bindValue(":id", q.id);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return false;
    }

    return qu.numRowsAffected() > 0;
}

bool QuestionRepository::remove(int id) {
    QSqlQuery qu(Database::instance().db());
    qu.prepare("DELETE FROM questions WHERE id=:id");
    qu.bindValue(":id", id);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return false;
    }
    return qu.numRowsAffected() > 0;
}

std::optional<Question> QuestionRepository::get(int id) {
    QSqlQuery qu(Database::instance().db());
    qu.prepare("SELECT id, quiz_id, type, text, hint, difficulty FROM questions WHERE id=:id");
    qu.bindValue(":id", id);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return std::nullopt;
    }

    if (qu.next()) {
        Question q;
        q.id = qu.value(0).toInt();
        q.quizId = qu.value(1).toInt();
        q.type = toType(qu.value(2).toInt());  // Notice this ;)
        q.text = qu.value(3).toString();
        q.hint = qu.value(4).toString();
        q.difficulty = qu.value(5).toInt();

        return q;
    }

    return std::nullopt;
}

QList<Question> QuestionRepository::byQuiz(int quizId)
{
    QList<Question> list;
    QSqlQuery qu(Database::instance().db());
    qu.prepare("SELECT id, quiz_id, type, text, hint, difficulty FROM questions WHERE quiz_id=:q");
    qu.bindValue(":q", quizId);

    if (!qu.exec()) {
        qWarning() << qu.lastError();
        return list;
    }

    while (qu.next()) {
        Question q;
        q.id = qu.value(0).toInt();
        q.quizId = qu.value(1).toInt();
        q.type = toType(qu.value(2).toInt());  // Notice this ;)
        q.text = qu.value(3).toString();
        q.hint = qu.value(4).toString();
        q.difficulty = qu.value(5).toInt();
        list.push_back(q);
    }

    return list;
}





















