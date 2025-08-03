#include "ui/MainMenuWindow.hpp"
#include "ui/QuizRunDialog.hpp"
#include "ui/QuestionEditorWindow.hpp"
#include "ui/QuizSelectDialog.hpp"
#include "ui/QuizRunnerDialog.hpp"

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
    m_questionEditorBtn = new QPushButton("Question Editor", central);

    layout->addWidget(hello);
    layout->addWidget(m_startBtn);
    layout->addWidget(m_leaderboardBtn);
    layout->addWidget(m_historyBtn);
    layout->addWidget(m_questionEditorBtn);
    layout->addStretch();

    setCentralWidget(central);

    // Connections
    connect(m_startBtn, &QPushButton::clicked, this, &MainMenuWindow::onStartQuiz);
    connect(m_leaderboardBtn, &QPushButton::clicked, this, &MainMenuWindow::onLeaderboard);
    connect(m_historyBtn, &QPushButton::clicked, this, &MainMenuWindow::onHistory);
    connect(m_questionEditorBtn, &QPushButton::clicked, this, &MainMenuWindow::onQuestionEditor);
}

void MainMenuWindow::onStartQuiz() {
    QuizSelectDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    QuizRunnerDialog run(m_user,
                         dlg.selectedQuizId(),
                         dlg.selectedMode(),
                         nullptr);

    run.exec();
}

void MainMenuWindow::onQuestionEditor() {
    auto* w = new QuestionEditorWindow();
    w->setAttribute(Qt::WA_DeleteOnClose, true);
    w->show();
}

void MainMenuWindow::onLeaderboard() {
    // TODO: open LeaderboardWindow
}

void MainMenuWindow::onHistory() {
    // TODO: open HistoryWindow
}
