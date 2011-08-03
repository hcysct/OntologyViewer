/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2008  Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA  02110-1301, USA.
 * 
 *
 * Author(s): Michael Wybrow  <http://michael.wybrow.info/>
*/


#include "libdunnartcanvas/gmlgraph.h"

#include <QString>
#include <QByteArray>
#include <map>
#include <list>
#include <algorithm>
//#include <tr1/functional>
#include <functional>
#include <cfloat>


#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/instrument.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/freehand.h"
#include "libdunnartcanvas/textshape.h"
#include "libdunnartcanvas/cluster.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/polygon.h"
#include "libdunnartcanvas/templates.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/utility.h"
#include "libdunnartcanvas/canvas.h"

#ifndef NOGRAPHVIZ
#include "libdunnartcanvas/align3.h"
#endif

#include <libavoid/libavoid.h>
#include <libavoid/viscluster.h>


namespace dunnart {

using Avoid::VertID;
using Avoid::EdgeInf;

LayoutDoneCallback* layoutDoneCallback = NULL;

QXmlNamespaceSupport namespaces;

bool queryMode = false;
CanvasItem *queryObj = NULL;

bool freehand_drawing = false;


#if 0
static bool video_handler(SDL_Event *event);
static bool key_handler(SDL_Event *event);
static Uint32 idle_loop_callback(Uint32 interval, void *param);
#endif


#if 0
void processShapeAnimation(SDL_Event *event)
{
    ShapeObj *shape = (ShapeObj *) event->user.data1;
    bool expand = (bool) event->user.data2;

    bool finished = shape->change_detail_level(expand);
    if ((lastCanvasRepaintN == canvasRepaintN))
    {
        // Forcing canvas_repaint, since it hasn't happened due to 
        // any other events since the last move of the canvas.
        repaint_canvas();
    }
    lastCanvasRepaintN = canvasRepaintN;

    if (finished)
    {
        shape->setBeingResized(false);
    }
    else
    {
        SDLGui::postUserEvent(USEREVENT_SHAPE_RESIZE, 
                event->user.data1, event->user.data2);
    }
}


class CanvasMoveAnimation {
    public:
        CanvasMoveAnimation(int x, int y):
                origX(cxoff),
                origY(cyoff),
                diffx(x - cxoff),
                diffy(y - cyoff),
                steps(30),
//                steps(sqrt(std::max(fabs(diffx), fabs(diffy) * 10))),
                currStep(0),
                range(2.4), 
                halfRange(range/2),
                domainStart(-tan(halfRange)),
                domainEnd(tan(halfRange)),
                domain(domainEnd-domainStart)
        {
            lastCanvasRepaintN = canvasRepaintN;
            SDLGui::postUserEvent(USEREVENT_CANVAS_ANIMATE);
        }
        bool process(void)
        {
            if (currStep >= steps)
            {
                return true;
            }

            if ((currStep > 0) && (lastCanvasRepaintN == canvasRepaintN))
            {
                // Forcing canvas_repaint, since it hasn't happened due to 
                // any other events since the last move of the canvas.
                repaint_canvas();
            }
            lastCanvasRepaintN = canvasRepaintN;
            SDLGui::postUserEvent(USEREVENT_CANVAS_ANIMATE);

            double dp=static_cast<double>(currStep)/static_cast<double>(steps);
            double p=domainStart+dp*domain;
            double d=(atan(p)+halfRange)/range;
            //assert(d>=0.0);
            //assert(d<=1.0);
            cxoff = static_cast<int> (origX + (d * diffx));
            cyoff = static_cast<int> (origY + (d * diffy));

            bool animate = false;
            move_canvas_offset(cxoff, cyoff, animate);
            currStep++;
            return false;
        }

