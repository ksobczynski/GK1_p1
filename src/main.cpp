#include <QApplication>
#include <QPalette>
#include <QWidget>


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QWidget canvas;
    canvas.setWindowTitle("Edytor wielokatow");
    canvas.resize(900,600);

    QPalette palette = canvas.palette();
    palette.setColor(QPalette::Window, Qt::white);
    canvas.setPalette(palette);
    canvas.setAutoFillBackground(true);

    canvas.show();

    return app.exec();

}
