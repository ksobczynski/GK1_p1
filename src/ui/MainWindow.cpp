#include "MainWindow.h"
#include "CanvasWidget.h"
#include "HelpDialog.h"
#include "model/Scene.h"
#include "render/LineDrawing.h"

#include <QButtonGroup>
#include <QLabel>
#include <QRadioButton>
#include <QToolBar>

MainWindow::MainWindow(model::Scene &scene, QWidget *parent)
    : QMainWindow(parent), canvas_(new CanvasWidget(scene, this)) {
  setWindowTitle("Edytor wielokątów");
  resize(1200, 900);
  setCentralWidget(canvas_);

  QToolBar *toolbar = addToolBar("Rysowanie");
  toolbar->addWidget(new QLabel("Odcinki:"));
  auto *library = new QRadioButton("Biblioteczny");
  auto *bresenham = new QRadioButton("Bresenham");

  library->setChecked(true);
  toolbar->addWidget(library);
  toolbar->addWidget(bresenham);

  auto *group = new QButtonGroup(this);
  group->addButton(library);
  group->addButton(bresenham);

  connect(bresenham, &QRadioButton::toggled, this, [this](bool checked) {
    canvas_->setLineAlgorithm(checked ? render::LineAlgorithm::Bresenham
                                      : render::LineAlgorithm::Library);
  });

  toolbar->addSeparator();
  QAction *helpAction = toolbar->addAction("Pomoc (F1)");
  helpAction->setShortcut(Qt::Key_F1);
  connect(helpAction, &QAction::triggered, this, &MainWindow::showHelp);
}

void MainWindow::showHelp() {
  if (!help_)
    help_ = new HelpDialog(this);
  help_->show();
  help_->raise();
  help_->activateWindow();
}