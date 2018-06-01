#include "applicationmodel.h"

ApplicationModel::ApplicationModel(QObject* parent) : QObject(parent)
{
  update_settings(new Settings());
  get_settings()->load();

  m_labyrinthModel.reset(new LabyrinthModel);
  m_roomModel.reset(new RoomModel);
  m_connectionModel.reset(new ConnectionModel);
  m_plannedRouteModel.reset(new PlannedRouteModel);
  m_goldenDoorModel.reset(new GoldenDoorModel);
  m_instructionModel.reset(new InstructionModel);
  m_planSummaryModel.reset(new PlanSummaryModel);

  m_newVersionAvailable = false;
  m_labMapOutdated = true;
  m_logFileOpen = true;
  m_atPlaza = false;
  m_inLab = false;
  m_currentRoomDetermined = true;
  m_isValid = false;
}

void ApplicationModel::saveLabyrinthData() {
    if (labyrinthData.id.isNull()) return;

    QString id = labyrinthData.id;
    cachedLabyrinthData[id] = labyrinthData;
    cachedPlanData[id] = planData;
    cachedNavigationData[id] = navigationData;
}

bool ApplicationModel::loadFromString(const QString& json)
{
  LabyrinthData newLabyrinthData;
  if (newLabyrinthData.loadFromString(json.toUtf8())) {
    saveLabyrinthData();
    labyrinthData = newLabyrinthData;
    update_labMapOutdated(labyrinthData.date != QDateTime::currentDateTimeUtc().date());
    updatePlanData(PlanData());

    update_inLab(false);
    update_isValid(true);
    resetModels();

    return true;
  }
  return false;
}

bool ApplicationModel::loadFromCache(const QString& id) {
  if (id == labyrinthData.id) return false;
  saveLabyrinthData();
  update_labMapOutdated(labyrinthData.date != QDateTime::currentDateTimeUtc().date());
  labyrinthData = cachedLabyrinthData[id];
  planData = cachedPlanData[id];
  navigationData = cachedNavigationData[id];

  update_inLab(false);
  update_isValid(true);
  resetModels();
  return true;
}

void ApplicationModel::updatePlanData(const PlanData& planData)
{
  this->planData = planData;
  NavigationData navigationData;
  navigationData.loadFromData(&labyrinthData, planData);
  navigationData.updatePlannedRouteAndInstructions();

  get_planSummaryModel()->loadFromData(navigationData);
  updateNavigationData(navigationData);
}

void ApplicationModel::updateNavigationData(const NavigationData& navigationData)
{
  this->navigationData = navigationData;
  update_currentRoomDetermined(navigationData.currentRoomDetermined);
  updateModels();
}

void ApplicationModel::resetModels()
{
  get_labyrinthModel()->loadFromData(labyrinthData);
  get_roomModel()->loadFromData(labyrinthData, navigationData);
  get_connectionModel()->loadFromData(labyrinthData, navigationData);
  get_goldenDoorModel()->loadFromData(labyrinthData);
  get_plannedRouteModel()->loadFromData(navigationData);
  get_instructionModel()->loadFromData(navigationData);
}

void ApplicationModel::updateModels()
{
  get_roomModel()->updateFromData(navigationData);
  get_connectionModel()->updateFromData(navigationData);
  get_plannedRouteModel()->loadFromData(navigationData);
  get_instructionModel()->loadFromData(navigationData);
}
