import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
//import QtCharts 2.0

Rectangle
{
    id:dataPage
    color: "transparent"

    property bool connected: true

    property int minTemp: 500
    property int maxTemp: -200

    property int minHumidity: 1000
    property int maxHumidity: 0

    property int minPM25: 500
    property int maxPM25: 0

    property int minFormaldeyde: 2000
    property int maxFormaldeyde: 0

    property int minOzone: 10000
    property int maxOzone: 0

    property int minCO2: 10000
    property int maxCO2: 0

    GridLayout
    {
        id: grid
        rows: 2
        columns: 3
        anchors.fill: parent
        visible: connected


        /////////////////////////////////////////////////////////////////////
        // ROW 1
        /////////////////////////////////////////////////////////////////////
        Rectangle
        {
            id: view_1
            color: "transparent"
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.row: 0
            Layout.column: 0

            ItemParam
            {
                id: gaugeTemperature
                width: parent.height
                anchors.centerIn: parent
                title2: "Temperature"
                unit: "°C"
                minValue: -20
                maxValue: 50
                paramValue: 0
                regMinValue: 0
                regMaxValue: 0
            }
         }

        Rectangle
        {
            id: view_2
            color: "transparent"
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.row: 0
            Layout.column: 1

            ItemParam
            {
                id: gaugeHumidity
                width: parent.height
                anchors.centerIn: parent
                title2: "Humidity"
                unit: "%"
                minValue: 0
                maxValue: 100
                paramValue: 0.0
                regMinValue: 0
                regMaxValue: 0
            }

         }

        Rectangle
        {
            id: view_3
            color: "transparent"
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.row: 0
            Layout.column: 2

            ItemParam
            {
                id: gaugePM25
                width: parent.height
                anchors.centerIn: parent
                title2: "PM2.5"
                unit: "ug/m3"
                minValue: 0
                maxValue: 500
                paramValue: 0
                regMinValue: 0
                regMaxValue: 0
            }
        }

        /////////////////////////////////////////////////////////////////////
        // ROW 2
        /////////////////////////////////////////////////////////////////////
        Rectangle
        {
            id: view_4
            color: "transparent"
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.row: 1
            Layout.column: 0

            ItemParam
            {
                id: gaugeFMH
                width: parent.height
                anchors.centerIn: parent
                title2: "Formaldehyde"
                unit: "ug/m3"
                minValue: 0
                maxValue: 2000
                paramValue: 0
                regMinValue: 0
                regMaxValue: 0
            }
         }

        Rectangle
        {
            id: view_5
            color: "transparent"
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.row: 1
            Layout.column: 1

            ItemParam
            {
                id: gaugeO3
                width: parent.height
                anchors.centerIn: parent
                title2: "Ozone"
                unit: "ppm"
                minValue: 0
                maxValue: 10
                paramValue: 0.0
                regMinValue: 0
                regMaxValue: 0
            }

         }

        Rectangle
        {
            id: view_6
            color: "transparent"
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.row: 1
            Layout.column: 2

            ItemParam
            {
                id: gaugeCO2
                width: parent.height
                anchors.centerIn: parent
                title2: "CO2"
                unit: "ppm"
                minValue: 0
                maxValue: 50000
                paramValue: 0
                regMinValue: 0
                regMaxValue: 0
            }
        }
    }

    Rectangle
    {
        id: btnConnect
        width: 100
        height: 40
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        color: "transparent"
        border.width: 1
        border.color: "gray"
        radius: 8
        visible: false

        Text
        {
            id: name
            anchors.centerIn: parent
            text: qsTr("CONNECT")
            color: "white"
            font.pointSize: 12
        }

        MouseArea
        {
            anchors.fill: parent
            onPressed:
            {
                btnConnect.color = "gray";
                rootPage.reqScanPage();
                btnConnect.visible = false;
                txtConnection.text = ""
            }
            onReleased:
            {
                btnConnect.color = "transparent";
            }

        }
    }


    Text
    {
        id: txtConnection
        text: ""
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 100
        color: "white"
        font.family: typeFont_Bold.name
        font.pointSize: 20
    }

    Image
    {
        id: imgBluetooth
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 5
        source: "qrc:/images/bluetooth.png"
        visible: false
    }

    Timer
    {
        id: imgTimer
        interval: 400
        running: false
        repeat: false
        onTriggered:
        {
            imgBluetooth.visible = false
        }
    }

    Connections
    {
        target: appBridge


        onSensorData:
        {
            minTemp = Math.min(temp, minTemp);
            maxTemp = Math.max(temp, maxTemp);
            gaugeTemperature.paramValue = (temp / 10).toFixed(2);
            gaugeTemperature.regMinValue = (minTemp / 10).toFixed(2);
            gaugeTemperature.regMaxValue = (maxTemp / 10).toFixed(2);

            minHumidity = Math.min(hum, minHumidity);
            maxHumidity = Math.max(hum, maxHumidity);
            gaugeHumidity.paramValue = (hum / 10).toFixed(2);
            gaugeHumidity.regMinValue = (minHumidity / 10).toFixed(2);
            gaugeHumidity.regMaxValue = (maxHumidity / 10).toFixed(2);

            minPM25 = Math.min(pm2_5, minPM25);
            maxPM25 = Math.max(pm2_5, maxPM25);
            gaugePM25.paramValue = pm2_5;
            gaugePM25.regMinValue = minPM25;
            gaugePM25.regMaxValue = maxPM25;

            minFormaldeyde = Math.min(formaldeyde, minFormaldeyde);
            maxFormaldeyde = Math.max(formaldeyde, maxFormaldeyde);
            gaugeFMH.paramValue = formaldeyde;
            gaugeFMH.regMinValue = minFormaldeyde;
            gaugeFMH.regMaxValue = maxFormaldeyde;

            minOzone = Math.min(ozone, minOzone);
            maxOzone = Math.max(ozone, maxOzone);
            gaugeO3.paramValue = (ozone / 1000).toFixed(3);
            gaugeO3.regMinValue = (minOzone / 1000).toFixed(3);
            gaugeO3.regMaxValue = (maxOzone / 1000).toFixed(3);

            minCO2 = Math.min(co2, minCO2);
            maxCO2 = Math.max(co2, maxCO2);
            gaugeCO2.paramValue = co2;
            gaugeCO2.regMinValue = minCO2;
            gaugeCO2.regMaxValue = maxCO2;

            imgBluetooth.visible = true
            imgTimer.start()
        }


        onConnectionChanged:
        {
          connected = false
          btnConnect.visible = true;
          txtConnection.text = "Sensor Disconnected"
        }

    }
}
