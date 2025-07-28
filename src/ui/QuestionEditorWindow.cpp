#include "ui/QuestionEditorWindow.hpp"

#include "repositories/QuizRepository.hpp"
#include "repositories/QuestionRepository.hpp"
#include "repositories/AnswerRepository.hpp"
#include "data/Database.hpp"

#include <QWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include <QScrollArea>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QIntValidator>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTextEdit>

QuestionEditorWindow::QuestionEditorWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Question Editor");
    resize(1000, 650);

    Database::instance().open();
    Database::instance().ensureSchema();

    // ----- ROOT LAYOUT -----
    auto* central = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(central);

    // --- Quiz bar (top)
    {
        auto* quizLayout = new QHBoxLayout();
        m_quizCombo = new QComboBox(central);
        m_newQuizBtn = new QPushButton("New Quiz", central);
        quizLayout->addWidget(new QLabel("Quiz:"));
        quizLayout->addWidget(m_quizCombo, 1);
        quizLayout->addWidget(m_newQuizBtn);
        rootLayout->addLayout(quizLayout);

        connect(m_newQuizBtn, &QPushButton::clicked, this, &QuestionEditorWindow::onCreateQuiz);
        connect(m_quizCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &QuestionEditorWindow::onQuizChanged);
    }

    // --- Splitter: left list, right editor
    auto* splitter = new QSplitter(Qt::Horizontal, central);
    rootLayout->addWidget(splitter, 1);

    // LEFT: table of questions
    {
        auto* leftWidget = new QWidget(splitter);
        auto* leftLayout = new QVBoxLayout(leftWidget);

        m_questionTable = new QTableView(leftWidget);
        m_questionModel = new QStandardItemModel(leftWidget);
        m_questionModel->setHorizontalHeaderLabels({"ID", "Text", "Type", "Diff"});
        m_questionTable->setModel(m_questionModel);
        m_questionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_questionTable->setSelectionMode(QAbstractItemView::SingleSelection);
        m_questionTable->horizontalHeader()->setStretchLastSection(true);
        m_questionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_questionTable->setColumnHidden(0, true); // hide ID
        leftLayout->addWidget(m_questionTable, 1);

        auto* btnBar = new QHBoxLayout();
        m_newQuestionBtn = new QPushButton("New", leftWidget);
        m_deleteBtn      = new QPushButton("Delete", leftWidget);
        m_previewBtn     = new QPushButton("Preview", leftWidget);
        btnBar->addWidget(m_newQuestionBtn);
        btnBar->addWidget(m_deleteBtn);
        btnBar->addWidget(m_previewBtn);
        leftLayout->addLayout(btnBar);

        connect(m_questionTable, &QTableView::doubleClicked,
                this, &QuestionEditorWindow::onQuestionRowActivated);
        connect(m_newQuestionBtn, &QPushButton::clicked,
                this, &QuestionEditorWindow::onNewQuestion);
        connect(m_deleteBtn, &QPushButton::clicked,
                this, &QuestionEditorWindow::onDeleteQuestion);
        connect(m_previewBtn, &QPushButton::clicked,
                this, &QuestionEditorWindow::onPreviewQuestion);

        leftWidget->setLayout(leftLayout);
        splitter->addWidget(leftWidget);
    }

    // RIGHT: editor form
    {
        auto* rightWidget = new QWidget(splitter);
        auto* editorLayout = new QVBoxLayout(rightWidget);

        auto* form = new QFormLayout();
        m_questionText = new QLineEdit(rightWidget);
        m_hintEdit     = new QLineEdit(rightWidget);
        m_typeCombo    = new QComboBox(rightWidget);
        m_difficultyEdit = new QLineEdit(rightWidget);
        m_difficultyEdit->setValidator(new QIntValidator(0, 10, m_difficultyEdit));

        m_typeCombo->addItem("True/False", static_cast<int>(QuestionType::TrueFalse));
        m_typeCombo->addItem("Multiple Choice", static_cast<int>(QuestionType::MultipleChoice));
        m_typeCombo->addItem("Short Answer", static_cast<int>(QuestionType::ShortAnswer));

        form->addRow("Question text:", m_questionText);
        form->addRow("Hint:", m_hintEdit);
        form->addRow("Type:", m_typeCombo);
        form->addRow("Difficulty (0-10):", m_difficultyEdit);

        editorLayout->addLayout(form);

        connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &QuestionEditorWindow::onQuestionTypeChanged);

        // Type-specific area
        m_typeStack = new QStackedWidget(rightWidget);
        editorLayout->addWidget(m_typeStack, 1);

        // Page 0: True/False
        {
            QWidget* tfPage = new QWidget(m_typeStack);
            auto* v = new QVBoxLayout(tfPage);
            m_tfTrue  = new QRadioButton("True", tfPage);
            m_tfFalse = new QRadioButton("False", tfPage);
            m_tfTrue->setChecked(true);
            v->addWidget(m_tfTrue);
            v->addWidget(m_tfFalse);
            v->addStretch();
            m_typeStack->addWidget(tfPage);
        }

        // Page 1: Multiple choice
        {
            m_mcPage = new QWidget(m_typeStack);
            auto* pv = new QVBoxLayout(m_mcPage);

            auto* scroll = new QScrollArea(m_mcPage);
            scroll->setWidgetResizable(true);
            QWidget* container = new QWidget(scroll);
            m_mcLayout = new QVBoxLayout(container);
            container->setLayout(m_mcLayout);
            scroll->setWidget(container);

            m_addAnswerBtn = new QPushButton("Add answer", m_mcPage);

            pv->addWidget(scroll, 1);
            pv->addWidget(m_addAnswerBtn);

            connect(m_addAnswerBtn, &QPushButton::clicked,
                    this, &QuestionEditorWindow::onAddAnswerRow);

            m_typeStack->addWidget(m_mcPage);
        }

        // Page 2: Short answer
        {
            QWidget* saPage = new QWidget(m_typeStack);
            auto* v = new QVBoxLayout(saPage);
            m_shortAnswerEdit = new QLineEdit(saPage);
            v->addWidget(new QLabel("Correct answer:"));
            v->addWidget(m_shortAnswerEdit);
            v->addStretch();
            m_typeStack->addWidget(saPage);
        }

        // Save
        m_saveBtn = new QPushButton("Save", rightWidget);
        editorLayout->addWidget(m_saveBtn);
        connect(m_saveBtn, &QPushButton::clicked,
                this, &QuestionEditorWindow::onSaveQuestion);

        rightWidget->setLayout(editorLayout);
        splitter->addWidget(rightWidget);
    }

    setCentralWidget(central);

    reloadQuizList();
    onQuizChanged(m_quizCombo->currentIndex());
    rebuildTypePage(); // initial MC rows
}

