#ifndef CONNECTIONMODEL_H
#define CONNECTIONMODEL_H

#include "stdafx.h"
#include "model/listmodel.h"
#include "model/labyrinthdata.h"
#include "model/navigationdata.h"

struct ConnectionModelData
{
  QString fromRoom;
  QString toRoom;
  QPoint fromCoordinate;
  QPoint toCoordinate;
  bool isSecretPassage {false};
  bool isPlanned {false};
  bool isBlocked {false};
};

class ConnectionModel : public ListModel
{
  Q_OBJECT

  QList<ConnectionModelData> d;
  enum Roles {
    FromCoordinateRole = Qt::UserRole + 1,
    ToCoordinateRole,
    IsSecretPassgeRole,
    IsPlannedRole,
    IsBlockedRole,
    FromRoomRole,
    ToRoomRole,
  };

public:
  ConnectionModel();
  void loadFromData(const LabyrinthData& data, const NavigationData& navigation);
  void updateFromData(const NavigationData& data);
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  void updateBlocked(int i, bool blocked);

protected:
  QHash<int, QByteArray> roleNames() const;
};

#endif // CONNECTIONMODEL_H
