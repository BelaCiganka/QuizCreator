#include "models/User.hpp"
#include "data/Database.hpp"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

std::optional<User> UserRepository::findByName(const QString& name) {
    QSqlQuery q(Database::instance().db());
    q.prepare("SELECT id, username FROM users WHERE username = :u");
    q.bindValue(":u", name);
    if (!q.exec()) {
        qWarning() << q.lastError();
        return std::nullopt;
    }

    if (q.next()) {
        User u;
        u.id = q.value(0).toInt();
        u.username = q.value(1).toString();
        return u;
    }
    return std::nullopt;
}

User UserRepository::create(const QString& name) {
    QSqlQuery q(Database::instance().db());
    q.prepare("INSERT INTO users(username) VALUES(:u)");
    q.bindValue(":u", name);
    if (!q.exec()) {
        qWarning() << q.lastError();
    }

    return {q.lastInsertId().toInt(), name };   // Good example of lastInsertId :)
}