QuestionEditorWindow::~QuestionEditorWindow() = default;

// ==== Slots ====

void QuestionEditorWindow::onCreateQuiz()
{
    bool ok = false;
    QString title = QInputDialog::getText(this, "New Quiz", "Title:", QLineEdit::Normal, {}, &ok);
    if (!ok || title.trimmed().isEmpty())
        return;

    Quiz q;
    q.title = title.trimmed();
    q.description = QString();
    int id = QuizRepository::create(q);
    if (id < 0) {
        QMessageBox::critical(this, "Error", "Failed to create quiz.");
        return;
    }
    reloadQuizList();
    // set current to the new one
    for (int i = 0; i < m_quizCombo->count(); ++i) {
        if (m_quizCombo->itemData(i).toInt() == id) {
            m_quizCombo->setCurrentIndex(i);
            break;
        }
    }
}

void QuestionEditorWindow::onQuizChanged(int)
{
    reloadQuestionList();
    resetEditor();
}

void QuestionEditorWindow::onQuestionRowActivated(const QModelIndex& index)
{
    if (!index.isValid()) return;
    int row = index.row();
    int qid = m_questionModel->item(row, 0)->text().toInt(); // hidden ID col
    loadQuestionIntoEditor(qid);
}

void QuestionEditorWindow::onNewQuestion()
{
    resetEditor();
    m_mode = Mode::Create;
}

