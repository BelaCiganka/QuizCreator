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
#include <QEvent>
#include <QKeyEvent>
#include <QScrollArea>

QuizRunnerDialog::QuizRunnerDialog(const User& user,
                                   int quizId,
                                   QuizSelectDialog::Mode mode,
                                   QuizSelectDialog::Direction dir,
                                   QWidget* parent)
    : QDialog(parent)
    , m_user(user)
    , m_quizId(quizId)
    , m_mode(mode)
    , m_dir(dir)
{
    loadLocalStyle();
    setWindowTitle("Quiz in progress");
    resize(700,500);

    loadQuestions();
    if (m_questions.isEmpty()) {
        QMessageBox::information(this,"Empty","No questions in quiz.");
        reject(); return;
    }
    setupUI();
    installEventFilter(this);
    showCurrentQuestion();
}


void QuizRunnerDialog::loadQuestions()
{
    auto qs = QuestionRepository::byQuiz(m_quizId);
    std::shuffle(qs.begin(), qs.end(), *QRandomGenerator::global());

    for (const Question &q : qs)
    {
        /* 1) filter po modu */
        if (m_mode == QuizSelectDialog::Mode::TrueFalse && q.type != QuestionType::TrueFalse)
            continue;
        if (m_mode == QuizSelectDialog::Mode::MultipleChoice && q.type == QuestionType::TrueFalse)
            continue;
        if (m_mode == QuizSelectDialog::Mode::ShortAnswer && q.type != QuestionType::ShortAnswer)
            continue;

        /* 2) SessionQuestion */
        SessionQuestion sq;
        sq.q       = q;                                  // OK: sq.q
        sq.answers = AnswerRepository::byQuestion(q.id);
        if (sq.answers.isEmpty())
            continue;

        /* smer – Reverse: zameni tekst pitanja i tačan odgovor */
        if (m_dir == QuizSelectDialog::Direction::Reverse &&
            q.type == QuestionType::ShortAnswer)
        {
            QString origAnswer = sq.answers.first().text; // prevod
            sq.answers.first().text = q.text;             // tačan = srpska reč
            sq.q.text = origAnswer;                       // pitanje = prevod
        }

        /* minimalna validacija */
        switch (sq.q.type) {
        case QuestionType::TrueFalse:
            if (sq.answers.size() != 2) continue;
            break;
        case QuestionType::MultipleChoice:
            if (sq.answers.size() < 2)  continue;
            break;
        case QuestionType::ShortAnswer:
            break;
        }

        /* 3) Konverzija SA → MC (Mixed ili MC mod) */
        if ((m_mode == QuizSelectDialog::Mode::Mixed || m_mode == QuizSelectDialog::Mode::MultipleChoice) &&
            q.type  == QuestionType::ShortAnswer)
        {
            QString correct = sq.answers.first().text;

            /* prikupi kandidatske distractore */
            QList<QString> pool;
            for (const Question &cand : qs) {
                if (cand.type != QuestionType::ShortAnswer || cand.id == q.id)
                    continue;
                auto ansList = AnswerRepository::byQuestion(cand.id);
                if (ansList.isEmpty()) continue;

                /*  ovde JE bila greška: cand.q.text –› cand.text  */
                QString candidate = (m_dir == QuizSelectDialog::Direction::Reverse)
                                        ? cand.text                // sr reč
                                        : ansList.first().text;    // eng prevod
                if (candidate != correct)
                    pool << candidate;
            }
            std::shuffle(pool.begin(), pool.end(), *QRandomGenerator::global());
            while (pool.size() < 3) pool << "???";

            QList<Answer> mc;
            mc << Answer{ .text = correct, .isCorrect = true };
            for (int i = 0; i < 3; ++i)
                mc << Answer{ .text = pool[i], .isCorrect = false };
            std::shuffle(mc.begin(), mc.end(), *QRandomGenerator::global());

            sq.q.type   = QuestionType::MultipleChoice;
            sq.answers  = mc;
        }

        m_questions << sq;
    }
}



