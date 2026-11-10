import QtQuick 2.15
import QtQuick.Controls.Basic

// Scroll view with the app's slim dark scroll bar instead of the light
// Controls Basic default.
ScrollView {
    id: scrollView

    clip: true
    ScrollBar.vertical: WScrollBar { }
}
