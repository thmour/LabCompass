import QtQuick 2.8
import QtGraphicalEffects 1.0
import com.labcompass 1.0
import '..'

Rectangle {
  id: root

  signal setRoomIsTarget(string id, bool target)
  signal setCurrentRoom(string id)
  signal setBlockedPath(string fromRoom, string toRoom, int conId)

  property var roomModel
  property var connectionModel
  property var goldenDoorModel

  color: Global.primaryColor

  Canvas {
    id: connectionView
    anchors.fill: parent
    onPaint: {
      var ctx = connectionView.getContext('2d');
      ctx.clearRect(0, 0, width, height);

      for (var i = 0; i < connectionModel.size(); i++) {
        var connection = connectionModel.get(i);

        ctx.lineWidth = connection.isSecretPassage ? 2 : 4;
        if(connection.isPlanned) {
            ctx.strokeStyle = Global.activePathColor;
        } else if (connection.isBlocked) {
            ctx.strokeStyle = Global.blockedPathColor;
        } else {
            ctx.strokeStyle = Global.inactivePathColor;
        }

        ctx.beginPath();
        ctx.moveTo(connection.fromCoordinate.x, connection.fromCoordinate.y);
        ctx.lineTo(connection.toCoordinate.x, connection.toCoordinate.y);
        ctx.stroke();
      }
    }
    MouseArea {
      id: mouseArea
      anchors.fill: parent
      onClicked: {
        function pDistance(x, y, x1, y1, x2, y2) {
          var A = x - x1;
          var B = y - y1;
          var C = x2 - x1;
          var D = y2 - y1;

          var dot = A * C + B * D;
          var len_sq = C * C + D * D;
          var param = -1;
          if (len_sq != 0) //in case of 0 length line
              param = dot / len_sq;

          var xx, yy;

          if (param < 0) {
            xx = x1;
            yy = y1;
          }
          else if (param > 1) {
            xx = x2;
            yy = y2;
          }
          else {
            xx = x1 + param * C;
            yy = y1 + param * D;
          }

          var dx = x - xx;
          var dy = y - yy;
          return Math.sqrt(dx * dx + dy * dy);
        }
        for (var i = 1; i < connectionModel.size() -1; i++) {
          var connection = connectionModel.get(i);
          var x1 = connection.fromCoordinate.x;
          var x2 = connection.toCoordinate.x;
          var y1 = connection.fromCoordinate.y;
          var y2 = connection.toCoordinate.y;
          if(pDistance(mouseX, mouseY, x1, y1, x2, y2) < 5) {
            setBlockedPath(connection.fromRoom, connection.toRoom, i);
            break;
          }
        }
      }
    }
  }

  Repeater {
    id: roomView
    model: root.roomModel

    Item {
      x: coordinate.x - 24
      y: coordinate.y - 24
      width: 48
      height: 48

      MouseArea {
        anchors.fill: parent
        onClicked: {
          if (roomClickModeSwitch.currentItem == 0)
            setRoomIsTarget(model.id, !isTarget);
          else if (roomClickModeSwitch.currentItem == 1)
            setCurrentRoom(model.id);
        }
      }

      Rectangle {
        id: rect
        anchors.fill: parent
        visible: false
        radius: 24
        color: model.name === 'Aspirant\'s Trial' ? '#101010' : '#E0E0E0'
        SvgImage {
          anchors.centerIn: parent
          width: 32
          height: 32
          source: model.name === 'Aspirant\'s Trial' ? 'qrc:/images/lab-content/izaro.svg' : ''
        }
      }

      DropShadow {
        anchors.fill: rect
        source: rect
        verticalOffset: 2
        radius: 4.0
        samples: 8
        color: '#80000000'
      }

      Row {
        id: contentsView
        visible: false
        anchors.centerIn: parent
        spacing: 2

        Repeater {
          model: contents
          SvgImage {
            source: modelData in Global.contentIconMapping ? 'qrc:/images/lab-content/' + Global.contentIconMapping[modelData] + '.svg' : ''
          }
        }
      }

      DropShadow {
        anchors.fill: contentsView
        source: contentsView
        verticalOffset: 1
        radius: 2.0
        samples: 8
        color: '#80000000'
      }

      Item {
        anchors {
          right: parent.right
          rightMargin: 8
          top: parent.top
          topMargin: 8
        }
        opacity: isTarget ? 1 : 0
        scale: isTarget ? 1 : 4
        Behavior on opacity { StandardAnimation {} }
        Behavior on scale { StandardAnimation {} }

        SvgImage {
          id: targetMarker
          anchors.centerIn: parent
          source: 'qrc:/images/star-gold.svg'
          visible: false
        }

        DropShadow {
          anchors.fill: targetMarker
          source: targetMarker
          verticalOffset: 1
          radius: 2.0
          samples: 8
          color: '#A0000000'
        }
      }

      Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: 16
        height: 16
        radius: 8
        color: '#80000000'

        Text {
          anchors.centerIn: parent
          text: index
          color: 'white'
          font.pixelSize: 13
        }
      }
    }
  }

  Repeater {
    id: goldenDoorView
    model: goldenDoorModel

    Item {
      x: coordinate.x
      y: coordinate.y

      SvgImage {
        id: goldenDoorIcon
        anchors.centerIn: parent
        source: 'qrc:/images/lab-content/golden-door.svg'
        visible: false
      }
      DropShadow {
        anchors.fill: goldenDoorIcon
        source: goldenDoorIcon
        verticalOffset: 2
        radius: 4.0
        samples: 8
        color: '#80000000'
      }
    }
  }

  Repeater {
    id: currentRoomView
    model: roomModel
    Item {
      x: coordinate.x
      y: coordinate.y
      SvgImage {
        id: mapMarker
        width: 36
        height: 36
        source: 'qrc:/images/map-marker-orange.svg'
        x: - 18
        y: isCurrent ? -44 : -76
        opacity: isCurrent ? 1 : 0
        Behavior on y { StandardAnimation {} }
        Behavior on opacity { StandardAnimation {} }
      }
    }
  }

  RoomClickModeSwitch {
    id: roomClickModeSwitch

    property int selectedIndex: 0
    onItemClicked: selectedIndex = index
    currentItem: !Global.model.inLab ? 0 :
                 !Global.model.currentRoomDetermined ? 1 :
                 selectedIndex
    enabled: Global.model.inLab && Global.model.currentRoomDetermined
    visible: Global.model.isValid

    anchors {
      top: parent.top
      right: parent.right
      topMargin: 10
      rightMargin: 10
    }
    width: 48
    height: 24
  }

  Repeater {
    id: roomTooltipView
    model: root.roomModel

    Item {
      x: coordinate.x - 24
      y: coordinate.y - 24
      width: 48
      height: 48

      TooltipHoverArea {
        id: roomHoverArea
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
      }

      RoomLabel {
        roomName: name
        roomContents: contents
        opacity: roomHoverArea.shouldDisplayTooltip ? 1 : 0

        Behavior on opacity {
          NumberAnimation {
            easing.type: Easing.Bezier
            easing.bezierCurve: [0.4, 0.0, 0.2, 1.0, 1.0, 1.0]
            duration: 100
          }
        }
      }
    }
  }

  Connections {
    target: connectionModel
    onModelReset: connectionView.requestPaint()
    onDataChanged: connectionView.requestPaint()
  }
}