        int origX;
        int origY;
        double diffx;
        double diffy;
        const int steps;
        int currStep;
        const double range;
        const double halfRange;
        const double domainStart;
        const double domainEnd;
        const double domain;
};
CanvasMoveAnimation *canvasMoveAnimation = NULL;


static Widget *overviewDialog = NULL;

void move_canvas_offset(int x, int y, bool animate, gml::Graph* g)
{
    if (animate)
    {
        if (canvasMoveAnimation)
        {
            delete canvasMoveAnimation;
        }
        canvasMoveAnimation = new CanvasMoveAnimation(x, y);
        return;
    }
    cxoff=x;
    cyoff=y;
    expCanvas.w = expCanvas.h = 0;
    canvasRect = wholeCanvasRect;
    if(g) {
        g->updateOverview();
    }

    if (gmlGraph)
    {
        // Quicly show where the selection is in the overview window.
        gmlGraph->drawOverviewOverlay();
    }
}

static void createOverviewDialog(QWidget **c)
{
    assert(gmlGraph!=NULL);
    if (overviewDialog)
    {
        overviewDialog->bring_to_front();
        return;
    }
    
    gmlGraph->createOverviewWindow(&overviewDialog,1,ytop);
}
    


void change_downward_separation(QWidget **gobj)
{
    // Tick the checkbox when the slider moves.
    //QT changeControlState(BUT_DOWNWARD, SDLGui::WIDGET_true);
    interrupt_graph_layout();
    restart_graph_layout();
}

#endif


#if 0
// When a new node is added to the canvas, this function is called
// to check for any constraints that need to be added to the new node
// and/or any other objects already on the canvas.  For example, 
// connecting the new node to an existing node via an edge, or adding
// the new node to a template.
static void addNewNodeConstraints(ShapeObj* toShape)
static void createConnector(ShapeObj* toShape)
{
//printf("*** inside addNewNodeConstraints ***\n");
    Avoid::Point p(mouse.x, mouse.y);
    offsetPoint(p);
    for(QWidget *co = canvas->get_children_head(); co; co=co->get_next())
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (co);
        Template *templatPtr = dynamic_cast<Template *> (co);
        if (shape && shape!=toShape && inPoly(*(shape->shapePoly), p))
        {
            if (!(shape->selectHandle(HAN_CONNPT | HAN_CENTER) &&
                    toShape->selectHandle(HAN_CONNPT | HAN_CENTER)))
            {
                // One of the shapes is missing a center connection point,
                // so do nothing.
                return;
            }

            printf("Over shape\n");
            Conn* newConn = new AvoidingConn(shape,toShape);
            add_undo_record(DELTA_ADD, newConn);
            //add_undo_record(DELTA_CONNS, newConn);

        }
	// if the new Shape overlaps a template, snap to template
        else if (templatPtr) {
		  if(templatPtr->testShapeOverlap(toShape)) {
			printf("###### testShapeOverlap returned true\n");
			templatPtr->snapShapeToNearestGuideline(toShape);
		  }
        }
    }
}

//============================================================================
// Selection code:

void shape_change_highlight_state(QWidget *shape, int highlight_type);
#endif


#if 0
void finish_partial_move(void)
{
    for (SelectionList::reverse_iterator sh = selection.rbegin();
            sh != selection.rend(); sh++)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (*sh);
        
        if (shape)
        {
            bool first_move = false;
            visalgo->obj_move(shape, first_move);
        }
    }

}

void dragAbortActions(void)
{
    mouse.b = 0;
    
    undo_action(NULL);
    clear_redo_stack();
}


// removeFromCanvas(CanvasItemList& list):
//     Input:   A list of Shape pointers.
//     Result:  Removes those shapes and all resultant dangling connectors
//              from the canvas, and puts a representation for them in the
//              inactiveObjs xml document.
//
void removeFromCanvas(CanvasItemList& list)
{
    assert(&list != &selection);

    // Add to the list, all the attached shapes.
    CanvasItemSet fullSet;
    for (CanvasItemList::iterator curr = list.begin(); curr != list.end(); 
            ++curr)
    {
        (*curr)->findAttachedSet(fullSet);
    }

    // Determine connectors attached to these shapes.
    CanvasItemSet connSet;
    for (CanvasItemSet::iterator curr = fullSet.begin(); curr != fullSet.end(); 
            ++curr)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (*curr);
        if (shape)
        {
            // Add attached connectors.
            ConnMultiset connMultiSet = shape->getConnMultiset();
            connSet.insert(connMultiSet.begin(), connMultiSet.end());
        }
    }
    // Remove the connectors that would be dangling after removing shapes
    // in the list.  Also remove guidelines and relationships.
    for (CanvasItemSet::iterator obj = connSet.begin(); 
            obj != connSet.end(); ++obj)
    {
        (*obj)->setAsInactive(true);
    }
    // Now, remove the shapes and attached objects in the list.
    for (CanvasItemSet::iterator curr = fullSet.begin(); curr != fullSet.end(); 
            ++curr)
    {
        (*curr)->setAsInactive(true, fullSet);
    }
}
#endif

