#include "GNOMEWindowObserverBackend.hpp"
#include "GNOME.hpp"
#include <QFile>
#include <QDataStream>

#include "gnome_script.c"

namespace Platform {

const QString GNOMEWindowObserverBackend::m_gnomeScriptUUID = "shijima-helper@pixelomer.github.io";
const QString GNOMEWindowObserverBackend::m_gnomeScriptPath = "/tmp/gnome-shijima-helper.zip";
const QString GNOMEWindowObserverBackend::m_gnomeScriptVersion = "1.1";

GNOMEWindowObserverBackend::GNOMEWindowObserverBackend() {
    if (!GNOME::userExtensionsEnabled()) {
        GNOME::setUserExtensionsEnabled(true);
    }
    writeExtensionToDisk();
    GNOME::installExtension(m_gnomeScriptPath);
    auto extensionInfo = GNOME::getExtensionInfo(m_gnomeScriptUUID);
    static const QString kVersionName = "version-name";
    std::string restartReason;
    if (!extensionInfo.contains(kVersionName)) {
        restartReason = "Extension was installed for the first time.";
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    // type() is used here because it also works with Qt5
    else if (extensionInfo[kVersionName].type() != QVariant::String) {
        restartReason = "Active extension contains malformed metadata.";
    }
#pragma GCC diagnostic pop
    else if (extensionInfo[kVersionName].toString() != m_gnomeScriptVersion) {
        restartReason = "Active extension is outdated.";
    }
    if (restartReason != "") {
        // Shell needs to be restarted
        throw std::runtime_error("Shijima GNOME Helper has been installed. "
            "To use Shijima-Qt, log out and log back in. (Restart reason: "
            + restartReason + ")");
    }
    GNOME::enableExtension(m_gnomeScriptUUID);
}

void GNOMEWindowObserverBackend::writeExtensionToDisk() {
    QFile file { m_gnomeScriptPath };
    if (!file.open(QFile::WriteOnly)) {
        throw std::runtime_error("could not open file for writing: "
            + m_gnomeScriptPath.toStdString());
    }
    QDataStream stream { &file };
    stream << QByteArray(gnome_script, gnome_script_len);
    file.flush();
    file.close();
}

GNOMEWindowObserverBackend::~GNOMEWindowObserverBackend() {
    if (alive()) {
        GNOME::disableExtension(m_gnomeScriptUUID);
    }
}

bool GNOMEWindowObserverBackend::alive() {
    return GNOME::isExtensionEnabled(m_gnomeScriptUUID);
}

}