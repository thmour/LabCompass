#ifndef LABYRINTHCONTROLLER_H
#define LABYRINTHCONTROLLER_H

#include "stdafx.h"
#include "model/applicationmodel.h"

class LabyrinthController : public QObject
{
  Q_OBJECT

  ApplicationModel* model;

public:
  LabyrinthController(ApplicationModel* model);

public slots:
  void importID(const QString& id);
  void importString(const QString& json);
};

#endif // LABYRINTHCONTROLLER_H
