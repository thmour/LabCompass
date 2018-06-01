#include "labyrinthcontroller.h"

LabyrinthController::LabyrinthController(ApplicationModel* model)
{
  this->model = model;
}

void LabyrinthController::importID(const QString& id)
{
  model->loadFromCache(id);
}

void LabyrinthController::importString(const QString& json)
{
  model->loadFromString(json);
}
