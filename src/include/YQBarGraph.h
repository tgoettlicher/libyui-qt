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

  File:	      YQBarGraph.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-

#ifndef YQBarGraph_h
#define YQBarGraph_h

#include <ycp/YCPString.h>

#include "QY2BarGraph.h"
#include "YBarGraph.h"


class YUIQt;

class YQBarGraph : public QWidget, public YBarGraph
{
    Q_OBJECT

public:

    /**
     * Create a new YQBarGraph.
     * @param yuiqt the YUIQt, for getting the font info
     * @param parent the parent widget
     * @param text the initial text of the label
     * @param heading true if this should be a big Heading()
     * @param output_field true if this should look like a read-only input field
     */
    YQBarGraph( YUIQt *yuiqt, QWidget *parent, YWidgetOpt &opt );

    /**
     * Inherited from YWidget: Sets the enabled state of the
     * widget. All new widgets are enabled per definition. Only
     * enabled widgets can take user input.
     */
    void setEnabling(bool enabled);

    /**
     * Minimum size the widget should have to make it look and feel
     * nice.
     * @dim Dimension, either YD_HORIZ or YD_VERT
     */
    long nicesize(YUIDimension dim);

    /**
     * Sets the new size of the widget.
     */
    void setSize(long newwidth, long newheight);


    /**
     * Perform a visual update on the screen.
     * Inherited from YBarGraph.
     */
    void doUpdate();

protected:

    QY2BarGraph *_barGraph;
};

#endif // YQBarGraph_h