#include "ui/QuizRunnerDialog.hpp"
#include "repositories/QuestionRepository.hpp"
#include "repositories/AnswerRepository.hpp"
#include "repositories/ResultRepository.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QButtonGroup>
#include <QRegularExpression>

QuizRunnerDialog::QuizRunnerDialog(const User& user,
                                   int quizId,
                                   QuizSelectDialog::Mode mode,
                                   QWidget* parent)
    : QDialog(parent)
    , m_user(user)
    , m_quizId(quizId)
    , m_mode(mode)
{
    setWindowTitle("Quiz in progress");
    resize(700,500);

    loadQuestions();
    if (m_questions.isEmpty()) {
        QMessageBox::information(this,"Empty","No questions in quiz.");
        reject(); return;
    }
    setupUI();
    showCurrentQuestion();
}

// ---------- data load ----------
void QuizRunnerDialog::loadQuestions()
{
    // 0) sve iz baze za taj kviz + shuffle
    auto qs = QuestionRepository::byQuiz(m_quizId);
    std::shuffle(qs.begin(), qs.end(), *QRandomGenerator::global());

    for (const auto &q : qs)
    {
        // 1) filter po režimu  ─────────────────────────────────────────
        //    True/False mod: prihvati samo TF
        if (m_mode == QuizSelectDialog::Mode::TrueFalse && q.type != QuestionType::TrueFalse)
            continue;

        //    MultipleChoice mod: odbaci SAMO True/False,
        //    zadrži MultipleChoice + ShortAnswer (konvertovaćemo ih)
        if (m_mode == QuizSelectDialog::Mode::MultipleChoice && q.type == QuestionType::TrueFalse)
            continue;
        //  (Mixed mod ne odbacuje ništa ovde)

        // 2) napravi SessionQuestion zapis
        SessionQuestion sq;
        sq.q       = q;
        sq.answers = AnswerRepository::byQuestion(q.id);

        //    elementarna validacija – preskoči ako nema odgovora
        if (sq.answers.isEmpty())
            continue;

        switch (q.type) {
        case QuestionType::TrueFalse:
            if (sq.answers.size() != 2) continue;
            break;
        case QuestionType::MultipleChoice:
            if (sq.answers.size() < 2)  continue;
            break;
        case QuestionType::ShortAnswer:
            /* bar 1 odgovor već imamo */
            break;
        }

        // 3) Kada je režim MC *ili* Mixed i pitanje je ShortAnswer
        //    → generiši MC varijantu (1 tačan + 3 pogrešna)
        if ((m_mode == QuizSelectDialog::Mode::MultipleChoice ||
             m_mode == QuizSelectDialog::Mode::Mixed) &&
            q.type  == QuestionType::ShortAnswer)
        {
            const QString correct = sq.answers.first().text;

            // pool drugih short-odgovora u istom kvizu
            QList<QString> pool;
            for (const auto &cand : qs) {
                if (cand.type != QuestionType::ShortAnswer || cand.id == q.id)
                    continue;
                auto candAns = AnswerRepository::byQuestion(cand.id);
                if (!candAns.isEmpty())
                    pool << candAns.first().text;
            }
            pool.removeAll(correct);
            std::shuffle(pool.begin(), pool.end(), *QRandomGenerator::global());

            // obezbedi 3 distractora
            while (pool.size() < 3)
                pool << "???";

            QList<Answer> mc;
            mc << Answer{ .text = correct, .isCorrect = true };
            for (int i = 0; i < 3; ++i)
                mc << Answer{ .text = pool[i], .isCorrect = false };
            std::shuffle(mc.begin(), mc.end(), *QRandomGenerator::global());

            // upiši izmenjeni tip i answers
            sq.q.type  = QuestionType::MultipleChoice;
            sq.answers = mc;
        }

        // 4) Ubaci u listu za kviz-run
        m_questions << sq;
    }
}
// ---------- UI ----------
void QuizRunnerDialog::setupUI()
{
    auto* v = new QVBoxLayout(this);

    m_progressLbl = new QLabel(this);
    v->addWidget(m_progressLbl);

    m_questionLabel = new QLabel(this);
    m_questionLabel->setWordWrap(true);
    m_questionLabel->setStyleSheet("font-weight:bold; font-size:16px");
    v->addWidget(m_questionLabel);

    m_answerWidget = new QWidget(this);
    m_answerLayout = new QVBoxLayout(m_answerWidget);
    v->addWidget(m_answerWidget,1);

    auto* h = new QHBoxLayout();
    m_submitBtn = new QPushButton("Submit", this);
    m_nextBtn   = new QPushButton("Next",   this);

    m_hintBtn = new QPushButton("Hint", this);     // NEW
    m_hintBtn->setEnabled(true);
    h->insertWidget(0, m_hintBtn);                 // levo od Submit

    m_hintLabel = new QLabel(this);                // NEW
    m_hintLabel->setStyleSheet("color:#ffaa00");
    v->insertWidget(2, m_hintLabel);               // ispod pitanja

    m_nextBtn->setEnabled(false);
    h->addStretch();
    h->addWidget(m_submitBtn);
    h->addWidget(m_nextBtn);
    v->addLayout(h);

    connect(m_submitBtn,&QPushButton::clicked,this,&QuizRunnerDialog::onSubmit);
    connect(m_nextBtn,  &QPushButton::clicked,this,&QuizRunnerDialog::onNext);
    connect(m_hintBtn, &QPushButton::clicked,
            this, &QuizRunnerDialog::onHint);      // NEW

}

