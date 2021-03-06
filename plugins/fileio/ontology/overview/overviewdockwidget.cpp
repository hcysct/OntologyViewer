#include "overviewdockwidget.h"
#include "ui_overviewdockwidget.h"
#include "owlclass.h"
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>

using namespace std;

OverviewDockWidget::OverviewDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::OverviewDockWidget)
{
    ui->setupUi(this);
    setWindowTitle("Ontology Overview");
    m_scene = new OverviewScene();
    ui->dockWidget->setScene(m_scene);
    m_view = ui->dockWidget;

    this->originalSeceneSize = m_scene->sceneRect();

    ui->comboBox_LayoutMethod->addItem("Orthogonal Tree");
    ui->comboBox_LayoutMethod->addItem("Tree");    
    ui->comboBox_LayoutMethod->addItem("Radial Tree(90)");
    ui->comboBox_LayoutMethod->addItem("Radial Tree(180)");
    ui->comboBox_LayoutMethod->addItem("FMS");
    ui->comboBox_LayoutMethod->addItem("FMS(Projection)");

    ui->comboBox_Direction->addItem("L->R");
    ui->comboBox_Direction->addItem("R->L");
    ui->comboBox_Direction->addItem("T->B");
    ui->comboBox_Direction->addItem("B->T");

    ui->lineEdit_dvn->setText("40");
    ui->lineEdit_ovn->setText("300");
    ui->lineEdit_ovn->setValidator(new QIntValidator(1,99999));
    ui->lineEdit_dvn->setValidator(new QIntValidator(1,99999));

    connect(ui->comboBox_LayoutMethod,SIGNAL(activated(QString)),this,SLOT(layoutMethodChanged(QString)));
    connect(ui->comboBox_Direction,SIGNAL(activated(QString)),this,SLOT(layoutDirectionChanged(QString)));
//    connect(m_scene,SIGNAL(myclick(QPointF)),this,SLOT(sceneClicked(QPointF)));
//    connect(ui->lineEdit_ovn,SIGNAL(textEdited(QString)),this,SLOT(le_ovnChange(QString)));
//    connect(ui->lineEdit_dvn,SIGNAL(textEdited(QString)),this,SLOT(le_dvnChange(QString)));
    connect(ui->btn_Go,SIGNAL(clicked()),this,SLOT(btnGo_Clicked()));
    connect(m_scene,SIGNAL(sceneRectChanged(QRectF)),this,SLOT(sceneResized(QRectF)));
    m_scene->setBackgroundBrush(QBrush(QColor(189, 189, 223)));

    this->ontology = NULL;
    this->highlightpolygon = NULL;
    this->ani_group = new QParallelAnimationGroup;

    QPen p = QPen(QColor("brown"));
    p.setWidth(2);
    hoverCircle = m_scene->addEllipse(0,0,12,12,p,QBrush(Qt::NoBrush));
    hoverCircle->setVisible(false);
}

OverviewDockWidget::~OverviewDockWidget()
{
    delete ui;
}

void OverviewDockWidget::setOntology(OwlOntology *onto){
    this->ontology = onto;
}

void OverviewDockWidget::addOverviewLine(OwlClass *start, OwlClass *end, QPen pen)
{
    qreal sx = start->overviewshape->pos().rx();
    qreal sy = start->overviewshape->pos().ry();
    qreal ex = end->overviewshape->pos().rx();
    qreal ey = end->overviewshape->pos().ry();
    QGraphicsItem *it = m_scene->addLine(sx,sy,ex,ey,pen);
    lines.append(it);
}

void OverviewDockWidget::addLine(qreal sx, qreal sy, qreal ex, qreal ey, QPen pen)
{
    QGraphicsItem *it = this->m_scene->addLine(sx,sy,ex,ey,pen);
    lines.append(it);
}

void OverviewDockWidget::addTreeConnector(DPolyline pl,QPen pen)
{
    ListConstIterator<DPoint> iter;
    qreal x0 = -167;
    qreal y0 = -167;
    for (iter = pl.begin(); iter.valid(); ++iter) {
        qreal x = (*iter).m_x;
        qreal y = (*iter).m_y;
        if(x0!=-167&&y0!=-167){
            QGraphicsItem *it = m_scene->addLine(x0,y0,x,y,pen);
            lines.append(it);
        }
        x0=x;
        y0=y;
    }
}

