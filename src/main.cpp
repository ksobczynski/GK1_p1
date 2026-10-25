#include "model/Scene.h"
#include "ui/CanvasWidget.h"
#include "ui/MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  model::Scene scene = model::Scene::createDemo();
  MainWindow window(scene);
  window.show();
  return app.exec();
}
