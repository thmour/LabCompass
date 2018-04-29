#include "plannerwindow.h"
#include "settings.h"

PlannerWindow::PlannerWindow(QQmlEngine* engine) : Window(engine, false)
{
  setSource(QUrl("qrc:/ui/planner/Planner.qml"));

  qnam = new QNetworkAccessManager();
  req = new QNetworkRequest();
  QObject::connect(qnam, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(onRequestFinished(QNetworkReply*)));

  connect(rootObject(), SIGNAL(drag(int, int)),
          this, SLOT(onDrag(int, int)));
  connect(global(), SIGNAL(plannerWindowOpenChanged()),
          this, SLOT(onWindowOpenChanged()));
  connect(rootObject(), SIGNAL(importLabNotesButtonClicked(QString)),
          this, SLOT(onImportLabNotesButtonClicked(QString)));
  connect(rootObject(), SIGNAL(openUrl(QString)),
          this, SLOT(onOpenUrl(QString)));

  connect(rootObject()->findChild<QObject*>("labyrinthMapDisplay"), SIGNAL(setRoomIsTarget(QString, bool)),
          this, SIGNAL(setRoomIsTarget(QString, bool)));
    connect(rootObject()->findChild<QObject*>("labyrinthMapDisplay"), SIGNAL(setBlockedPath(QString, QString)),
          this, SIGNAL(setBlockedPath(QString,QString)));
  connect(rootObject()->findChild<QObject*>("labyrinthMapDisplay"), SIGNAL(setCurrentRoom(QString)),
          this, SIGNAL(setCurrentRoom(QString)));
}

void PlannerWindow::onWindowOpenChanged()
{
  bool open = global()->property("plannerWindowOpen").toBool();
  setVisible(open);
}

void PlannerWindow::onImportLabNotesButtonClicked(const QString &labType)
{
  QString id = labType + "-" + QDate::currentDate().toString("yyyy-MM-dd");
  if (labs.contains(id)) {
    emit importID(id);
  } else {
    QUrl labUrl("https://www.poelab.com/wp-content/labfiles/" + id + ".json");
    labUrl.setQuery(id);
    req->setUrl(labUrl);
    qnam->get(*req);
  }
}

void PlannerWindow::onRequestFinished(QNetworkReply *reply) {
  if (reply->error()) {
    return;
  }
  QString id = reply->request().url().query();
  labs.insert(id);
  emit importString(reply->readAll());
}

void PlannerWindow::onOpenUrl(const QString& url)
{
  global()->setProperty("plannerWindowOpen", false);
  QDesktopServices::openUrl(QUrl(url));
}

void PlannerWindow::onDrag(int dx, int dy)
{
  move(x() + dx, y() + dy);
  emit moved(x(), y());
}