QRectF OverviewDockWidget::fixSceneRect(){
    QRectF rect;
    for(int i=0;i<m_scene->items().size();i++){
        if(m_scene->items()[i]!=this->highlightpolygon)
        rect |= m_scene->items()[i]->boundingRect();
    }
    rect = rect.adjusted(-20,-20,20,20);
    this->m_scene->setSceneRect(rect);
    return rect;
}

int OverviewDockWidget::getGItemID(QString shortname)
{
    for(int i=0;i<gitems.length();i++){
        if(gitems[i]->toolTip().toLower()==shortname.toLower())
            return i;
    }
    return -1;
}

void OverviewDockWidget::animationPre(){
    ani_group->clear();
}

void OverviewDockWidget::animationStart(){
    ani_group->start();
    for(int i=0;i<this->hideclasses.size();i++){
        this->hideclasses[i]->setVisible(false);
    }
    this->fixSceneRect();
    this->scaling();
}

void OverviewDockWidget::clearall()
{
//    this->m_scene->clear();
    for(int i=0;i<lines.size();i++){
        m_scene->removeItem(lines[i]);
    }
    lines.clear();
    for(int i=0;i<this->hideclasses.size();i++){
        this->hideclasses[i]->setVisible(true);
    }
    hideclasses.clear();
    this->highlightpolygon = NULL;
    for(int i=0;i<searchResultHighlights.size();i++)
        m_scene->removeItem(searchResultHighlights[i]);
    this->searchResultHighlights.clear();

}

void OverviewDockWidget::clearallitems()
{    
//    for(int i=0;i<gitems.size();i++)m_scene->removeItem(gitems[i]);
    ani_group->clear();
    oripos.clear();
    oriabspos.clear();
    gitems.clear();
    gitem_status.clear();
    lines.clear();
    hideclasses.clear();
    highlightpolygon=NULL;
    searchResultHighlights.clear();
    this->m_scene->clear();

    QPen p = QPen(QColor("brown"));
    p.setWidth(2);
    hoverCircle = m_scene->addEllipse(0,0,12,12,p,QBrush(Qt::NoBrush));
    hoverCircle->setVisible(false);
}


