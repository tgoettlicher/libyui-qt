/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:	      YUIQt_builtins.cc

  Author:     Stefan Hundhammer <sh@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

  Textdomain "packages-qt"

/-*/

#define USE_QT_CURSORS		1
#define FORCE_UNICODE_FONT	0

#include <sys/stat.h>
#include <unistd.h>

#include <qcursor.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qvbox.h>
#include <qwidgetlist.h>

#include <ycp/YCPTerm.h>
#define y2log_component "qt-ui"
#include <ycp/y2log.h>

#include "YUIQt.h"
#include "YUISymbols.h"
#include "YQDialog.h"

#include "utf8.h"
#include "YQi18n.h"

#include <X11/Xlib.h>


#define DEFAULT_MACRO_FILE_NAME		"macro.ycp"



YCPValue YUIQt::setLanguage( const YCPTerm & term)
{
    return YCPVoid();	// OK (YCPNull() would mean error)
}


int YUIQt::getDisplayWidth()
{
    return desktop()->width();
}


int YUIQt::getDisplayHeight()
{
    return desktop()->height();
}


int YUIQt::getDisplayDepth()
{
    return QColor::numBitPlanes();
}


long YUIQt::getDisplayColors()
{
    return 1L << QColor::numBitPlanes();
}


int YUIQt::getDefaultWidth()
{
    return default_size.width();
}


int YUIQt::getDefaultHeight()
{
    return default_size.height();
}


long YUIQt::defaultSize(YUIDimension dim) const
{
    if ( haveWM() )
	return dim == YD_HORIZ ? default_size.width() : default_size.height();
    else
	return dim == YD_HORIZ ? desktop()->width() : desktop()->height();
}


void
YUIQt::busyCursor ( void )
{
#if USE_QT_CURSORS

    setOverrideCursor( waitCursor );

#else
    /**
     * There were versions of Qt where simply using
     * QApplication::setOverrideCursor( waitCursor ) didn't work any more:
     * We _need_ the WType_Modal flag for non-defaultsize dialogs (i.e. for
     * popups), but Qt unfortunately didn't let such dialogs have a clock
     * cursor.  :-(
     *
     * They might have their good reasons for this (whatever they are), so
     * let's simply make our own busy cursors and set them to all widgets
     * created thus far.
     **/

    QWidgetList *widget_list = allWidgets();
    QWidgetListIt it( *widget_list );

    while ( *it )
    {
	if ( ! (*it)->testWFlags( WType_Desktop ) )	// except desktop (root window)
	{
	    XDefineCursor( (*it)->x11Display(), (*it)->winId(), busy_cursor->handle() );
	}
	++it;
    }

    if ( widget_list )
	delete widget_list;
#endif
}


void
YUIQt::normalCursor ( void )
{
#if USE_QT_CURSORS

    while ( overrideCursor() )
	restoreOverrideCursor();
#else
    /**
     * Restore the normal cursor for all widgets (undo busyCursor() ).
     *
     * Fortunately enough, Qt widgets keep track of their normal cursor
     * (QWidget::cursor()) so this can easily be restored - it's not always the
     * arrow cursor - e.g., input fields (QLineEdit) have the "I-beam" cursor.
     **/

    QWidgetList *widget_list = allWidgets();
    QWidgetListIt it( *widget_list );

    while ( *it )
    {
	if ( ! (*it)->testWFlags( WType_Desktop ) )	// except desktop (root window)
	{
	    XDefineCursor( (*it)->x11Display(), (*it)->winId(), (*it)->cursor().handle() );
	}
	++it;
    }

    if ( widget_list )
	delete widget_list;
#endif
}


