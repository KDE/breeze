//////////////////////////////////////////////////////////////////////////////
// breezeconfigurationui.cpp
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "breezeconfigwidget.h"

#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QLayout>

#include <KLocalizedString>

namespace Breeze
{

    //_________________________________________________________
    ConfigWidget::ConfigWidget( QWidget* parent ):
        QWidget( parent ),
        _changed( false )
    {

        ui.setupUi( this );

        // track ui changes
        connect( ui.titleAlignment, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( ui.buttonSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );
        connect( ui.frameBorder, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()) );

        connect( ui.closeFromMenuButton, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( ui.drawBorderOnMaximizedWindows, SIGNAL(clicked()), SLOT(updateChanged()) );

        // track exception changes
        connect( ui.exceptions, SIGNAL(changed(bool)), SLOT(updateChanged()) );

        // track animations changes
        connect( ui.animationsEnabled, SIGNAL(clicked()), SLOT(updateChanged()) );
        connect( ui.animationsDuration, SIGNAL(valueChanged(int)), SLOT(updateChanged()) );

    }

    //_________________________________________________________
    void ConfigWidget::setConfiguration( ConfigurationPtr configuration )
    { _configuration = configuration; }

    //_________________________________________________________
    void ConfigWidget::load( void )
    {
        if( !_configuration ) return;
        ui.titleAlignment->setCurrentIndex( _configuration->titleAlignment() );
        ui.buttonSize->setCurrentIndex( _configuration->buttonSize() );
        ui.frameBorder->setCurrentIndex( _configuration->frameBorder() );
        ui.animationsEnabled->setChecked( _configuration->animationsEnabled() );
        ui.animationsDuration->setValue( _configuration->animationsDuration() );
        ui.closeFromMenuButton->setChecked( _configuration->closeWindowFromMenuButton() );
        ui.drawBorderOnMaximizedWindows->setChecked( _configuration->drawBorderOnMaximizedWindows() );
        setChanged( false );

    }

    //_________________________________________________________
    void ConfigWidget::save( void )
    {

        if( !_configuration ) return;

        // apply modifications from ui
        _configuration->setTitleAlignment( ui.titleAlignment->currentIndex() );
        _configuration->setButtonSize( ui.buttonSize->currentIndex() );
        _configuration->setFrameBorder( ui.frameBorder->currentIndex() );
        _configuration->setCloseWindowFromMenuButton( ui.closeFromMenuButton->isChecked() );
        _configuration->setDrawBorderOnMaximizedWindows( ui.drawBorderOnMaximizedWindows->isChecked() );
        _configuration->setAnimationsEnabled( ui.animationsEnabled->isChecked() );
        _configuration->setAnimationsDuration( ui.animationsDuration->value() );
        setChanged( false );



    }

    //_______________________________________________
    void ConfigWidget::updateChanged( void )
    {

        // check configuration
        if( !_configuration ) return;

        // track modifications
        bool modified( false );

        if( ui.titleAlignment->currentIndex() != _configuration->titleAlignment() ) modified = true;
        else if( ui.buttonSize->currentIndex() != _configuration->buttonSize() ) modified = true;
        else if( ui.frameBorder->currentIndex() != _configuration->frameBorder() ) modified = true;
        else if( ui.closeFromMenuButton->isChecked() != _configuration->closeWindowFromMenuButton() ) modified = true;
        else if( ui.drawBorderOnMaximizedWindows->isChecked() != _configuration->drawBorderOnMaximizedWindows() ) modified = true;

        // exceptions
        else if( ui.exceptions->isChanged() ) modified = true;

        // animations
        else if( ui.animationsEnabled->isChecked() !=  _configuration->animationsEnabled() ) modified = true;
        else if( ui.animationsDuration->value() != _configuration->animationsDuration() ) modified = true;

        setChanged( modified );

    }
}