void OverviewDockWidget::addOverviewShape(OwlClass *cls)
{
    qreal x = cls->overviewshape->pos().rx();
    qreal y = cls->overviewshape->pos().ry();
    int stat = cls->overviewshape->getStatus();

    int gid = getGItemID(cls->shortname);
    if(gid!=-1){
        if(stat==gitem_status[gid]){
            QPropertyAnimation * ani = new QPropertyAnimation(gitems[gid], "pos");
            ani->setDuration(600);
            ani->setStartValue(oripos[gid]);
            qreal ex = oripos[gid].rx() + (x - oriabspos[gid].rx());
            qreal ey = oripos[gid].ry() + (y - oriabspos[gid].ry());
            ani->setEndValue(QPointF(ex, ey));
            oripos[gid]=QPointF(ex,ey);
            oriabspos[gid]=QPointF(x,y);
            ani_group->addAnimation(ani);

            return;
        }
        else{
            m_scene->removeItem(gitems[gid]);
            gitems.removeAt(gid);
            gitem_status.removeAt(gid);
            oripos.removeAt(gid);
            oriabspos.removeAt(gid);
        }
    }

    OverviewShape *its = NULL;
    switch(stat)
    {
    case OverviewClassShape::STATUS_Hide:
        break;
    case OverviewClassShape::STATUS_OutDetailview:
//        it = m_scene->addRect(x-2,y-2,4,4,QPen(QColor("black")),QBrush(QColor("gray")));
        its = new OverviewShape();
        its->setRect(x-2,y-2,4,4);
        its->setPen(QPen(QColor("black")));
        its->setBrush(QBrush(QColor("gray")));
        m_scene->addItem(its);

//        cout<<"x,y = "<<x<<","<<y<<endl;
//        cout<<"POS = "<<its->pos().rx()<<","<<its->pos().ry()<<endl;

        break;
    case OverviewClassShape::STATUS_InDetailview_Default:
//        it = m_scene->addRect(x-3,y-3,6,6,QPen(QColor("black")),QBrush(OwlClass::CLASS_SHAPE_COLOR));
        its = new OverviewShape();
        its->setRect(x-3,y-3,6,6);
        its->setPen(QPen(QColor("black")));
        its->setBrush(QBrush(OwlClass::CLASS_SHAPE_COLOR));
        m_scene->addItem(its);

        break;
    case OverviewClassShape::STATUS_InDetailview_Focused:
//        it = m_scene->addRect(x-3,y-3,6,6,QPen(QColor("black")),QBrush(OwlClass::CLASS_SHAPE_FOCUSED_COLOR));
        its = new OverviewShape();
        its->setRect(x-3,y-3,6,6);
        its->setPen(QPen(QColor("black")));
        its->setBrush(QBrush(OwlClass::CLASS_SHAPE_FOCUSED_COLOR));
        m_scene->addItem(its);
        break;
    case OverviewClassShape::STATUS_InDetailview_SubFocused:
//        it = m_scene->addRect(x-3,y-3,6,6,QPen(QColor("black")),QBrush(OwlClass::SUBCLASS_SHAPE_FOCUSED_COLOR));
        its = new OverviewShape();
        its->setRect(x-3,y-3,6,6);
        its->setPen(QPen(QColor("black")));
        its->setBrush(QBrush(OwlClass::SUBCLASS_SHAPE_FOCUSED_COLOR));
        m_scene->addItem(its);
        break;
    case OverviewClassShape::STATUS_InDetailview_SuperFocused:
//        it = m_scene->addRect(x-3,y-3,6,6,QPen(QColor("black")),QBrush(OwlClass::SUPERCLASS_SHAPE_FOCUSED_COLOR));
        its = new OverviewShape();
        its->setRect(x-3,y-3,6,6);
        its->setPen(QPen(QColor("black")));
        its->setBrush(QBrush(OwlClass::SUPERCLASS_SHAPE_FOCUSED_COLOR));
        m_scene->addItem(its);
        break;
    case OverviewClassShape::STATUS_COMPACT:
    {
        QGraphicsTextItem * io = new QGraphicsTextItem();
        io->setPos(x-8,y-8);
        QFont font;
        font.setPixelSize(7);
        font.setBold(false);
        font.setFamily("Calibri");
        io->setFont(font);
        io->setDefaultTextColor(QColor("black"));

        QString s = QString::number(cls->subclasses.size());
        io->setPlainText(s);

        QGraphicsItem * it = m_scene->addRect(x-6,y-4,12,8,QPen(QColor("black")),QBrush(QColor("pink"),Qt::LinearGradientPattern));
        m_scene->addItem(io);
        this->lines.append(io);
        this->lines.append(it);

        for(int i=0;i<cls->subclasses.size();i++){
            double sx = x+0.0001*i;
            double sy = y+0.0001*i;
            cls->subclasses[i]->overviewshape->setCentrePos(QPointF(sx,sy));
            this->addOverviewShape(cls->subclasses[i]);
            int gidx = getGItemID(cls->subclasses[i]->shortname);
            if(gidx!=-1)this->hideclasses.append(gitems[gidx]);
        }
    }
        break;
    default:
        break;
    }
    if(its != NULL){
        its->setToolTip(cls->shortname);
        oripos.append(its->pos());
        oriabspos.append(QPointF(x,y));
        gitems.append(its);
        gitem_status.append(stat);
    }
}


//void OverviewDockWidget::addOverviewShape(OwlClass *cls)
//{
//    qreal x = cls->overviewshape->pos().rx();
//    qreal y = cls->overviewshape->pos().ry();