// void returnToCanvas(CanvasItemList& list):
//     Input:   A list of Dunnart shape pointers.
//     Result:  Inactive shapes and their attached connectors are returned
//              to the canvas.
void returnToCanvas(CanvasItemList& list)
{
    Q_UNUSED (list)
#if 0
    // Add to the list, all the attached shapes.
    CanvasItemSet fullSet;
    for (CanvasItemList::iterator curr = list.begin(); curr != list.end(); 
            ++curr)
    {
        (*curr)->findAttachedSet(fullSet);
    }

    // Return the shapes in the list.
    for (CanvasItemSet::iterator curr = fullSet.begin(); curr != fullSet.end(); 
            ++curr)
    {
        if (dynamic_cast<ShapeObj *> (*curr))
        {
            (*curr)->setAsInactive(false);
        }
    }
    for (CanvasItemSet::iterator curr = fullSet.begin(); curr != fullSet.end(); 
            ++curr)
    {
        if (dynamic_cast<Guideline *> (*curr))
        {
            (*curr)->setAsInactive(false);
        }
    }
    // Now, bring back appropriate connectors.
    returnAppropriateConnectors();
#endif
}


#if 0
void selection_makeInactive(QWidget **c)
{
    CanvasItemList selCopy = selection;
    GraphLayout::getInstance()->setInterruptFromDunnart();
    removeFromCanvas(selCopy);
    restart_graph_layout(c);
    repaint_canvas();
}


void selection_returnInactive(QWidget **c)
{
    GraphLayout::getInstance()->setInterruptFromDunnart();
    returnAllInactive();
    restart_graph_layout(c);
}
#endif


#if 0


void change_selected_shape_label(void)
{
    if (selection.size() < 1)
    {
        return;
    }
    
    // Take the first Object in selection.
    ShapeObj *shape = dynamic_cast<ShapeObj *> (selection.front());

    // Set label if it's a shape.
    if (shape)
    {
        shape->change_label();
    }
}


//============================================================================


void remove_directed_edge_constraints(QWidget **c)
{
    for (CanvasItemList::iterator curr = selection.begin();
            curr != selection.end(); ++ curr)
    {
        Conn *conn = dynamic_cast<Conn *> (*curr);
        if (conn)
        {
            conn->isInEa(true);
        }
    }
    interrupt_graph_layout(NULL);
}


void toggle_query_mode(QWidget **c)
{
    if (queryMode == false)
    {
        queryMode = true;
        changeControlState(BUT_QUERY, SDLGui::WIDGET_true);
        if (selection.size() == 1)
        {
            queryObj = dynamic_cast<CanvasItem *> (selection.front());
            if (queryObj)
            {
                // If it's a shape, render the highlight.
                queryObj->update();
            }
        }
        // We might need to highlight straight away.
        if (dynamic_cast<CanvasItem *> (active_obj))
        {
            // Query the shape under the mouse if there is one.
            pair_query(active_obj);
        }
        else if (active_obj)
        {
            // This case copes with us being over a handle.
            // Query the parent of the handle, i.e, the shape.
            if (dynamic_cast<CanvasItem *> (active_obj->get_parent()))
            {
                pair_query(active_obj->get_parent());
            }
        }
    }
    else  // if (queryMode == true)
    {
        changeControlState(BUT_QUERY, SDLGui::WIDGET_false);
        
        bool resetQueryObj = true;
        resetQueryModeIllumination(resetQueryObj);
        queryMode = false;
    }
}


