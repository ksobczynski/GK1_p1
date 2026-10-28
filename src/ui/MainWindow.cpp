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
  toolbar->addWidget(new QLabel("B-spline:"));
  auto *bsplineOff = new QRadioButton("Wyłączone");
  auto *bsplineCurve = new QRadioButton("Pokaż B-spline");
  auto *bsplineHelpers = new QRadioButton("Pokaż pomocnicze wierzchołki");

  bsplineOff->setChecked(true);
  toolbar->addWidget(bsplineOff);
  toolbar->addWidget(bsplineCurve);
  toolbar->addWidget(bsplineHelpers);

  auto *bsplineGroup = new QButtonGroup(this);
  bsplineGroup->addButton(bsplineOff);
  bsplineGroup->addButton(bsplineCurve);
  bsplineGroup->addButton(bsplineHelpers);

  const std::pair<QRadioButton *, CanvasWidget::BSplineMode> bsplineModes[] = {
      {bsplineOff, CanvasWidget::BSplineMode::Off},
      {bsplineCurve, CanvasWidget::BSplineMode::Curve},
      {bsplineHelpers, CanvasWidget::BSplineMode::Helpers}};
  for (const auto &[button, mode] : bsplineModes)
    connect(button, &QRadioButton::toggled, this,
            [this, mode = mode](bool checked) {
              if (checked)
                canvas_->setBSplineMode(mode);
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