YCPString
YUIQt::glyph( const YCPSymbol & glyphSymbol )
{
    string sym = glyphSymbol->symbol();
    QChar unicodeChar;

    // Hint: Use 'xfd' to view characters available in the Unicode font.

    if      ( sym == YUIGlyph_ArrowLeft		)	unicodeChar = QChar( 0x2190 );
    else if ( sym == YUIGlyph_ArrowRight	)	unicodeChar = QChar( 0x2192 );
    else if ( sym == YUIGlyph_ArrowUp		)	unicodeChar = QChar( 0x2191 );
    else if ( sym == YUIGlyph_ArrowDown		)	unicodeChar = QChar( 0x2193 );
    else if ( sym == YUIGlyph_CheckMark		)	unicodeChar = QChar( 0x2714 );
    else if ( sym == YUIGlyph_BulletArrowRight	)	unicodeChar = QChar( 0x279c );
    else if ( sym == YUIGlyph_BulletCircle	)	unicodeChar = QChar( 0x274d );
    else if ( sym == YUIGlyph_BulletSquare	)	unicodeChar = QChar( 0x274f );
    else return YCPString( "" );

    QString qstr( unicodeChar );

    return YCPString( toUTF8( qstr ) );
}


YCPValue YUIQt::runPkgSelection( YWidget * packageSelector )
{
    y2milestone( "Running package selection..." );
    wm_close_blocked		= true;
    auto_activate_dialogs	= false;

    YCPValue input = evaluateUserInput( YCPTerm( YCPSymbol( "", false ) ), false );

    auto_activate_dialogs	= true;
    wm_close_blocked 		= false;
    y2milestone( "Package selection done - returning %s", input->toString().c_str() );

    return input;
}


void YUIQt::makeScreenShot( std::string stl_filename )
{

    //
    // Grab the pixels off the screen
    //

    QWidget *dialog = (QWidget *) currentDialog()->widgetRep();
    QPixmap screenShot = QPixmap::grabWindow( dialog->winId() );
    QString fileName ( stl_filename.c_str() );

    if ( fileName.isEmpty() )
    {
	// Open a file selection box. Figure out a reasonable default
	// directory / file name.

	if ( screenShotNameTemplate.isEmpty() )
	{
	    //
	    // Initialize screen shot directory
	    //

	    QString home = QDir::homeDirPath();
	    char *ssdir = getenv("Y2SCREENSHOTS");
	    QString dir  = ssdir ? ssdir : "yast2-screen-shots";

	    if ( home == "/" )
	    {
		// Special case: $HOME is not set. This is normal in the inst-sys.
		// In this case, rather than simply dumping all screen shots into
		// /tmp which is world-writable, let's try to create a subdirectory
		// below /tmp with restrictive permissions.
		// If that fails, trust nobody - in particular, do not suggest /tmp
		// as the default in the file selection box.

		dir = "/tmp/" + dir;

		if ( mkdir( dir, 0700 ) == -1 )
		    dir = "";
	    }
	    else
	    {
		// For all others let's create a directory ~/yast2-screen-shots and
		// simply ignore if this is already present. This gives the user a
		// chance to create symlinks to a better location if he wishes so.

		dir = home + "/" + dir;
		(void) mkdir( dir, 0750 );
	    }

	    screenShotNameTemplate = dir + "/%s-%03d.png";
	}


	//
	// Figure out a file name
	//

	const char *baseName = moduleName();
	if ( ! baseName ) baseName = "scr";
	int no = screenShotNo[ baseName ];
	fileName.sprintf( screenShotNameTemplate, baseName, no );
	y2debug( "screenshot: %s", (const char *) fileName );
	fileName = askForSaveFileName( fileName, QString( "*.png" ) , "Save screen shot to..." );

	if ( fileName.isEmpty() )
	{
	    y2debug( "Save screen shot canceled by user" );
	    return;
	}

	screenShotNo.insert( baseName, ++no );
    } // if fileName.isEmpty()


    //
    // Actually save the screen shot
    //

    y2debug( "Saving screen shot to %s", (const char *) fileName );
    screenShot.save( fileName, "PNG" );
}


void YUIQt::toggleRecordMacro()
{
    if ( recordingMacro() )
    {
	stopRecordMacro();
	normalCursor();

	QMessageBox::information( 0,						// parent
				  "YaST2 Macro Recorder",			// caption
				  "Macro recording done.",			// text
				  QMessageBox::Ok | QMessageBox::Default,	// button0
				  QMessageBox::NoButton,			// button1
				  QMessageBox::NoButton );			// button2
	busyCursor();
    }
    else
    {
	normalCursor();

	QString filename =
	    QFileDialog::getSaveFileName( DEFAULT_MACRO_FILE_NAME,		// startWith
					  "*.ycp",				// filter
					  0,					// parent
					  0,					// (widget) name
					  "Select Macro File to Record to" );	// caption
	busyCursor();

	if ( ! filename.isEmpty() )	// file selection dialog has been cancelled
	{
	    recordMacro( (const char *) filename );
	}
    }
}


