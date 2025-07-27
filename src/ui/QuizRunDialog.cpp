#include "ui/QuizRunDialog.hpp"

#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

QuizRunDialog::QuizRunDialog(const User& user, QWidget* parent)
    : QDialog(parent)
    , m_user(user) {

    setWindowTitle("Quiz - In Progress");
    resize(600, 400);

    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Ovde ce ici prikaz pitanja/odgovora...", this));

    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QuizRunDialog::reject);

    layout->addWidget(btnBox);
}
