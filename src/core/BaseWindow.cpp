#include "core/BaseWindow.hpp"
#include "core/Styles.hpp"

BaseWindow::BaseWindow(QWidget* parent)
    : QMainWindow(parent) {
    applyStyleSheet();
}

void BaseWindow::applyStyleSheet() {
    Styles::applyGlobalStyle(this);
}