void YUIQt::askPlayMacro()
{
    normalCursor();

    QString filename =
	QFileDialog::getOpenFileName( DEFAULT_MACRO_FILE_NAME,		// startWith
				      "*.ycp",				// filter
				      0,				// parent
				      0,				// (widget) name
				      "Select Macro File to Play" );	// caption
    busyCursor();

    if ( ! filename.isEmpty() )	// file selection dialog has been cancelled
    {
	playMacro( (const char *) filename );

	// Do special magic to get out of any UserInput() loop right now
	// without doing any harm - otherwise this would hang until the next
	// mouse click on a PushButton etc.

	event_type	= ET_WIDGET;
	event_widget	= currentDialog();

	if ( do_exit_loop )
	{
	    exit_loop();
	}
    }
}



YCPValue YUIQt::askForExistingDirectory( const YCPString & startDir,
					 const YCPString & headline )
{
    normalCursor();

    QString dir_name =
	QFileDialog::getExistingDirectory( fromUTF8( startDir->value() ),
					   main_win, 				// parent
					   "dir_selector",			// name
					   fromUTF8( headline->value() ) );	// caption
    busyCursor();

    if ( dir_name.isEmpty() )	// this includes dir_name.isNull()
	return YCPVoid();	// nothing selected -> return 'nil'

    return YCPString( toUTF8( dir_name ) );
}


YCPValue YUIQt::askForExistingFile( const YCPString & startWith,
				    const YCPString & filter,
				    const YCPString & headline )
{
    normalCursor();

    QString file_name =
	QFileDialog::getOpenFileName( fromUTF8( startWith->value() ),
				      fromUTF8( filter->value() ),
				      main_win, 			// parent
				      "file_selector",			// name
				      fromUTF8( headline->value() ) );	// caption
    busyCursor();

    if ( file_name.isEmpty() )	// this includes file_name.isNull()
	return YCPVoid();	// nothing selected -> return 'nil'

    return YCPString( toUTF8( file_name ) );
}


YCPValue YUIQt::askForSaveFileName( const YCPString & startWith,
				    const YCPString & filter,
				    const YCPString & headline )
{
    normalCursor();

    QString file_name = askForSaveFileName( fromUTF8( startWith->value() ),
					    fromUTF8( filter->value() ),
					    fromUTF8( headline->value() ) );
    busyCursor();

    if ( file_name.isEmpty() )		// this includes file_name.isNull()
	return YCPVoid();		// nothing selected -> return 'nil'

    return YCPString( toUTF8( file_name ) );
}



QString YUIQt::askForSaveFileName( const QString & startWith,
				   const QString & filter,
				   const QString & headline )
{
    QString file_name;
    bool try_again = false;

    do
    {
	// Leave the mouse cursor alone - this function might be called from
	// some other widget, not only from UI::AskForSaveFileName().

	file_name = QFileDialog::getSaveFileName( startWith,
						  filter,
						  main_win, 		// parent
						  "file_selector",	// name
						  headline );		// caption

	if ( file_name.isEmpty() )	// this includes file_name.isNull()
	    return QString::null;


	if ( access( (const char *) file_name, F_OK ) == 0 )	// file exists?
	{
	    QString msg;

	    if ( access( (const char *) file_name, W_OK ) == 0 )
	    {
		// Confirm if the user wishes to overwrite an existing file
		msg = ( _( "%1 exists! Really overwrite?" ) ).arg( file_name );
	    }
	    else
	    {
		// Confirm if the user wishes to overwrite a write-protected file %1
		msg = ( _( "%1 exists and is write-protected!\nReally overwrite?" ) ).arg( file_name );
	    }

	    int button_no = QMessageBox::information( main_win,
						      // Window title for confirmation dialog
						      _( "Confirm"   ),
						      msg,
						      _( "C&ontinue" ),
						      _( "&Cancel"   ) );
	    try_again = ( button_no != 0 );
	}

    } while ( try_again );

    return file_name;
}


