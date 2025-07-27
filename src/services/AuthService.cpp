#include "services/AuthService.hpp"
#include "data/Database.hpp"

std::optional<User> AuthService::loginOrRegister(const QString& username) {
    auto existing = UserRepository::findByName(username);
    if (existing) return existing;

    return UserRepository::create(username);
}
