#include "ui/QuizSelectDialog.hpp"
#include "repositories/QuizRepository.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>

QuizSelectDialog::QuizSelectDialog(QWidget* parent)
    : QDialog(parent)
{

    setWindowTitle("Select Quiz");
    resize(400, 200);

    auto* v = new QVBoxLayout(this);
    m_quizBox = new QComboBox(this);
    m_modeBox = new QComboBox(this);
    m_dirBox  = new QComboBox(this);
    m_startBtn= new QPushButton("Start", this);

    // quizzes
    const auto quizzes = QuizRepository::all();
    for (const auto& q : quizzes)
        m_quizBox->addItem(q.title, q.id);

    // modes
    m_modeBox->addItem("Mixed",           static_cast<int>(Mode::Mixed));
    m_modeBox->addItem("Multiple Choice", static_cast<int>(Mode::MultipleChoice));
    m_modeBox->addItem("True / False",    static_cast<int>(Mode::TrueFalse));
    m_modeBox->addItem("Short Answer", static_cast<int>(Mode::ShortAnswer));

    m_dirBox->addItem("→ prevod",   int(Direction::Normal));   // srpski → eng
    m_dirBox->addItem("← prevod",   int(Direction::Reverse));  // eng → srp

    auto* f1 = new QHBoxLayout();  f1->addWidget(new QLabel("Quiz:"));  f1->addWidget(m_quizBox,1);
    auto* f2 = new QHBoxLayout();  f2->addWidget(new QLabel("Mode:"));  f2->addWidget(m_modeBox,1);
    auto *f3 = new QHBoxLayout();  f3->addWidget(new QLabel("Direction:")); f3->addWidget(m_dirBox,1);

    v->addLayout(f1);
    v->addLayout(f2);
    v->addLayout(f3);
    v->addStretch();
    v->addWidget(m_startBtn);

    connect(m_startBtn,&QPushButton::clicked,this,&QuizSelectDialog::onStart);
}

void QuizSelectDialog::onStart()
{
    if (m_quizBox->currentIndex()<0) {
        QMessageBox::warning(this,"Select","Choose a quiz."); return;
    }
    m_quizId = m_quizBox->currentData().toInt();
    m_mode   = static_cast<Mode>(m_modeBox->currentData().toInt());
    m_dir = Direction( m_dirBox->currentData().toInt() );
    accept();
}
