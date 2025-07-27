#pragma once

#include <QString>
#include <optional>

struct User {
    int         id          = -1;
    QString     username;
};

// Q_DECLARE_METATYPE(User);


class UserRepository {
public:
    static std::optional<User> findByName(const QString& name);
    static User create(const QString& name);
};