void set_query_mode(const bool setting)
{
    if (queryMode != setting)
    {
        toggle_query_mode(NULL);
    }
}
#endif


#if 0
static void toggle_poly_conns(GuiObj **c)
{
    if (new_connector_type != CONN_TYPE_AVOID)
    {
        new_connector_type = CONN_TYPE_AVOID;
        changeControlState(BUT_ORTHCONN, SDLGui::WIDGET_false);
        changeControlState(BUT_POLYCONN, SDLGui::WIDGET_true);
    }
}
#endif


#if 0
static void toggle_directed_connectors(QWidget **c)
{
    if (!createDirectedConns)
    {
        createDirectedConns = true;
        changeControlState(BUT_DIRECTED, SDLGui::WIDGET_true);
    }
    else  // if (createDirectedConns)
    {
        createDirectedConns = false;
        changeControlState(BUT_DIRECTED, SDLGui::WIDGET_false);
    }
}


void toggle_partial_feedback(QWidget **c)
{
    bool& PartialFeedback = router->PartialFeedback;

    if (!PartialFeedback)
    {
        PartialFeedback = true;
        changeControlState(BUT_PARTIAL, SDLGui::WIDGET_true);
    }
    else  // if (PartialFeedback)
    {
        PartialFeedback = false;
        changeControlState(BUT_PARTIAL, SDLGui::WIDGET_false);
    }
}


static void buffer_change_callback(QWidget **c)
{
    if (automatic_graph_layout)
    {
        // XXX: Do we need to reroute connectors attached to the ports on
        //      the edge of shapes so their endpoints change?
        interrupt_graph_layout(c);
    }
    else
    {
        // Recompute the visibility graph with new spacing:
        for (QWidget *go = canvas->get_children_head(); go; go = go->get_next())
        {
            ShapeObj *shape = dynamic_cast<ShapeObj *> (go);

            if (shape)
            {
                bool store_undo = false;
                visalgo->obj_move(shape, store_undo);
            }
        }

        if (router) 
        {
            router->processTransaction();
        }

        bool force = true;
        reroute_connectors(force);
        repaint_canvas();
    }
}


void relayout_selection(QWidget **c) {
    GraphLayout* gl = GraphLayout::getInstance();
    //gl->runLevel=1;
    gl->setInterruptFromDunnart();
    printf(".Relayout selection!\n");
    if (!selection.empty()) {
        printf("Relayout selection!\n");
        gl->lockUnselectedShapes(c);
        gl->apply(false);
        gl->unlockAll(c);
    } else {
        gl->apply(false);
    }
}


static void fade_settings_callback(QWidget **objectPtr)
{
    int level = (int) placement_aid_base_alpha;

    if (fade_aid_amount == 0)
    {
        // Never faded.
        fade_out_aids = false;
        always_faded_aids = false;
        level = 255;
    }
    else if (fade_aid_amount == 60)
    {
        // Always faded.
        fade_out_aids = false;
        always_faded_aids = true;
    }
    else
    {
        // Variable fade time:
        fade_out_aids = true;
        always_faded_aids = false;
        level = 255;
    }

    for (QWidget *go = canvas->get_children_head(); go; go = go->get_next())
    {
        Indicator *indicator = dynamic_cast<Indicator *> (go);

        if (indicator)
        {
            indicator->setAlpha(level);
            indicator->update();
        }
    }
}


static void redraw_indicators(QWidget **slider)
{
    for (QWidget *go = canvas->get_children_head(); go; go = go->get_next())
    {
        Indicator *indicator = dynamic_cast<Indicator *> (go);

        if (indicator)
        {
            if (always_faded_aids)
            {
                indicator->setAlpha((int) placement_aid_base_alpha);
            }
            indicator->update();
        }
    }
}


