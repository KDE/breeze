/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezewidgetstateengine_h
#define breezewidgetstateengine_h

#include "breeze.h"
#include "breezebaseengine.h"
#include "breezedatamap.h"
#include "breezewidgetstatedata.h"

namespace Breeze
{

//* used for simple widgets
class WidgetStateEngine : public BaseEngine
{
    Q_OBJECT

public:
    //* constructor
    explicit WidgetStateEngine(QObject *parent)
        : BaseEngine(parent)
    {
    }

    //* register widget
    bool registerWidget(QWidget *, AnimationModes);

    //* returns registered widgets
    WidgetList registeredWidgets(AnimationModes) const;

    using BaseEngine::registeredWidgets;

    //* true if widget hover state is changed
    bool updateState(const QObject *, AnimationMode, bool);

    //* true if widget is animated
    bool isAnimated(const QObject *, AnimationMode);

    //* animation opacity
    qreal opacity(const QObject *object, AnimationMode mode)
    {
        return isAnimated(object, mode) ? data(object, mode).data()->opacity() : AnimationData::OpacityInvalid;
    }

    //* animation mode
    /** precedence on focus */
    AnimationMode frameAnimationMode(const QObject *object)
    {
        if (isAnimated(object, AnimationEnable))
            return AnimationEnable;
        else if (isAnimated(object, AnimationFocus))
            return AnimationFocus;
        else if (isAnimated(object, AnimationHover))
            return AnimationHover;
        else
            return AnimationNone;
    }

    //* animation opacity
    /** precedence on focus */
    qreal frameOpacity(const QObject *object)
    {
        if (isAnimated(object, AnimationEnable))
            return data(object, AnimationEnable).data()->opacity();
        else if (isAnimated(object, AnimationFocus))
            return data(object, AnimationFocus).data()->opacity();
        else if (isAnimated(object, AnimationHover))
            return data(object, AnimationHover).data()->opacity();
        else
            return AnimationData::OpacityInvalid;
    }

    //* animation mode
    /** precedence on mouseOver */
    AnimationMode buttonAnimationMode(const QObject *object)
    {
        if (isAnimated(object, AnimationEnable))
            return AnimationEnable;
        else if (isAnimated(object, AnimationHover))
            return AnimationHover;
        else if (isAnimated(object, AnimationFocus))
            return AnimationFocus;
        else
            return AnimationNone;
    }

    //* animation opacity
    /** precedence on mouseOver */
    qreal buttonOpacity(const QObject *object)
    {
        if (isAnimated(object, AnimationEnable))
            return data(object, AnimationEnable).data()->opacity();
        else if (isAnimated(object, AnimationHover))
            return data(object, AnimationHover).data()->opacity();
        else if (isAnimated(object, AnimationFocus))
            return data(object, AnimationFocus).data()->opacity();
        else
            return AnimationData::OpacityInvalid;
    }

    //* duration
    void setEnabled(bool value) override
    {
        BaseEngine::setEnabled(value);
        _hoverData.setEnabled(value);
        _focusData.setEnabled(value);
        _enableData.setEnabled(value);
        _pressedData.setEnabled(value);
    }

    //* duration
    void setDuration(int value) override
    {
        BaseEngine::setDuration(value);
        _hoverData.setDuration(value);
        _focusData.setDuration(value);
        _enableData.setDuration(value);
        _pressedData.setDuration(value / 2);
    }

public Q_SLOTS:

    //* remove widget from map
    bool unregisterWidget(QObject *object) override
    {
        if (!object)
            return false;
        bool found = false;
        if (_hoverData.unregisterWidget(object))
            found = true;
        if (_focusData.unregisterWidget(object))
            found = true;
        if (_enableData.unregisterWidget(object))
            found = true;
        if (_pressedData.unregisterWidget(object))
            found = true;
        return found;
    }

protected:
    //* returns data associated to widget
    DataMap<WidgetStateData>::Value data(const QObject *, AnimationMode);

    //* returns data map associated to animation mode
    DataMap<WidgetStateData> &dataMap(AnimationMode);

private:
    //* maps
    DataMap<WidgetStateData> _hoverData;
    DataMap<WidgetStateData> _focusData;
    DataMap<WidgetStateData> _enableData;
    DataMap<WidgetStateData> _pressedData;
};

}

#endif
