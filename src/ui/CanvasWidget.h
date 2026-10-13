#pragma once

#include "model/Scene.h"
#include <QWidget>

class CanvasWidget : public QWidget {
public:
    explicit CanvasWidget(model::Scene& scene, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    model::Scene& scene_;
};
