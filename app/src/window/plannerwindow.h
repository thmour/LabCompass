#ifndef PLANNERWINDOW_H
#define PLANNERWINDOW_H

#include "stdafx.h"
#include "window/window.h"

class PlannerWindow : public Window
{
  Q_OBJECT

signals:
  void moved(int x, int y);
  void importID(QString id);
  void importString(QString json);
  void setBlockedPath(QString fromRoom, QString toRoom, int conId);
  void setRoomIsTarget(QString id, bool target);
  void setCurrentRoom(QString id);

public:
  PlannerWindow(QQmlEngine* engine);

private slots:
  void onWindowOpenChanged();
  void onRequestFinished(QNetworkReply* reply);
  void onImportLabNotesButtonClicked(const QString& labType);
  void onOpenUrl(const QString& url);
  void onDrag(int dx, int dy);

private:
  QNetworkAccessManager* qnam;
  QNetworkRequest* req;
  QSet<QString> labs;
};

#endif // PLANNERWINDOW_H
