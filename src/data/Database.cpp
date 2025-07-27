#include "data/Database.hpp"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Database::Database() = default;

Database& Database::instance() {
    static Database inst;
    return inst;
}

bool Database::open(const QString& path) {
    if (m_db.isValid() && m_db.isOpen())
        return true;

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qCritical() << "DB open error: " << m_db.lastError();
        return false;
    }

    return true;
}

void Database::ensureSchema() {
    // Minimal tables for beginning
    const char* ddlUsers = R"(
        CREATE TABLE IF NOT EXISTS users(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP
        );
    )";

    const char* ddlResults = R"(
        CREATE TABLE IF NOT EXISTS results(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            quiz_id INTEGER NOT NULL,
            score INTEGER NOT NULL,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(user_id) REFERENCES users(id)
        );
    )";

    QSqlQuery q;
    if (!q.exec(ddlUsers)) qWarning() << q.lastError();
    if (!q.exec(ddlResults)) qWarning() << q.lastError();

    // TODO: Add later: questions, answers, quizzes, tags...
}
