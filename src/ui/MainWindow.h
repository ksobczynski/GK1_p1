#pragma once

#include "model/Scene.h"
#include "ui/CanvasWidget.h"
#include <QMainWindow>

class CanvasWidget;

class MainWindow : public QMainWindow {
public:
  explicit MainWindow(model::Scene &scene, QWidget *parent = nullptr);

private:
  CanvasWidget *canvas_;
};