void QuestionEditorWindow::onDeleteQuestion()
{
    auto idx = m_questionTable->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::information(this, "Info", "Select question to delete.");
        return;
    }
    int row = idx.row();
    int qid = m_questionModel->item(row, 0)->text().toInt();

    if (QMessageBox::question(this, "Confirm", "Delete this question?") == QMessageBox::Yes) {
        // remove answers -> cascade ON DELETE CASCADE for answers?
        AnswerRepository::removeByQuestion(qid);
        if (!QuestionRepository::remove(qid)) {
            QMessageBox::warning(this, "Error", "Delete failed.");
            return;
        }
        reloadQuestionList();
        if (m_currentQuestionId == qid) resetEditor();
    }
}

void QuestionEditorWindow::onPreviewQuestion()
{
    auto idx = m_questionTable->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::information(this, "Info", "Select question to preview.");
        return;
    }
    int row = idx.row();
    int qid = m_questionModel->item(row, 0)->text().toInt();

    auto qOpt = QuestionRepository::get(qid);
    if (!qOpt) return;
    auto answers = AnswerRepository::byQuestion(qid);

    // Simple preview dialog
    QDialog dlg(this);
    dlg.setWindowTitle("Preview");

    auto* v = new QVBoxLayout(&dlg);
    auto* text = new QTextEdit(&dlg);
    text->setReadOnly(true);

    QString html;
    html += "<b>Question:</b> " + qOpt->text.toHtmlEscaped() + "<br/>";
    html += "<b>Type:</b> " + QString::number(static_cast<int>(qOpt->type)) + "<br/>";
    if (!qOpt->hint.isEmpty())
        html += "<i>Hint: " + qOpt->hint.toHtmlEscaped() + "</i><br/><br/>";

    if (qOpt->type == QuestionType::TrueFalse) {
        for (const auto& a : answers) {
            html += QString(" - %1 %2<br/>")
            .arg(a.text.toHtmlEscaped())
                .arg(a.isCorrect ? "(correct)" : "");
        }
    } else if (qOpt->type == QuestionType::MultipleChoice) {
        int n = 1;
        for (const auto& a : answers) {
            html += QString("%1) %2 %3<br/>")
            .arg(n++)
                .arg(a.text.toHtmlEscaped())
                .arg(a.isCorrect ? "(correct)" : "");
        }
    } else {
        if (!answers.isEmpty())
            html += "Correct answer: <b>" + answers.first().text.toHtmlEscaped() + "</b><br/>";
    }

    text->setHtml(html);
    v->addWidget(text);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    v->addWidget(box);

    dlg.exec();
}

void QuestionEditorWindow::onQuestionTypeChanged(int)
{
    rebuildTypePage();
}

void QuestionEditorWindow::onAddAnswerRow()
{
    addAnswerRowMC();
}

void QuestionEditorWindow::onSaveQuestion()
{
    Question q;
    QList<Answer> answers;
    if (!validateAndCollect(q, answers))
        return;

    if (m_mode == Mode::Create) {
        int qid = QuestionRepository::create(q);
        if (qid < 0) {
            QMessageBox::critical(this, "Error", "Failed to insert question.");
            return;
        }
        for (auto& a : answers) {
            a.questionId = qid;
            if (AnswerRepository::create(a) < 0) {
                QMessageBox::warning(this, "Warning", "Failed to insert some answer.");
            }
        }
        QMessageBox::information(this, "OK", "Question created.");
        reloadQuestionList();
        resetEditor(); // ready for new create
    } else {
        // Update existing
        q.id = m_currentQuestionId;
        if (!QuestionRepository::update(q)) {
            QMessageBox::critical(this, "Error", "Question update failed!");
            return;
        }
        // Remove old answers and insert new
        AnswerRepository::removeByQuestion(q.id);
        for (auto& a : answers) {
            a.questionId = q.id;
            AnswerRepository::create(a);
        }
        QMessageBox::information(this, "OK", "Question updated.");
        reloadQuestionList();
    }
}

