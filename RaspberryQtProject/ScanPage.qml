import QtQuick 2.12
import QtQuick.Window 2.12

Rectangle
{
    id: scanPage
    color: "transparent"

    property int inputFontSize : 14

    function clearList()
    {
        device.clearDeviceList();
    }

    /*
    Text
    {
        id: label
        anchors.centerIn: parent
        color: "white"
        text: qsTr("Display Size: ") + Screen.width + "x" + Screen.height + " Orientation: " + orientationStatus;
    }
    */

    Rectangle
    {
        id:conteinerList
        width: parent.width * 0.95
        height: parent.height * 0.7
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        color: "transparent"
        //border.width: 1
        //border.color: "white"
        //radius: 3

        Component
        {
            id: delegate
            Rectangle
            {
                id: rectContainer
                width: parent.width * 0.95
                height: 60
                color: "transparent"
                anchors.horizontalCenter: parent.horizontalCenter
                radius: 4

                Rectangle
                {
                    id: backgroundFill
                    anchors.fill: parent
                    color:"white"
                    opacity: 0.08
                    radius: 4
                }

                Text    // Device Name
                {
                    id: txtDeviceName
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.top: parent.top
                    anchors.topMargin: 3
                    text: modelData.deviceName
                    font.pointSize: 15
                    color: "lightgreen"
                }

                Text    // Device address
                {
                    id: txtDeviceAddress
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3
                    text: modelData.deviceAddress
                    font.pointSize: 12
                    color: "white"
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:
                    {
                        device.startConnect(modelData.deviceAddress);
                        device.update = " "
                        rootPage.reqDataPage();
                    }
                }
            }
        }

        ListView
        {
            id:devicesList
            anchors.fill: parent
            model: device.devicesList
            delegate: delegate
            spacing: 2
        }
    }

    Text
    {
        id: rowMessage
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: btnStartDiscover.top
        anchors.bottomMargin: 10
        text: device.update
        color: "white"
        font.pointSize: 12
    }

    Rectangle
    {
        id: btnStartDiscover
        width: 100
        height: 40
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        color: "transparent"
        border.width: 1
        border.color: "gray"
        radius: 8

        Text
        {
            id: name
            anchors.centerIn: parent
            text: qsTr("SCAN")
            color: "white"
            font.pointSize: 12
        }

        MouseArea
        {
            anchors.fill: parent
            onPressed:
            {
                btnStartDiscover.color = "gray";
                device.startScan();
            }
            onReleased:
            {
                btnStartDiscover.color = "transparent";
            }

        }
    }
}
