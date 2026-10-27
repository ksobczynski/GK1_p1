#include "HelpDialog.h"

#include <QFontDatabase>
#include <QTextBrowser>
#include <QUrl>
#include <QVBoxLayout>

HelpDialog::HelpDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Dokumentacja");
  resize(820, 720);

  auto *browser = new QTextBrowser(this);
  const QString fixedFont =
      QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
  browser->document()->setDefaultStyleSheet(
      QString("code { font-family: \"%1\"; }").arg(fixedFont));
  browser->setSource(QUrl("qrc:/docs/help.html"));

  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(browser);
}
