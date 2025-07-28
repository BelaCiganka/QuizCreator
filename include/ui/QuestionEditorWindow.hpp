#pragma once

#include "models/Question.hpp"
#include "models/Quiz.hpp"
#include "models/Answer.hpp"

#include <QMainWindow>
#include <QList>

class QComboBox;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QVBoxLayout;
class QScrollArea;
class QCheckBox;
class QRadioButton;
class QTableView;
class QStandardItemModel;
class QModelIndex;

class QuestionEditorWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit QuestionEditorWindow(QWidget* parent = nullptr);
    ~QuestionEditorWindow() override;

private slots:
    // Top controls
    void onCreateQuiz();
    void onQuizChanged(int);

    // Table / list
    void onQuestionRowActivated(const QModelIndex& index);
    void onNewQuestion();
    void onDeleteQuestion();
    void onPreviewQuestion();

    // Editor
    void onQuestionTypeChanged(int);
    void onAddAnswerRow();
    void onSaveQuestion();

private:
    enum class Mode { Create, Edit };
    Mode m_mode = Mode::Create;
    int  m_currentQuestionId = -1;

    // Helpers
    void rebuildTypePage();
    bool validateAndCollect(Question& outQ, QList<Answer>& outA);
    void clearAnswersMC();
    void addAnswerRowMC(const QString& text = QString(), bool correct = false);

    void reloadQuizList();
    void reloadQuestionList();
    void loadQuestionIntoEditor(int questionId);
    void resetEditor();

    // Widgets
    QComboBox*     m_quizCombo{};
    QPushButton*   m_newQuizBtn{};

    QTableView*            m_questionTable{};
    QStandardItemModel*    m_questionModel{};

    QLineEdit*     m_questionText{};
    QLineEdit*     m_hintEdit{};
    QComboBox*     m_typeCombo{};
    QLineEdit*     m_difficultyEdit{};

    QStackedWidget* m_typeStack{};   // 0=TrueFalse,1=MC,2=ShortAnswer

    // True/False
    QRadioButton*  m_tfTrue{};
    QRadioButton*  m_tfFalse{};

    // MultipleChoice
    QWidget*       m_mcPage{};
    QVBoxLayout*   m_mcLayout{};     // answers list
    QPushButton*   m_addAnswerBtn{};
    struct MCRow {
        QWidget*    rowWidget{};
        QLineEdit*  text{};
        QCheckBox*  correct{};
    };
    QList<MCRow> m_mcRows;

    // Short answer
    QLineEdit*     m_shortAnswerEdit{};

    QPushButton*   m_saveBtn{};
    QPushButton*   m_newQuestionBtn{};
    QPushButton*   m_deleteBtn{};
    QPushButton*   m_previewBtn{};

    QList<Quiz> m_quizzes;
};
