#pragma once

#include "models/User.hpp"

#include <QString>
#include <optional>

class AuthService {
public:
    std::optional<User> loginOrRegister(const QString& username);
};
