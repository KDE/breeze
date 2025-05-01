# Breeze

Breeze is the default application style for apps run in [Plasma](https://kde.org/plasma-desktop/).

## Components

This repository contains many parts of Breeze, such as:

* Color schemes, located under [/colors](/colors).
* Cursors, located under [/cursors](/cursors).
* Window decorations, located under [/kdecoration](/kdecoration).
* Application style, located under [/kstyle](/kstyle).

### See Also

If you can't find what you're looking for, it may exist in another repository instead:

* [Breeze Icons](https://invent.kde.org/frameworks/breeze-icons) contains the icon set used in Plasma.
* [libplasma](https://invent.kde.org/plasma/libplasma/-/tree/master/src/desktoptheme) includes the [Plasma Style](https://develop.kde.org/docs/plasma/theme/) that affects the visuals for the Plasma panel and widgets.
* The [Breeze SDDM theme](https://invent.kde.org/plasma/plasma-desktop/-/tree/master/sddm-theme) lives within the Plasma Desktop repository, adjacent to the lock screen.
* [Plasma Workspace Wallpapers](https://invent.kde.org/plasma/plasma-workspace-wallpapers) contains the rest of the default wallpapers shipped with Plasma.
* [Breeze for GTK](https://invent.kde.org/plasma/breeze-gtk) is the Breeze [GTK](https://gtk.org/) theme.
* [QQC2 Desktop Style](https://invent.kde.org/frameworks/qqc2-desktop-style) is the default [Qt Quick Controls](https://doc.qt.io/qt-6/qtquickcontrols-index.html) style for desktop KDE applications. It draws controls in QtQuick-based apps using the current application style. If you're using KDE apps on Plasma or another Linux desktop, you're most likely using it.
* [Breeze Style for QQC2](https://invent.kde.org/plasma/qqc2-breeze-style) is a Qt Quick Controls style that mimics the visual styling of Breeze specifically. If you're using KDE apps on Android or Plasma Mobile, you're most likely using it.

## Building

The easiest way to make changes and test Breeze during development is to [build it with kdesrc-build](https://community.kde.org/Get_Involved/development/Build_software_with_kdesrc-build).

When building Breeze manually, keep in mind that the Qt5 and Qt6 versions will be built by default. To control which versions are built, use the `BUILD_QT5` and `BUILD_QT6` CMake variables.

## Contributing

Like other projects in the KDE ecosystem, contributions are welcome from all. This repository is managed in [KDE Invent](https://invent.kde.org/plasma/breeze), our GitLab instance.

* Want to contribute code? See the [GitLab wiki page](https://community.kde.org/Infrastructure/GitLab) for a tutorial on how to send a merge request.
* Reporting a bug? Please submit it on the [KDE Bugtracking System](https://bugs.kde.org/enter_bug.cgi?format=guided&product=breeze). Please do not use the Issues
tab to report bugs.
* Is there a part of Breeze that's not translated? See the [Getting Involved in Translation wiki page](https://community.kde.org/Get_Involved/translation) to see how
you can help translate!

If you get stuck or need help with anything at all, head over to the [KDE New Contributors room](https://go.kde.org/matrix/#/#kde-welcome:kde.org) on Matrix. For questions about Breeze, please ask in the [KDE Development room](https://go.kde.org/matrix/#/#kde-devel:kde.org). See [Matrix](https://community.kde.org/Matrix) for more details.
