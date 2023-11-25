#include <KColorScheme>
#include <KConfig>
#include <KConfigGroup>

#include <QDebug>

int main(int, char **)
{
    KConfig globals("kdeglobals");
    KConfigGroup general(&globals, QStringLiteral("General"));
    if (general.readEntry("ColorScheme") != QLatin1String("BreezeHighContrast")) {
        return 0;
    }
    QString breezeDarkPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes/BreezeDark.colors"));
    if (breezeDarkPath.isEmpty()) {
        return 0;
    }
    KConfig breezeDark(breezeDarkPath, KConfig::SimpleConfig);
    for (const auto &group : breezeDark.groupList()) {
        auto destination = KConfigGroup(&globals, group);
        KConfigGroup(&breezeDark, group).copyTo(&destination, KConfig::Notify);
    }
    return 0;
}
