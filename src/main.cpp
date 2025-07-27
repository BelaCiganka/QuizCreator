#include "app/Application.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication qtApp(argc, argv);

    Application app;  // Our layer controller

    return app.run();
}