// ==== Helpers ====

void QuestionEditorWindow::rebuildTypePage()
{
    m_typeStack->setCurrentIndex(m_typeCombo->currentIndex());

    if (static_cast<QuestionType>(m_typeCombo->currentData().toInt()) == QuestionType::MultipleChoice) {
        clearAnswersMC();
        // default 4 empty rows
        for (int i = 0; i < 4; ++i) addAnswerRowMC();
    }
}

bool QuestionEditorWindow::validateAndCollect(Question& outQ, QList<Answer>& outA)
{
    if (m_quizCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "Validation", "Select a quiz.");
        return false;
    }
    const QString text = m_questionText->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Question text is empty.");
        return false;
    }

    outQ.quizId = m_quizCombo->currentData().toInt();
    outQ.text   = text;
    outQ.hint   = m_hintEdit->text().trimmed();
    outQ.difficulty = m_difficultyEdit->text().toInt();
    outQ.type = static_cast<QuestionType>(m_typeCombo->currentData().toInt());

    switch (outQ.type) {
    case QuestionType::TrueFalse: {
        Answer aTrue;  aTrue.text = "True";  aTrue.isCorrect  = m_tfTrue->isChecked();
        Answer aFalse; aFalse.text = "False"; aFalse.isCorrect = m_tfFalse->isChecked();
        outA << aTrue << aFalse;
        if (!aTrue.isCorrect && !aFalse.isCorrect) {
            QMessageBox::warning(this, "Validation", "Mark True or False as correct.");
            return false;
        }
        break;
    }
    case QuestionType::MultipleChoice: {
        if (m_mcRows.isEmpty()) {
            QMessageBox::warning(this, "Validation", "Add at least one answer.");
            return false;
        }
        bool anyCorrect = false;
        for (const auto& r : m_mcRows) {
            QString t = r.text->text().trimmed();
            if (t.isEmpty()) continue;
            Answer a; a.text = t; a.isCorrect = r.correct->isChecked();
            anyCorrect = anyCorrect || a.isCorrect;
            outA << a;
        }
        if (outA.size() < 2) {
            QMessageBox::warning(this, "Validation", "Need at least 2 non-empty answers.");
            return false;
        }
        if (!anyCorrect) {
            QMessageBox::warning(this, "Validation", "At least one answer must be correct.");
            return false;
        }
        break;
    }
    case QuestionType::ShortAnswer: {
        QString corr = m_shortAnswerEdit->text().trimmed();
        if (corr.isEmpty()) {
            QMessageBox::warning(this, "Validation", "Enter correct answer.");
            return false;
        }
        Answer a; a.text = corr; a.isCorrect = true;
        outA << a;
        break;
    }
    }
    return true;
}

void QuestionEditorWindow::clearAnswersMC()
{
    for (auto& row : m_mcRows) {
        row.rowWidget->deleteLater();
    }
    m_mcRows.clear();
}

void QuestionEditorWindow::addAnswerRowMC(const QString& text, bool correct)
{
    QWidget* row = new QWidget(m_mcPage);
    auto* h = new QHBoxLayout(row);
    auto* le = new QLineEdit(row);
    auto* cb = new QCheckBox("Correct", row);
    auto* rm = new QPushButton("X", row);
    rm->setFixedWidth(24);

    le->setText(text);
    cb->setChecked(correct);

    h->addWidget(le, 1);
    h->addWidget(cb);
    h->addWidget(rm);
    m_mcLayout->addWidget(row);

    MCRow r{ row, le, cb };
    m_mcRows.push_back(r);

    connect(rm, &QPushButton::clicked, this, [this, row]() {
        for (int i = 0; i < m_mcRows.size(); ++i) {
            if (m_mcRows[i].rowWidget == row) {
                m_mcRows.removeAt(i);
                row->deleteLater();
                break;
            }
        }
    });
}

