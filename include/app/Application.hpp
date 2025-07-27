#pragma once
#include <memory>

class LoginWindow;

class Application {
public:
    Application();
    ~Application();
    int run();  // Start event loop

private:
    void showLogin();

    std::unique_ptr<LoginWindow> m_loginWindow;
};
