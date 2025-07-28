#pragma once

#include "core/BaseWindow.hpp"
#include "models/User.hpp"

class QPushButton;
class QuestionEditorWindow;

class MainMenuWindow : public BaseWindow {
    Q_OBJECT

public:
    explicit MainMenuWindow(const User& user, QWidget* parent = nullptr);

private slots:
    void onStartQuiz();
    void onLeaderboard();
    void onHistory();
    void onQuestionEditor();

private:
    User m_user;
    QPushButton* m_startBtn{};
    QPushButton* m_leaderboardBtn{};
    QPushButton* m_historyBtn{};
    QPushButton* m_questionEditorBtn{};
};