//    int stat = cls->overviewshape->getStatus();
//    QGraphicsItem *it;
//    switch(stat)
//    {
//    case OverviewClassShape::STATUS_Hide:
//// this->setFillColour(QColor("gray"));
//// this->setSize(QSizeF(1,1));
//        break;
//    case OverviewClassShape::STATUS_OutDetailview:
//// this->setFillColour(QColor("gray"));
//// this->setSize(QSizeF(4,4));
//        it = m_scene->addRect(x-2,y-2,4,4,QPen(QColor("black")),QBrush(QColor("gray")));
//        break;
//    case OverviewClassShape::STATUS_InDetailview_Default:
//// this->setFillColour(OwlClass::CLASS_SHAPE_COLOR);
//// this->setSize(QSizeF(8,8));
//        it = m_scene->addRect(x-3,y-3,6,6,QPen(QColor("black")),QBrush(OwlClass::CLASS_SHAPE_COLOR));
//        break;
//    case OverviewClassShape::STATUS_InDetailview_Focused:
//// this->setFillColour(OwlClass::CLASS_SHAPE_FOCUSED_COLOR);
//// this->setSize(QSizeF(8,8));
//        it = m_scene->addRect(x-3,y-3,6,6,QPen(QColor("black")),QBrush(OwlClass::CLASS_SHAPE_FOCUSED_COLOR));
//        break;
//    case OverviewClassShape::STATUS_InDetailview_SubFocused:
//// this->setFillColour(OwlClass::SUBCLASS_SHAPE_FOCUSED_COLOR);
//// this->setSize(QSizeF(8,8));
//        it = m_scene->addRect(x-3,y-3,6,6,QPen(QColor("black")),QBrush(OwlClass::SUBCLASS_SHAPE_FOCUSED_COLOR));
//        break;
//    case OverviewClassShape::STATUS_InDetailview_SuperFocused:
//// this->setFillColour(OwlClass::SUPERCLASS_SHAPE_FOCUSED_COLOR);
//// this->setSize(QSizeF(8,8));
//        it = m_scene->addRect(x-3,y-3,6,6,QPen(QColor("black")),QBrush(OwlClass::SUPERCLASS_SHAPE_FOCUSED_COLOR));
//        break;
//    case OverviewClassShape::STATUS_COMPACT:
//    {
//        QGraphicsTextItem * io = new QGraphicsTextItem;
//        io->setPos(x-8,y-8);
//        QFont font;
//        font.setPixelSize(7);
//        font.setBold(false);
//        font.setFamily("Calibri");
//        io->setFont(font);
//        io->setDefaultTextColor(QColor("black"));

//        QString s = QString::number(cls->subclasses.size());
//        io->setPlainText(s);

//        it = m_scene->addRect(x-6,y-4,12,8,QPen(QColor("black")),QBrush(QColor("pink"),Qt::LinearGradientPattern));
//        m_scene->addItem(io);

//        for(int i=0;i<cls->subclasses.size();i++){
//            double sx = x+0.0001*i;
//            double sy = y+0.0001*i;
//            cls->subclasses[i]->overviewshape->setCentrePos(QPointF(sx,sy));
//        }
//    }
//        break;
//    default:
//        break;
//    }
//    if(it != NULL)it->setToolTip(cls->shortname);
//}




void OverviewDockWidget::sceneClicked(QPointF pos)
{
//    std::cout<<"Scene Clicked:"<<pos.rx()<<","<<pos.ry()<<std::endl;
    m_centerpos = pos;
    QColor grey(0, 0, 0, 60);
    QRectF viewRect = QRectF(m_centerpos.rx()-40,m_centerpos.ry()-30,80,60);
    QPolygon polygon = QPolygon(m_scene->sceneRect().toRect()).subtracted(
            QPolygon(viewRect.toRect()));
    QGraphicsItem *it = m_scene->addPolygon(polygon,QPen(Qt::transparent),QBrush(grey));
    lines.append(it);
}

void OverviewDockWidget::sceneResized(QRectF r)
{
    this->scaling();
}

void OverviewDockWidget::layoutMethodChanged(QString method)
{
//    if(method.left(11)=="Radial Tree"||method=="FMS"){
//        ui->comboBox_Direction->setHidden(true);
//    }
//    else ui->comboBox_Direction->setHidden(false);
    emit(this->layoutChanged(method));
}
void OverviewDockWidget::layoutDirectionChanged(QString dir)
{
    emit(this->directionChanged(dir));
}

void OverviewDockWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED (event)

    this->scaling();
}

void OverviewDockWidget::scaling()
{
    //drawing Rect is the window size for the overview.
    //diagramBounds is the rectangle that encloses the entire network.
    QRectF drawingRect = QRectF(m_view->rect());
    QRectF diagramBounds = m_scene->sceneRect();
    // Compute the scale to with the drawing into the overview rect.
    qreal xscale = drawingRect.width() / diagramBounds.width();
    qreal yscale = drawingRect.height() /  diagramBounds.height();

    // Choose the smallest of the two scale values.
    qreal scale = std::min(xscale, yscale);
    // Scale uniformly, and transform to center in the overview.
    QTransform scaleTransform = QTransform::fromScale(scale, scale);
    QRectF targetRect = scaleTransform.mapRect(diagramBounds);
    QPointF diff = drawingRect.center() - targetRect.center();
    QTransform m_transform = QTransform();
    m_transform.translate(diff.x(), diff.y());
    m_transform.scale(scale, scale);

    m_view->setTransform(m_transform);
}

void OverviewDockWidget::highlightItems(QList<OwlClass *> cls)
{
    if(this->highlightpolygon!=NULL)m_scene->removeItem(highlightpolygon);
    QColor grey(0, 0, 0, 60);
    QPolygon polygon = QPolygon(m_scene->sceneRect().toRect());
    for(int i=0;i<cls.size();i++){
        double x = cls[i]->overviewshape->pos().rx() - 10;
        double y = cls[i]->overviewshape->pos().ry() - 10;
        QRectF viewRect = QRectF(x,y,20,20);
        polygon = polygon.subtracted(QPolygon(viewRect.toRect()));
    }

    highlightpolygon = m_scene->addPolygon(polygon,QPen(Qt::transparent),QBrush(grey));
    lines.append(highlightpolygon);
}

void OverviewDockWidget::highlightSearchResultItems(QList<OwlClass *> cls)
{
    if(!this->searchResultHighlights.empty()){
        for(int i=0;i<searchResultHighlights.size();i++)
            m_scene->removeItem(searchResultHighlights[i]);
        searchResultHighlights.clear();
    }
    for(int i=0;i<cls.size();i++){
        double x = cls[i]->overviewshape->pos().rx()-4.2;
        double y = cls[i]->overviewshape->pos().ry()-4.2;
        QRectF viewRect = QRectF(x,y,8,8);
        QPen p(Qt::transparent);
        QBrush b(QColor("orange"),Qt::SolidPattern);

        QGraphicsItem *it = m_scene->addRect(viewRect,p,b);
        QGraphicsOpacityEffect *ef = new QGraphicsOpacityEffect(this);
        ef->setOpacity(0.8);
        it->setGraphicsEffect(ef);

        searchResultHighlights.append(it);
    }
}

void OverviewDockWidget::circleItem(QString shortname)
{
    for(int i = 0;i<gitems.size();i++){
        if(gitems[i]->toolTip()==shortname){
            QPointF p = oriabspos[i];
            this->hoverCircle->setPos(p.rx()-6,p.ry()-6);
            this->hoverCircle->setVisible(true);
            break;
        }
    }
}

void OverviewDockWidget::le_ovnChange(QString ovn)
{
    int n = ovn.toInt();
    emit this->setOverviewNodeNumber(n);
}

void OverviewDockWidget::le_dvnChange(QString ovn)
{
    int n = ovn.toInt();
    emit this->setDetailviewNodeNumber(n);
}

void OverviewDockWidget::btnGo_Clicked()
{
    if(ui->lineEdit_ovn->text().trimmed() == "")return;
    if(ui->lineEdit_dvn->text().trimmed() == "")return;
    int ovn = ui->lineEdit_ovn->text().toInt();
    int dvn = ui->lineEdit_dvn->text().toInt();
    if(!ovn>0)ovn =1;
    if(!dvn>0)dvn =1;
    emit this->setNumber(ovn,dvn);
}
