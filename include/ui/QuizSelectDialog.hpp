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
    enum class Direction { Normal, Reverse };

    Mode selectedMode() const { return m_mode; }
    Direction  selectedDir()    const { return m_dir; }

private slots:
    void onStart();

private:
    QComboBox* m_quizBox{};
    QComboBox* m_modeBox{};
    QPushButton* m_startBtn{};
    QComboBox*  m_dirBox{};

    int  m_quizId { -1 };
    Mode m_mode   { Mode::Mixed };
    Direction   m_dir = Direction::Normal;

};
