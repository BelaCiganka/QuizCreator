#include "app/Application.hpp"
#include "ui/LoginWindow.hpp"

#include <QApplication>

Application::Application() = default;
Application::~Application() = default;

int Application::run() {
    showLogin();
    return QApplication::instance()->exec();
}

void Application::showLogin() {
    m_loginWindow = std::make_unique<LoginWindow>();
    m_loginWindow->show();
}
