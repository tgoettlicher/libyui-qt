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

  File:	      YQSpacing.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-

#ifndef YQSpacing_h
#define YQSpacing_h

#include <qwidget.h>
#include "YSpacing.h"

class YUIQt;

class YQSpacing : public QWidget, public YSpacing
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    YQSpacing(YUIQt *yuiqt, QWidget *parent, YWidgetOpt &opt,
	      float size, bool horizontal, bool vertical );

    /**
     * Inherited from YSpacing:
     * Return size in pixels. Assume one basic unit to be equivalent to
     * 80*25 characters at dialog default size (640*480).
     */
    long absoluteSize(YUIDimension dim, float relativeSize);
};

#endif // YQSpacing_h