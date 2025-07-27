#pragma once

#include <QMainWindow>

class BaseWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit BaseWindow(QWidget* parent = nullptr);
    virtual ~BaseWindow() = default;

protected:
    void applyStyleSheet();  // loads QSS from resources
};