void QuestionEditorWindow::reloadQuizList()
{
    m_quizzes = QuizRepository::all();

    m_quizCombo->clear();
    for (const auto& q : m_quizzes) {
        m_quizCombo->addItem(q.title, q.id);
    }
}

void QuestionEditorWindow::reloadQuestionList()
{
    m_questionModel->removeRows(0, m_questionModel->rowCount());

    if (m_quizCombo->currentIndex() < 0) return;

    int quizId = m_quizCombo->currentData().toInt();
    auto qs = QuestionRepository::byQuiz(quizId);

    m_questionModel->setRowCount(qs.size());
    for (int i = 0; i < qs.size(); ++i) {
        const auto& qq = qs[i];
        auto* itId   = new QStandardItem(QString::number(qq.id));
        auto* itText = new QStandardItem(qq.text.left(60)); // crop
        auto* itType = new QStandardItem(QString::number(static_cast<int>(qq.type)));
        auto* itDiff = new QStandardItem(QString::number(qq.difficulty));

        m_questionModel->setItem(i, 0, itId);
        m_questionModel->setItem(i, 1, itText);
        m_questionModel->setItem(i, 2, itType);
        m_questionModel->setItem(i, 3, itDiff);
    }
    m_questionTable->resizeColumnsToContents();
}

void QuestionEditorWindow::loadQuestionIntoEditor(int questionId)
{
    auto qOpt = QuestionRepository::get(questionId);
    if (!qOpt) return;

    resetEditor(); // clear fields/rows
    m_mode = Mode::Edit;
    m_currentQuestionId = questionId;

    const auto q = qOpt.value();
    // set fields
    m_questionText->setText(q.text);
    m_hintEdit->setText(q.hint);
    m_difficultyEdit->setText(QString::number(q.difficulty));

    // set type combobox -> triggers rebuild
    for (int i = 0; i < m_typeCombo->count(); ++i) {
        if (m_typeCombo->itemData(i).toInt() == static_cast<int>(q.type)) {
            m_typeCombo->setCurrentIndex(i);
            break;
        }
    }

    // load answers
    auto answers = AnswerRepository::byQuestion(q.id);
    switch (q.type) {
    case QuestionType::TrueFalse: {
        // expect two answers True/False
        bool tCorrect = false;
        bool fCorrect = false;
        for (const auto& a : answers) {
            if (a.text.compare("true", Qt::CaseInsensitive) == 0)
                tCorrect = a.isCorrect;
            else if (a.text.compare("false", Qt::CaseInsensitive) == 0)
                fCorrect = a.isCorrect;
        }
        m_tfTrue->setChecked(tCorrect);
        m_tfFalse->setChecked(fCorrect);
        break;
    }
    case QuestionType::MultipleChoice: {
        clearAnswersMC();
        for (const auto& a : answers) {
            addAnswerRowMC(a.text, a.isCorrect);
        }
        if (answers.isEmpty()) { // ensure at least one blank row
            addAnswerRowMC();
        }
        break;
    }
    case QuestionType::ShortAnswer: {
        if (!answers.isEmpty())
            m_shortAnswerEdit->setText(answers.first().text);
        break;
    }
    }
}

void QuestionEditorWindow::resetEditor()
{
    m_mode = Mode::Create;
    m_currentQuestionId = -1;
    m_questionText->clear();
    m_hintEdit->clear();
    m_difficultyEdit->clear();
    m_typeCombo->setCurrentIndex(0);
    rebuildTypePage();
}
