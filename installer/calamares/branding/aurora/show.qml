/* Aurora OS Installer Slideshow */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Presentation {
    id: presentation

    Timer {
        interval: 5000
        running: true
        repeat: true
        onTriggered: presentation.goToNextSlide()
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0a1a"

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 24

                Text {
                    text: "Welcome to Aurora OS"
                    color: "#00ff87"
                    font.pixelSize: 36
                    font.weight: Font.Light
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "A beautiful Arch-based operating system\ninspired by the northern lights"
                    color: "#e0e0e0"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0a1a"

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 24

                Text {
                    text: "🖥️ Aurora Desktop"
                    color: "#00ff87"
                    font.pixelSize: 32
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Custom Wayland compositor with smooth animations,\naurora-inspired themes, and desktop widgets"
                    color: "#e0e0e0"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0a1a"

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 24

                Text {
                    text: "🚀 Powered by Arch Linux"
                    color: "#b388ff"
                    font.pixelSize: 32
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Rolling release • Latest packages • AUR access\nFull control over your system"
                    color: "#e0e0e0"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0a1a"

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 24

                Text {
                    text: "🎨 Beautiful by Default"
                    color: "#00e5ff"
                    font.pixelSize: 32
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Dark theme with aurora accent colors\nCustom GTK theme • Icon pack • Wallpapers"
                    color: "#e0e0e0"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }
}
