#include "navigationcontroller.h"

NavigationController::NavigationController(ApplicationModel* model)
{
  this->model = model;
}

void NavigationController::onPlazaEntered()
{
  model->update_atPlaza(true);
}

void NavigationController::onLabStarted()
{
  if (!model->get_isValid())
    return;

  NavigationData navigation;
  navigation.loadFromData(&model->labyrinthData, model->planData);
  navigation.updatePlannedRouteAndInstructions();
  model->updateNavigationData(navigation);
  model->update_inLab(true);
  model->update_atPlaza(true);
  model->get_instructionModel()->update_finishedSections(0);
}

void NavigationController::onSectionFinished()
{
  if (!model->get_isValid())
    return;

  int finishedSections = model->get_instructionModel()->get_finishedSections();
  model->get_instructionModel()->update_finishedSections(finishedSections + 1);
}

void NavigationController::onLabExit()
{
  if (!model->get_isValid())
    return;

  NavigationData navigation;
  navigation.loadFromData(&model->labyrinthData, model->planData);
  navigation.updatePlannedRouteAndInstructions();
  model->updateNavigationData(navigation);
  model->update_atPlaza(false);
  model->update_inLab(false);
}

void NavigationController::onRoomChanged(const QString& name)
{
  if (!model->get_isValid())
    return;

  NavigationData data = model->navigationData;
  if (data.currentRoomDetermined)
    data.previousRoom = data.currentRoom;

  // the user follows the planned route
  if (data.currentRoomDetermined && data.plannedRoute.size() >= 2 &&
      model->labyrinthData.getRoomFromId(data.plannedRoute[1]).name == name) {
    data.possibleCurrentRooms = {data.plannedRoute[1]};

  // the user takes a portal to the next trial room
  } else if (data.currentRoomDetermined &&
             data.contentState.portalLocations.contains(data.currentRoom) &&
             name == "Aspirant\'s Trial") {
    int currentSection = data.lab->getRoomFromId(data.currentRoom).section;
    data.possibleCurrentRooms = {data.lab->sections[currentSection].trialRoom};

    // remove all targets in the current section
    if (model->get_settings()->get_portalSkipsSection()) {
      foreach (const auto& room, data.lab->sections[currentSection].roomIds)
        data.targetRooms.removeAll(room);
    }

  } else {
    QSet<QString> connectedRooms;
    foreach (const auto& current, data.possibleCurrentRooms) {
      foreach (const auto& room, model->labyrinthData.rooms)
        if (model->labyrinthData.hasConnection(current, room.id))
          connectedRooms.insert(room.id);
    }

    data.possibleCurrentRooms = QSet<QString>();
    for (auto i = connectedRooms.constBegin(); i != connectedRooms.constEnd(); i++)
      if (model->labyrinthData.getRoomFromId(*i).name == name)
        data.possibleCurrentRooms.insert(*i);
  }

  if (data.possibleCurrentRooms.size() == 1) {
    data.currentRoom = *data.possibleCurrentRooms.constBegin();
    data.currentRoomDetermined = true;
  } else {
    data.currentRoom = "";
    data.currentRoomDetermined = false;
  }

  if (data.currentRoomDetermined) {
    // unlock golden doors
    if (data.contentState.lockedDoors.contains(std::pair<QString, QString>(data.currentRoom, data.previousRoom)) ||
        data.contentState.lockedDoors.contains(std::pair<QString, QString>(data.previousRoom, data.currentRoom))) {
      data.contentState.goldenKeysInInventory -= 1;
      data.contentState.lockedDoors.removeAll(std::pair<QString, QString>(data.currentRoom, data.previousRoom));
      data.contentState.lockedDoors.removeAll(std::pair<QString, QString>(data.previousRoom, data.currentRoom));
    }

    // loot golden keys
    if (data.contentState.goldenKeyLocations.contains(data.currentRoom)) {
      data.contentState.goldenKeyLocations.removeAll(data.currentRoom);
      data.contentState.goldenKeysInInventory += 1;
    }

    // remove target
    data.targetRooms.removeAll(data.currentRoom);
  }

  data.updatePlannedRouteAndInstructions();
  model->updateNavigationData(data);
  model->update_atPlaza(false);
}

void NavigationController::onPortalSpawned()
{
  if (!model->get_isValid())
    return;

  NavigationData data = model->navigationData;
  if (data.currentRoomDetermined)
    data.contentState.portalLocations.append(data.currentRoom);

  model->updateNavigationData(data);
}

bool NavigationController::dfs(const QString& currRoom,
                               const QString& targetRoom,
                               QHash<QString, bool>& visited) {
  auto& ld = model->labyrinthData;
  auto& con = ld.connections;
  auto& bp = ld.blockedPaths;
  if (currRoom == targetRoom) {
    return true;
  } else {
    bool found = false;
    visited[currRoom] = true;
    for (auto it = con.begin(); !found && it != con.end(); ++it) {
      auto next_room = it.key();
      if (!visited[next_room]                   &&
          ld.hasConnection(currRoom, next_room) &&
          !bp[currRoom].contains(next_room)) {
        found = dfs(next_room, targetRoom, visited);
      }
    }
    return found;
  }
}

void NavigationController::onBlockedPath(QString fromRoom, QString toRoom, int conId)
{
  if (!model->get_isValid()) {
    return;
  }

  auto& bp = model->labyrinthData.blockedPaths;
  if (bp[fromRoom].contains(toRoom)) {
    bp[fromRoom].removeAt(bp[fromRoom].indexOf(toRoom));
    bp[toRoom].removeAt(bp[toRoom].indexOf(fromRoom));
    model->get_connectionModel()->updateBlocked(conId, false);
  } else {
    QList<QString> rooms = model->labyrinthData.connections.keys();
    QString endRoom = *std::max_element(rooms.begin(), rooms.end(), [](auto a, auto b) {
      return a.toInt() < b.toInt();
    });
    QHash<QString, bool> visited;
    for(auto& room : rooms) {
      visited[room] = false;
    }
    bp[fromRoom].append(toRoom);
    bp[toRoom].append(fromRoom);
    if (!dfs(fromRoom, endRoom, visited)) {
      bp[fromRoom].removeAt(bp[fromRoom].indexOf(toRoom));
      bp[toRoom].removeAt(bp[toRoom].indexOf(fromRoom));
    } else {
      model->get_connectionModel()->updateBlocked(conId, true);
    }
  }

  if (model->get_inLab()) {
    NavigationData data = model->navigationData;
    data.updatePlannedRouteAndInstructions();
    model->updateNavigationData(data);
  } else {
    model->updatePlanData(model->planData);
  }
}

void NavigationController::onRoomIsTargetSet(const QString& id, bool isTarget)
{
  if (!model->get_isValid())
    return;

  if (model->get_inLab()) {
    NavigationData data = model->navigationData;
    data.targetRooms.removeAll(id);
    if (isTarget)
      data.targetRooms.append(id);
    data.updatePlannedRouteAndInstructions();
    model->updateNavigationData(data);
  } else {
    PlanData data = model->planData;
    data.targetRooms.removeAll(id);
    if (isTarget)
      data.targetRooms.append(id);
    model->updatePlanData(data);
  }
}

void NavigationController::onRoomIdSet(const QString& id)
{
  if (!model->get_isValid() || !model->get_inLab())
    return;

  NavigationData data = model->navigationData;
  data.currentRoomDetermined = true;
  data.currentRoom = id;
  data.updatePlannedRouteAndInstructions();
  model->updateNavigationData(data);
  model->update_atPlaza(false);
}
