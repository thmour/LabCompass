#ifndef NAVIGATIONCONTROLLER_H
#define NAVIGATIONCONTROLLER_H

#include "stdafx.h"
#include "model/applicationmodel.h"

class NavigationController : public QObject
{
  Q_OBJECT

  ApplicationModel* model;

public:
  NavigationController(ApplicationModel* model);

public slots:
  void onLabStarted();
  void onSectionFinished();
  void onLabExit();
  void onRoomChanged(const QString& name);
  void onPortalSpawned();
  void onBlockedPath(QString fromRoom, QString toRoom);
  void onRoomIsTargetSet(const QString& id, bool isTarget);
  void onRoomIdSet(const QString& id);

private:
  bool dfs(const QString&, const QString&, QHash<QString, bool>&, QString);
};

#endif // NAVIGATIONCONTROLLER_H