static bool repaint_all_indicators(void)
{
    bool changes = false;

    for (QWidget *go = canvas->get_children_head(); go; go = go->get_next())
    {
        Indicator *indicator = dynamic_cast<Indicator *> (go);

        if (indicator)
        {
            if (fade_out_aids) {
                changes |= indicator->reduceAlpha();
            }
            else {
                changes |= indicator->reduceGlow();
            }
            
            if (changes)
            {
                indicator->repaint();
            }
        }
    }
    return changes;
}



void idle_loop_handler(SDL_Event *event)
{
    if (event->user.code == USEREVENT_FADE_TIMER)
    {
        bool changes = repaint_all_indicators();
        
        if (changes)
        {
            repaint_canvas();
        }
    }
    else if (event->user.code == USEREVENT_CANVAS_ANIMATE)
    {
        if (canvasMoveAnimation)
        {
            bool finished = canvasMoveAnimation->process();
            
            if (finished)
            {
                delete canvasMoveAnimation;
                canvasMoveAnimation = NULL;
            }
        }
    }
    else if (event->user.code == USEREVENT_SHAPE_RESIZE)
    {
        processShapeAnimation(event);
    }
    else if (event->user.code == USEREVENT_LAYOUT_FREESHIFT)
    {
        // XXX: We would like to use partial connector feedback here to
        //      only reroute the connectors attached to the moving shape(s).
        /*
        if (!straighten_bends)
        {
            printf("USEREVENT_LAYOUT_FREESHIFT\n");
            bool lastSimpleRouting = router->SimpleRouting;
            router->SimpleRouting = false;
            reroute_connectors();
            router->SimpleRouting = lastSimpleRouting;
        }
        */
        repaint_canvas();
    }
    else if (event->user.code == USEREVENT_LAYOUT_DONE)
    {
    }
    else if (event->user.code == USEREVENT_LAYOUT_UPDATES)
    {
    }
}

    
static Uint32 idle_loop_callback(Uint32 interval, void *param)
{
    //printf("--- %d, %d\n", mouse.x - canvas->get_xpos(), mouse.y - canvas->get_ypos());

    SDLGui::postUserEvent(USEREVENT_FADE_TIMER);

    idleTimerID = 0;
    return 0;
}


void _callback(QWidget **object_addr) {};


int fadeLevelExtraDraw(QPixmap *sur, bool disabled)
{
    int texth = TTF_FontHeight(winStndrd);

    if (!sur)
    {
        return 8;
    }

    int lower = 8;
    int upper = sur->w - 11;
    int pixels = upper - lower + 1;
    for (int x = lower; x <= upper; x++)
    {
        SDL_Rect crect = { x, texth + 2, 1, 10 };
        SDL_SetClipRect(sur, &crect);
        int alpha = (int) (((x - lower) / (double) pixels) * 245);
        boxRGBA(sur, lower, texth + 5, upper, texth + 9,
                0, 0, 255, alpha);
        SDL_SetClipRect(sur, NULL);
    }
    return 0;
}
    

int fadeSpeedExtraDraw(QPixmap *image, bool disabled)
{
    int texth = TTF_FontHeight(winStndrd);

    if (!image)
    {
        return 8;
    }

    int lower = 8;
    int upper = image->w - 11;
    filledTrigonRGBA(image, lower, texth + 9, upper, texth + 3, upper,
            texth + 9, 128, 128, 128, 255);
    aatrigonRGBA(image, lower, texth + 9, upper, texth + 3, upper,
            texth + 9, 128, 128, 128, 255);
    return 0;
}
    

static Widget *indicatorPropertiesDialog = NULL;