// ---------- per-question ----------
void QuizRunnerDialog::showCurrentQuestion()
{
    m_hintLevel = 0;           // NEW
    m_hintLabel->clear();      // NEW
    m_hintBtn->setEnabled(true);

    const auto total = m_questions.size();
    m_progressLbl->setText(
        QString("Question %1 / %2  |  Score: %3")
            .arg(m_currIdx+1).arg(total).arg(m_score));

    // clear previous widgets
    QLayoutItem* item;
    while ((item = m_answerLayout->takeAt(0))) {
        delete item->widget();
        delete item;
    }
    m_radioBtns.clear();
    m_tfChecks.clear();
    m_shortEdit=nullptr;

    SessionQuestion& sq = m_questions[m_currIdx];
    m_questionLabel->setText(sq.q.text);

    switch (sq.q.type) {
    case QuestionType::TrueFalse: {
        auto* g = new QButtonGroup(this);
        for (int i=0;i<sq.answers.size();++i) {
            auto* c = new QRadioButton(sq.answers[i].text, m_answerWidget);
            g->addButton(c,i);
            m_answerLayout->addWidget(c);
            m_radioBtns<<c;
        }
        break;
    }
    case QuestionType::MultipleChoice: {
        auto* g = new QButtonGroup(this);
        for (int i=0;i<sq.answers.size();++i) {
            auto* c = new QRadioButton(sq.answers[i].text, m_answerWidget);
            g->addButton(c,i);
            m_answerLayout->addWidget(c);
            m_radioBtns<<c;
        }
        break;
    }
    case QuestionType::ShortAnswer: {
        m_shortEdit = new QLineEdit(m_answerWidget);
        m_answerLayout->addWidget(m_shortEdit);
        break;
    }
    }
    m_hintBtn->setVisible(sq.q.type != QuestionType::TrueFalse);
    m_submitBtn->setEnabled(true);
    m_nextBtn->setEnabled(false);
}

bool QuizRunnerDialog::checkAnswer(SessionQuestion& sq)
{
    bool correct = false;
    switch (sq.q.type) {
    case QuestionType::TrueFalse:
    case QuestionType::MultipleChoice: {
        int sel = -1;
        for (int i=0;i<m_radioBtns.size();++i)
            if (m_radioBtns[i]->isChecked()) { sel=i; break; }
        if (sel==-1){ QMessageBox::information(this,"Select","Choose one"); return false; }
        correct = sq.answers[sel].isCorrect;
        break;
    }
    case QuestionType::ShortAnswer: {
        if (!m_shortEdit){ return false; }
        const QString ans = m_shortEdit->text().trimmed();
        if (ans.isEmpty()){ QMessageBox::information(this,"Type","Enter answer"); return false; }
        correct = ans.compare(sq.answers.first().text, Qt::CaseInsensitive)==0;
        break;
    }
    }
    if (correct) ++m_score;
    return true;
}

