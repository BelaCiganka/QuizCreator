#pragma once

#include <QString>
#include <QSqlDatabase>

class Database {
public:
    static Database& instance();

    bool open(const QString& path = "quizcreator.db");
    void ensureSchema();  // CREATE TABLE IF NOT EXISTS...

    QSqlDatabase db() const {return m_db;}

private:
    Database();
    QSqlDatabase m_db;
};
