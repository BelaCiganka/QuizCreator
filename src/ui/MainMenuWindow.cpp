#include "ui/MainMenuWindow.hpp"
#include "ui/QuizRunDialog.hpp"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

MainMenuWindow::MainMenuWindow(const User& user, QWidget* parent)
    : BaseWindow(parent)
    , m_user(user) {

    setWindowTitle("QuizCreator - Main Menu");
    resize(500, 300);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    auto* hello = new QLabel(QString("Zdravo, %1!").arg(m_user.username), central);

    m_startBtn          = new QPushButton("Start Quiz", central);
    m_leaderboardBtn    = new QPushButton("Leaderboard", central);
    m_historyBtn        = new QPushButton("My results", central);

    layout->addWidget(hello);
    layout->addWidget(m_startBtn);
    layout->addWidget(m_leaderboardBtn);
    layout->addWidget(m_historyBtn);
    layout->addStretch();

    setCentralWidget(central);

    // Connections
    connect(m_startBtn, &QPushButton::clicked, this, &MainMenuWindow::onStartQuiz);
    connect(m_leaderboardBtn, &QPushButton::clicked, this, &MainMenuWindow::onLeaderboard);
    connect(m_historyBtn, &QPushButton::clicked, this, &MainMenuWindow::onHistory);
}

void MainMenuWindow::onStartQuiz() {
    QuizRunDialog dlg(m_user, this);
    dlg.exec();
}

void MainMenuWindow::onLeaderboard() {
    // TODO: open LeaderboardWindow
}

void MainMenuWindow::onHistory() {
    // TODO: open HistoryWindow
}
