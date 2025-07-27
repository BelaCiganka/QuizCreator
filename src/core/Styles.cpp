#include "core/Styles.hpp"

#include <QFile>
#include <QWidget>
#include <QDebug>

void Styles::applyGlobalStyle(QWidget* root) {
    QFile f(":/styles/modern.qss");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        root->setStyleSheet(QString::fromUtf8(f.readAll()));
    } else {
        qWarning() << "Failed to load QSS!";
    }
}
