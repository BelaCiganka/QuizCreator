#pragma once
#include <QDialog>

class QComboBox;
class QPushButton;

class QuizSelectDialog : public QDialog {
    Q_OBJECT
public:
    explicit QuizSelectDialog(QWidget* parent = nullptr);
    int  selectedQuizId() const { return m_quizId; }
    enum class Mode { Mixed, MultipleChoice, TrueFalse, ShortAnswer };
    Mode selectedMode() const { return m_mode; }

private slots:
    void onStart();

private:
    QComboBox* m_quizBox{};
    QComboBox* m_modeBox{};
    QPushButton* m_startBtn{};

    int  m_quizId { -1 };
    Mode m_mode   { Mode::Mixed };
};