static void createIndicatorPropertiesDialog(QWidget **c)
{
    if (indicatorPropertiesDialog)
    {
        indicatorPropertiesDialog->bring_to_front();
        return;
    }

    int sectionOffset = 10;
    int itemOffset = 0;
    int itemBuffer = 4;
    QWidget *item = NULL;

    Window *win = new Window(0, 1, 55 + shboxh, 150, 215,
            "Indicator Visibility");
    win->setGlobalPointer(&indicatorPropertiesDialog);
    
    int winYStart = 35;
    int awidth = win->get_width() - 20;

    itemOffset = winYStart;
    item = new Checkbox(CHK_GLOW, 10, itemOffset, "Glow when active",
            &glow_active_aids, redraw_indicators, win);
    itemOffset += item->get_height() + itemBuffer + sectionOffset;

    item = new TextArea(10, itemOffset, awidth, "Inactive fade speed:", win);
    itemOffset += item->get_height() + itemBuffer;
    Slider *slider = new Slider(10, itemOffset, awidth,
            &fade_aid_amount, 0, 60, fade_settings_callback,
            fadeSpeedExtraDraw, win);
    slider->setLabels("Never", "Instantly");
    slider->setPositions(5);
    itemOffset += slider->get_height() + itemBuffer + sectionOffset;
    
    item = new TextArea(5, itemOffset, awidth, "Minimum Fade Level:", win);
    itemOffset += item->get_height() + itemBuffer;
    slider = new Slider(5, itemOffset, awidth,
            &placement_aid_base_alpha, 0, 255, redraw_indicators,
            fadeLevelExtraDraw, win);
    slider->setLabels("Low", "High");
    slider->setPositions(7);
    itemOffset += slider->get_height() + itemBuffer;
    
}


static void selectModeButton(GraphLayout::Mode mode)
{
    SDLGui::changeWidgetStates(BUT_L_ORGANIC, SDLGui::WIDGET_false);
    SDLGui::changeWidgetStates(BUT_L_FLOW,    SDLGui::WIDGET_false);
    SDLGui::changeWidgetStates(BUT_L_LAYERED, SDLGui::WIDGET_false);
    switch(mode) {
        case GraphLayout::ORGANIC:
            SDLGui::changeWidgetStates(BUT_L_ORGANIC, SDLGui::WIDGET_true);
            showDownwardSepSlider(false);
            break;
        case GraphLayout::FLOW:
            SDLGui::changeWidgetStates(BUT_L_FLOW,    SDLGui::WIDGET_true);
            showDownwardSepSlider(true);
            break;
        case GraphLayout::LAYERED:
            SDLGui::changeWidgetStates(BUT_L_LAYERED, SDLGui::WIDGET_true);
            showDownwardSepSlider(false);
            break;
        default:
            break;
    }
}

static void layoutModeButtonCallback(QWidget **c)
{
    assert(c != NULL);
    Button *button = dynamic_cast<Button *> (*c);
    int ID = button->getIdentifier();
    GraphLayout::Mode mode = GraphLayout::ORGANIC;
    GraphLayout* gl=GraphLayout::getInstance();
    switch (ID)
    {
        case BUT_L_ORGANIC:
            mode = GraphLayout::ORGANIC;
            break;
        case BUT_L_FLOW:
            mode = GraphLayout::FLOW;
            break;
        case BUT_L_LAYERED:
            mode = GraphLayout::LAYERED;
            break;
        default:
            break;
    }
    
    selectModeButton(mode);
    gl->setLayoutMode(mode);
    fully_restart_graph_layout(c);
}


static void visinfo_action(QWidget **c)
{
    router->printInfo();
}
#endif


void load_diagram(Canvas *canvas, const QString& filename)
{
    if (filename.isEmpty())
    {
        ins_init("untitled.svg");
        return;
    }
    ins_init(filename);

    QFileInfo fileInfo(filename);
    if ((fileInfo.completeSuffix() == "svg") ||
            (fileInfo.completeSuffix() == "exd"))
    {
        Avoid::db_printf("Loading diagram `%s'...\n",
                qPrintable(fileInfo.absoluteFilePath()));
        canvas->loadSvgDiagram(fileInfo.absoluteFilePath());
        canvas->set_filename(filename);
    }
    else if (fileInfo.completeSuffix() == "gml")
    {
        Avoid::db_printf("Loading GML Graph `%s'...\n",
                qPrintable(fileInfo.absoluteFilePath()));
        canvas->loadGmlDiagram(fileInfo.absoluteFilePath());
        canvas->set_filename(filename);
    }
    else
    {
        qFatal("File \"%s\" does not have a valid extention.",
               qPrintable(filename));
    }
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