// ---------- UI ----------
void QuizRunnerDialog::setupUI()
{
    /* =====  ROOT (card-layout)  ===== */
    auto *root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0,0,0,0);

    /* ———  Card container  ——— */
    auto *card = new QFrame(this);
    card->setObjectName("quizCard");
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *cv = new QVBoxLayout(card);
    cv->setContentsMargins(32,24,32,24);
    cv->setSpacing(18);
    root->addWidget(card);

    /* ── 1. Header: progress bar + timer ──────────────────── */
    auto *hdr = new QHBoxLayout();
    hdr->setSpacing(12);

    m_progBar = new QProgressBar(card);
    m_progBar->setRange(0, m_questions.size());
    m_progBar->setValue(1);
    m_progBar->setFixedHeight(14);
    m_progBar->setTextVisible(false);

    m_timerLbl = new QLabel("00:00", card);
    m_timerLbl->setObjectName("timerLabel");

    hdr->addWidget(m_progBar, /*stretch*/4);
    hdr->addWidget(m_timerLbl);
    cv->addLayout(hdr);

    /* ── 2. Brojač pitanja ───────────────────────────────── */
    m_progressLbl = new QLabel(card);                 // “Q 1 / 10   |   0 / 10”
    m_progressLbl->setObjectName("statusLabel");
    cv->addWidget(m_progressLbl);

    /* ── 3. Tekst pitanja ────────────────────────────────── */
    m_questionLabel = new QLabel(card);
    m_questionLabel->setObjectName("questionText");
    m_questionLabel->setWordWrap(true);
    cv->addWidget(m_questionLabel);

    /* ── 4. Hint label ───────────────────────────────────── */
    m_hintLabel = new QLabel(card);
    m_hintLabel->setObjectName("hintLabel");
    cv->addWidget(m_hintLabel);

    /* ── 5. Scroll-zona s ponudama ───────────────────────── */
    auto *scroll = new QScrollArea(card);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_answerWidget = new QWidget(scroll);
    m_answerLayout = new QVBoxLayout(m_answerWidget);
    m_answerLayout->setSpacing(12);
    m_answerLayout->setContentsMargins(4,4,4,4);
    scroll->setWidget(m_answerWidget);

    cv->addWidget(scroll, /*stretch*/3);

    /* ── 6. Footer bar ───────────────────────────────────── */
    auto *ftr = new QHBoxLayout();
    ftr->setSpacing(20);

    m_hintBtn   = new QPushButton(card);
    m_hintBtn->setText("Hint  (H)");
    m_hintBtn->setObjectName("ghostBtn");

    m_submitBtn = new QPushButton("Submit  (Space)", card);
    m_submitBtn->setObjectName("primaryBtn");

    m_nextBtn   = new QPushButton("Next  (Space)", card);
    m_nextBtn->setObjectName("primaryBtn");
    m_nextBtn->setEnabled(false);

    ftr->addWidget(m_hintBtn);
    ftr->addStretch();
    ftr->addWidget(m_submitBtn);
    ftr->addWidget(m_nextBtn);
    cv->addLayout(ftr);

    /* ── signali ─────────────────────────────────────────── */
    connect(m_submitBtn,&QPushButton::clicked,this,&QuizRunnerDialog::onSubmit);
    connect(m_nextBtn,  &QPushButton::clicked,this,&QuizRunnerDialog::onNext);
    connect(m_hintBtn,  &QPushButton::clicked,this,&QuizRunnerDialog::onHint);

    /* ———  Tajmer ——— */
    m_timer.setInterval(1000);
    connect(&m_timer,&QTimer::timeout,this,[this]{
        m_elapsedMs += 1000;
        m_timerLbl->setText(QString::asprintf("%02d:%02d",
                                              (m_elapsedMs/1000)/60, (m_elapsedMs/1000)%60));
    });
    m_timer.start();
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
    m_progBar->setValue(m_currIdx);
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
        m_timer.stop();
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

bool QuizRunnerDialog::eventFilter(QObject*, QEvent* ev)
{
    if (ev->type() == QEvent::KeyPress) {
        auto* kev = static_cast<QKeyEvent*>(ev);
        if (kev->key() == Qt::Key_Space) {
            if (m_submitBtn->isEnabled())  onSubmit();
            else if (m_nextBtn->isEnabled()) onNext();
            return true;       // pojeli smo ga
        }
    }
    return false;
}

void QuizRunnerDialog::loadLocalStyle()
{
    static const char *qss = R"(
    /* card background */
    #quizCard { background:#1e1e1e; border-radius:20px; }

    /* timer */
    #timerLabel { color:#9cdcfe; font-weight:600; }

    /* status */
    #statusLabel { color:#bbbbbb; font-size:13px; }

    /* question text */
    #questionText { font-size:20px; font-weight:600; color:#f2f2f2; }

    /* hint */
    #hintLabel { color:#ffb454; font-style:italic; }

    /* progress bar */
    QProgressBar { background:#2c2c2c; border:none; border-radius:7px; }
    QProgressBar::chunk { background:#4e8cff; border-radius:7px; }

    /* buttons */
    QPushButton { padding:6px 18px; border-radius:14px; }
    QPushButton#primaryBtn {
        background:#4e8cff; color:#fff; border:none;
    }
    QPushButton#primaryBtn:hover { background:#639bff; }
    QPushButton#primaryBtn:pressed { background:#3a6fe6; }
    QPushButton#ghostBtn {
        background:transparent; color:#cccccc;
    }
    QPushButton#ghostBtn:hover { color:#ffffff; }
    )";
    this->setStyleSheet(qss);
}
