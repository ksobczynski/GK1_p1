#pragma once

#include "model/Scene.h"
#include "ui/CanvasWidget.h"
#include <QMainWindow>

class CanvasWidget;
class HelpDialog;

class MainWindow : public QMainWindow {
public:
  explicit MainWindow(model::Scene &scene, QWidget *parent = nullptr);

private:
  void showHelp();

  CanvasWidget *canvas_;
  HelpDialog *help_ = nullptr;
};