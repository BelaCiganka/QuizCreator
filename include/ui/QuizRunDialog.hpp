#pragma once

#include "models/User.hpp"

#include <QDialog>

class QuizRunDialog : public QDialog {
    Q_OBJECT
public:
    explicit QuizRunDialog(const User& user, QWidget* parent = nullptr);

private:
    User m_user;
};
