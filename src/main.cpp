#include "model/Scene.h"
#include "ui/CanvasWidget.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    model::Scene scene = model::Scene::createDemo();
    CanvasWidget canvas(scene);
    canvas.show();
    return app.exec();
}
