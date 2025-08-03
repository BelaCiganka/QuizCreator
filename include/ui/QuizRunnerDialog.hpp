#pragma once
#include <QDialog>
#include <QProgressBar>
#include <QTimer>
#include "models/Question.hpp"
#include "models/Answer.hpp"
#include "models/User.hpp"
#include "ui/QuizSelectDialog.hpp"   // for Mode

class QLabel;
class QWidget;
class QVBoxLayout;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QLineEdit;
class QButtonGroup;

struct SessionQuestion {
    Question            q;
    QList<Answer>       answers;   // shuffled (MC / TF)
    int                 userChoice = -1; // index, or -1
    QString             userText;       // for short-answer
};

class QuizRunnerDialog : public QDialog {
    Q_OBJECT
public:
    QuizRunnerDialog(const User& user,
                     int quizId,
                     QuizSelectDialog::Mode mode,
                     QuizSelectDialog::Direction dir,
                     QWidget* parent=nullptr);

    bool eventFilter(QObject *, QEvent *ev);

private slots:
    void onSubmit();
    void onNext();
    void onHint();

private:
    void loadQuestions();
    void setupUI();
    void showCurrentQuestion();
    bool checkAnswer(SessionQuestion& sq);     // returns correct?
    void loadLocalStyle();

    // widgets
    QLabel*        m_questionLabel{};
    QWidget*       m_answerWidget{};  // container swapped per type
    QVBoxLayout*   m_answerLayout{};  // inside container
    QPushButton*   m_submitBtn{};
    QPushButton*   m_nextBtn{};
    QLabel*        m_progressLbl{};
    QProgressBar* m_progBar{};

    // helpers for radios
    QList<QRadioButton*> m_radioBtns;
    QList<QCheckBox*>    m_tfChecks;
    QLineEdit*           m_shortEdit{};

    QPushButton*   m_hintBtn{};
    int            m_hintLevel = 0;
    QLabel*        m_hintLabel{};
    int QRegExp(const char *);

    // data
    const User m_user;
    const int  m_quizId;
    const QuizSelectDialog::Mode m_mode;
    const QuizSelectDialog::Direction m_dir;

    QList<SessionQuestion> m_questions;
    int    m_currIdx  = 0;
    int    m_score    = 0;

    QTimer        m_timer;
    int           m_elapsedMs = 0;

    QLabel *m_timerLbl;

};
