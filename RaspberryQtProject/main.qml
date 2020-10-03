import QtQuick 2.12
import QtQuick.Window 2.12

Window
{
    id:rootPage
    visible: true
    width: 800
    height: 480
    color: "black"

    FontLoader
    {
        id: typeFont
        source:"qrc:/images/Helvetica.ttf"
    }

    FontLoader
    {
       id: typeFont_Bold
       source:"qrc:/images/HELVETICA-SEMIBOLD.ttf"
    }

    FontLoader
    {
        id: fontOpenSans
        source:"qrc:/images/OpenSans-Light.ttf"
    }

    signal reqScanPage()
    signal reqDataPage()

    onReqScanPage:
    {
        scanPage.visible = true;
        dataPage.visible = false;
        scanPage.clearList();
    }

    onReqDataPage:
    {
        scanPage.visible = false;
        dataPage.visible = true;
        dataPage.connected = true;
    }


    //////////////////////////////////////////////////////////////
    // Qml pages
    //////////////////////////////////////////////////////////////
    ScanPage
    {
        id: scanPage
        width: parent.width
        height: parent.height
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        visible: true
    }

    DataPage
    {
        id: dataPage
        width: parent.width
        height: parent.height
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        visible: false
    }

}
