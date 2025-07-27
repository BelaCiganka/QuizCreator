#pragma once

#include "core/BaseWindow.hpp"
#include "models/User.hpp"
#include "services/AuthService.hpp"
#include <memory>

class QLineEdit;
class QPushButton;

class LoginWindow : public BaseWindow {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget* parent = nullptr);
    ~LoginWindow() override;  // out-of-line destructor

signals:
    void loggedIn(const User& user);

private slots:
    void onLoginClicked();

private:
    std::unique_ptr<AuthService> m_authService;
    QLineEdit* m_usernameEdit{};
    QPushButton* m_loginBtn{};
};
