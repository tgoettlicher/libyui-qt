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

  File:	      YQRichText.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "qt-ui"
#include <ycp/y2log.h>

#include "YDialog.h"	// ???
#include "utf8.h"
#include "YUIQt.h"
#include "YQRichText.h"


YQRichText::YQRichText(YUIQt *yuiqt, QWidget *parent, YWidgetOpt &opt,
		       const YCPString& text)
    : QTextBrowser(parent)
    , YRichText(opt, text)
    , yuiqt(yuiqt)
{
    setWidgetRep((QWidget *)this);
    setFont(yuiqt->currentFont());
    setMargin(AlignRight);
    setTextFormat( opt.plainTextMode.value() ? Qt::PlainText : Qt::RichText );
    QTextBrowser::setText( fromUTF8(text->value()) );


    // Set the text foreground color to black, regardless of its current
    // settings - it might be changed if this widget resides in a
    // warnColor dialog - which we cannot find right now out since our
    // parent is not set yet :-(

    QPalette pal(palette());
    QColorGroup normalColors(pal.normal());
    normalColors.setColor(QColorGroup::Text, black);
    pal.setNormal(normalColors);
    setPalette(pal);

    // Set the text background to a light grey

    setPaper(QColor( 234, 234, 234 ));

    // Very small default size if specified

    shrinkable = opt.isShrinkable.value();
}


void YQRichText::setEnabling(bool enabled)
{
    setEnabled(enabled);
}


long YQRichText::nicesize(YUIDimension dim)
{
    if (dim == YD_HORIZ) return shrinkable ? 10 : 100;
    else 		return shrinkable ? 10 :100;
}


void YQRichText::setSize(long newwidth, long newheight)
{
    resize(newwidth, newheight);
}


void YQRichText::setText(const YCPString &text)
{
    if (verticalScrollBar())   verticalScrollBar()->setValue(0);
    if (horizontalScrollBar()) horizontalScrollBar()->setValue(0);
    QTextBrowser::setText(fromUTF8(text->value()));
}


bool YQRichText::setKeyboardFocus()
{
    QTextBrowser::setFocus();

    return true;
}


void YQRichText::setSource( const QString & name )
{
    y2milestone( "Selected hyperlink \"%s\"", (const char *) name );
    yuiqt->setMenuSelection( YCPString( (const char *) name ) );
    yuiqt->returnNow( YUIInterpreter::ET_MENU, this );
}


#include "YQRichText.moc.cc"