import QtQuick 2.15

Rectangle {
    property int maxNumber : 1000
    property int minNumber : 0
    property int currentNumber : 0
    property int nextNumber : 0
    signal valueChanged();
    id: buttonRect
    width: parent.width * 0.5
    radius: Theme.radiusMd
    height: 40
    color: Theme.surfaceAlt
    border.width: 1
    border.color: Theme.border
    scale: 1.0
    Behavior on color {
        ColorAnimation {
            duration: 150
        }
    }

    SequentialAnimation {
        id: fadeIn
        NumberAnimation { target: buttonlabel; property: "opacity"; to: 1; duration: 75 }
    }
    SequentialAnimation {
        id: fadeOut
        NumberAnimation { target: buttonlabel; property: "opacity"; to: 0; duration: 75 }
        onStopped:
        {
            currentNumber = nextNumber;
            valueChanged();
            fadeIn.start()
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: buttonRect.color = Theme.surfaceHover
        onExited: buttonRect.color = Theme.surfaceAlt
    }
    Row
    {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.topMargin: 5
        anchors.bottomMargin: 5
        WNumberBoxButton
        {
            y:-4
            height: parent.height
            isPlus: true
            width: parent.width * 0.2
            onInvoked: {
                nextNumber = Math.min(maxNumber, nextNumber + 1)
                if(nextNumber!= currentNumber)
                {
                    fadeOut.start();
                }
            }

        }
        Text
        {
            y:-2
            height: parent.height
            width: parent.width * 0.6
            color: Theme.text
            id: buttonlabel
            text: currentNumber
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 16
        }
        WNumberBoxButton
        {
            y:-4
            id:minus
            font.pointSize: 30
            height: parent.height
            width: parent.width * 0.2
            onInvoked: {
                nextNumber = Math.max(minNumber, nextNumber - 1)
                if(nextNumber!= currentNumber)
                {
                    fadeOut.start();
                }
            }
        }
    }


}
