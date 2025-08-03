#include "data/Database.hpp"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>

Database::Database() = default;

Database& Database::instance() {
    static Database inst;
    return inst;
}

bool Database::open(const QString& customPath /* = QString() */)
{
    // Ако је база већ отворена – ништа не ради
    if (m_db.isValid() && m_db.isOpen())
        return true;

    // 1. Одреди коначну путању
    QString dbPath;
    if (customPath.isEmpty()) {
        //  ~/.local/share/QuizCreator  (или системски еквивалент)
        QString dir = QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation);    // нпр. ~/.local/share/QuizCreator
        if (dir.isEmpty()) {
            qCritical() << "Cannot determine AppDataLocation!";
            return false;
        }
        QDir().mkpath(dir);             // креирај ако не постоји
        dbPath = dir + "/quizcreator.db";
    } else {
        dbPath = customPath;
        QDir().mkpath(QFileInfo(dbPath).absolutePath());
    }

    // 2. Иницијализуј QSQLITE драјвер
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    // 3. Отвори везу
    if (!m_db.open()) {
        qCritical() << "DB open error:" << m_db.lastError();
        return false;
    }

    // 4. Обавезно укључи FOREIGN KEY ограничења (SQLite опција)
    {
        QSqlQuery q(m_db);
        q.exec("PRAGMA foreign_keys = ON;");
    }

    qDebug() << "SQLite DB opened at:" << dbPath;
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

    const char* ddlQuizzes = R"(
        CREATE TABLE IF NOT EXISTS quizzes(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            description TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP
        );
    )";

    const char* ddlQuestions = R"(
        CREATE TABLE IF NOT EXISTS questions(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            quiz_id INTEGER NOT NULL,
            type INTEGER NOT NULL,            -- 0=TrueFalse,1=MultipleChoice,2=ShortAnswer
            text TEXT NOT NULL,
            hint TEXT,
            difficulty INTEGER DEFAULT 0,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(quiz_id) REFERENCES quizzes(id) ON DELETE CASCADE
        );
    )";

    const char* ddlAnswers = R"(
        CREATE TABLE IF NOT EXISTS answers(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            question_id INTEGER NOT NULL,
            text TEXT NOT NULL,
            is_correct INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY(question_id) REFERENCES questions(id) ON DELETE CASCADE
        );
    )";

    const char* ddlUserResults = R"(
        CREATE TABLE IF NOT EXISTS user_results(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            quiz_id INTEGER NOT NULL,
            score INTEGER NOT NULL,
            total INTEGER NOT NULL,
            started_at TEXT DEFAULT CURRENT_TIMESTAMP,
            finished_at TEXT,
            duration_ms INTEGER,
            FOREIGN KEY(user_id) REFERENCES users(id),
            FOREIGN KEY(quiz_id) REFERENCES quizzes(id)
        );
    )";

    QSqlQuery q;
    if (!q.exec(ddlUsers)) qWarning() << q.lastError();
    if (!q.exec(ddlResults)) qWarning() << q.lastError();

    if (!q.exec(ddlQuizzes))     qWarning() << q.lastError();
    if (!q.exec(ddlQuestions))   qWarning() << q.lastError();
    if (!q.exec(ddlAnswers))     qWarning() << q.lastError();
    if (!q.exec(ddlUserResults)) qWarning() << q.lastError();

    // TODO: Add later: questions, answers, quizzes, tags...
}