void QuizRunnerDialog::onSubmit()
{
    SessionQuestion& sq = m_questions[m_currIdx];
    if (!checkAnswer(sq)) return;

    // disable inputs
    for (auto* r : m_radioBtns)  r->setEnabled(false);
    if (m_shortEdit) m_shortEdit->setEnabled(false);
    m_submitBtn->setEnabled(false);
    m_nextBtn->setEnabled(true);

    // color feedback
    QPalette okPal = palette();
    okPal.setColor(QPalette::WindowText, Qt::green);
    QPalette badPal = palette();
    badPal.setColor(QPalette::WindowText, Qt::red);

    switch (sq.q.type) {
    case QuestionType::MultipleChoice:
    case QuestionType::TrueFalse:
        for (int i=0;i<m_radioBtns.size();++i){
            if (sq.answers[i].isCorrect)
                m_radioBtns[i]->setPalette(okPal);
        }
        break;
    case QuestionType::ShortAnswer:
        if (m_shortEdit){
            m_shortEdit->setPalette(
                sq.answers.first().text.compare(m_shortEdit->text(),Qt::CaseInsensitive)==0
                    ? okPal : badPal);
        }
        break;
    }
}

void QuizRunnerDialog::onNext()
{
    ++m_currIdx;
    if (m_currIdx >= m_questions.size()) {
        // finished
        UserResult ur;
        ur.userId = m_user.id;
        ur.quizId = m_quizId;
        ur.total  = m_questions.size();
        ur.score  = m_score;
        ResultRepository::create(ur);

        QMessageBox::information(this,"Done",
                                 QString("Score %1 / %2").arg(m_score).arg(m_questions.size()));
        accept();
        return;
    }
    showCurrentQuestion();
}

void QuizRunnerDialog::onHint()            // NEW
{
    SessionQuestion& sq = m_questions[m_currIdx];

    switch (sq.q.type) {
    case QuestionType::ShortAnswer: {
        const QString ans = sq.answers.first().text;
        if (m_hintLevel >= ans.size()) {
            m_hintBtn->setEnabled(false);
            return;
        }
        ++m_hintLevel;

        QString hinted;
        for (int i = 0; i < ans.size(); ++i) {
            QChar ch = ans[i];
            if (ch.isSpace()) { hinted += ' '; continue; }
            hinted += (i < m_hintLevel ? ch : QChar('_'));
        }
        m_hintLabel->setText("Hint: " + hinted);

        int totalVisible = ans.size() - ans.count(' ');
        if (m_hintLevel >= totalVisible)
            m_hintBtn->setEnabled(false);
        break;
    }
    case QuestionType::MultipleChoice: {
        // 50-50: ostavi jedan tačan i jedan pogrešan
        if (m_radioBtns.size() <= 2) {
            m_hintBtn->setEnabled(false);
            return;
        }
        // prvo pronađi indeks tačnog
        int correctIdx = -1;
        for (int i = 0; i < sq.answers.size(); ++i)
            if (sq.answers[i].isCorrect) correctIdx = i;
        // nasumično ukloni pogrešne dok ne ostanu 2
        QList<int> pogresni;
        for (int i=0;i<sq.answers.size();++i)
            if (i != correctIdx) pogresni << i;
        std::shuffle(pogresni.begin(), pogresni.end(), *QRandomGenerator::global());
        while (pogresni.size() > 1) {
            int idx = pogresni.takeFirst();
            m_radioBtns[idx]->setEnabled(false);
        }
        m_hintBtn->setEnabled(false);   // samo jednom
        break;
    }
    default:
        m_hintBtn->setEnabled(false);   // True/False – hint besmislen
        break;
    }
}
