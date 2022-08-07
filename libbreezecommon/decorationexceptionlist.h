#ifndef decorationexceptionlist_h
#define decorationexceptionlist_h

//////////////////////////////////////////////////////////////////////////////
// decorationexceptionlist.h
// window decoration exceptions
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezecommon_export.h"

#include "breeze.h"
#include "breezesettings.h"

#include <KSharedConfig>

namespace Breeze
{

//! breeze exceptions list
class BREEZECOMMON_EXPORT DecorationExceptionList
{
public:
    //! constructor from list
    explicit DecorationExceptionList(const InternalSettingsList &exceptions = InternalSettingsList(),
                                     const InternalSettingsList &defaultExceptions = InternalSettingsList())
        : _exceptions(exceptions)
        , _defaultExceptions(defaultExceptions)
    {
    }

    //! exceptions
    const InternalSettingsList &get(void) const
    {
        return _exceptions;
    }

    //! default exceptions
    const InternalSettingsList &getDefault(void) const
    {
        return _defaultExceptions;
    }

    //! read from KConfig
    void readConfig(KSharedConfig::Ptr, const bool readDefaults = false);

    //! return the number of default exceptions (call afer calling readConfig)
    int numberDefaults();

    //! write to kconfig
    void writeConfig(KSharedConfig::Ptr);

protected:
    //! generate exception group name for given exception index
    static QString exceptionGroupName(int index);

    //! generate exception group name for given default exception index
    static QString defaultExceptionGroupName(int index);

    //! read configuration
    static void readConfig(KCoreConfigSkeleton *, KConfig *, const QString &);

    //! write configuration
    static void writeConfig(KCoreConfigSkeleton *, KConfig *, const QString &);

private:
    void readIndividualExceptionFromConfig(KSharedConfig::Ptr config, QString &groupName, InternalSettingsList &appendTo);

    //! exceptions
    InternalSettingsList _exceptions;

    //! default exceptions
    InternalSettingsList _defaultExceptions;
};

}

#endif
