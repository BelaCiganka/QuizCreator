#include "ui/LoginWindow.hpp"
#include "services/AuthService.hpp"
#include "models/User.hpp"
#include "ui/MainMenuWindow.hpp"
#include "data/Database.hpp"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget* parent)
    : BaseWindow(parent)
    , m_authService(std::make_unique<AuthService>()) {

    setWindowTitle("QUizCreator - Login");
    resize(400, 200);

    Database::instance().open();
    Database::instance().ensureSchema();

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);  // Only for central we add as argument

    auto* label = new QLabel("Username:", central);
    m_usernameEdit = new QLineEdit(central);
    m_loginBtn = new QPushButton("Login / Register", central);

    layout->addWidget(label);
    layout->addWidget(m_usernameEdit);
    layout->addWidget(m_loginBtn);
    layout->addStretch();

    setCentralWidget(central);

    // Connections
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
}

LoginWindow::~LoginWindow() = default;

void LoginWindow::onLoginClicked() {
    const auto username = m_usernameEdit->text().trimmed();
    if (username.isEmpty()) {
        QMessageBox::critical(this, "Greska", "Unesite korisnicko ime.");
        return;
    }

    std::optional<User> userOpt = m_authService->loginOrRegister(username);
    if (!userOpt.has_value()) {
        QMessageBox::critical(this, "Greska", "Neuspelo logovanje/registracija.");
        return;
    }

    // TODO: emit loggedIn(userOpt.value()); if you want to use signal
    auto* menu = new MainMenuWindow(userOpt.value());
    menu->show();

    close();
}
