#include "owlontology.h"

#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/pluginshapefactory.h"

#include <java_magic.h>
#include <edu_monash_infotech_OWLAPIWrapper.h>
#include <iostream>
#include <stack>
#include <libdunnartcanvas/canvas.h>

using namespace dunnart;
using namespace std;

OwlOntology::OwlOntology(Canvas *canvas, QMainWindow *mainwin)
{
    this->maincanvas = canvas;
    this->currentfocusedclassidx = -1;
    this->appmainwindow = mainwin;

    wid = new DetailDockWidget();
    this->appmainwindow->addDockWidget(Qt::LeftDockWidgetArea,wid);
    wid->setWindowTitle("Equivalent Class");
    wid->show();

    zoomdwgt = new ZoomDockWidget();
    this->appmainwindow->addDockWidget(Qt::LeftDockWidgetArea,zoomdwgt);
    zoomdwgt->show();

    filterdwgt = new FilterDockWidget();
    appmainwindow->addDockWidget(Qt::RightDockWidgetArea,filterdwgt);
    filterdwgt->show();

    historydwgt = new HistoryDockWidget();
    appmainwindow->addDockWidget(Qt::RightDockWidgetArea,historydwgt);
    historydwgt->show();

    searchdwgt = new SearchDockWidget();
    appmainwindow->addDockWidget(Qt::RightDockWidgetArea,searchdwgt);
    searchdwgt->show();

    infoboxdwgt = new InformationBoxDockWidget();
    appmainwindow->addDockWidget(Qt::RightDockWidgetArea,infoboxdwgt);
    infoboxdwgt->show();

    //temp TODO create new widget (QListWidget)
    InformationBoxDockWidget * detailinforofnode = new InformationBoxDockWidget();
    appmainwindow->addDockWidget(Qt::LeftDockWidgetArea,detailinforofnode);
    detailinforofnode->show();
}

/** TODO List:
1. --all URI name ---only one namespace!!!
2. --prefix---only one namespace!!!
3. --properties --done
4. --about Thing?
5. --(done) Did not deal with the -1 error when use getIndexOfClasses(),getIndexOfIndividuals(),getIndexOfProperties()
6. --(done) drawEquivalentClass() and getFormula() has not fully covered all the expressions
7. link different views
*/

//const
const QColor OwlOntology::CLASS_SHAPE_COLOR = QColor(65,105,225);
const QColor OwlOntology::INDIVIDUAL_SHAPE_COLOR = QColor(238,130,238);
const QColor OwlOntology::PROPERTY_SHAPE_COLOR = QColor(143,188,143);

const QColor OwlOntology::CLASS_CONNECTOR_COLOR = QColor("blue");
const QColor OwlOntology::INDIVIDUAL_CONNECTOR_COLOR = QColor("purple");
const QColor OwlOntology::PROPERTY_CONNECT_TO_CLASS_COLOR = QColor("yellow");
const QColor OwlOntology::PROPERTY_CONNECTOR_COLOR = QColor("green");

//get index of individuals by its shortname
int OwlOntology::getIndexOfIndividuals(QString shortname)
{
    for(int i=0;i<individuals.length();i++)
    {
        if(individuals[i]->shortname == shortname) return i;
    }
    return -1;
}

//get index of classes by its shortname
int OwlOntology::getIndexOfClasses(QString shortname)
{
    for(int i=0;i<classes.length();i++)
    {
        if(classes[i]->shortname == shortname) return i;
    }
    return -1;
}

//get index of properties by its shortname
int OwlOntology::getIndexOfProperties(QString shortname)
{
    for(int i=0;i<properties.length();i++)
    {
        if(properties[i]->shortname == shortname) return i;
    }
    return -1;
}

//load ontology using owlapi via OWLAPIWrapper
void OwlOntology::loadontology(const QFileInfo& fileInfo)
{
    //init JVM
    initJavaWrapper(0,NULL);
    //init OWLAPIWrapper class
    edu::monash::infotech::OWLAPIWrapper * wp = new edu::monash::infotech::OWLAPIWrapper();
    //load owl file
    QString filename = fileInfo.absoluteFilePath();
    this->ontologyname = fileInfo.baseName();
    wp->loadOntologyFile(filename.toUtf8().constData());

    /** get owl namespace (Warn: only one namespace was handled!) **/
    //get namespace for the URI
    QString owlnamespace = wp->getDefaultNameSpace();

    //get all individuals
    JavaObjectArray *resGetAllIndividuals = wp->getAllIndividuals();
    java::lang::String ** owlindividuals = (java::lang::String **)resGetAllIndividuals->getArrayData();
    for(int i=0;i<resGetAllIndividuals->getArrayLength();i++)
    {
        cout<<owlindividuals[i]->toString()<<endl;
        //set individual name
        OwlIndividual * tmpindividual = new OwlIndividual();
        tmpindividual->shortname=QString(owlindividuals[i]->toString());
        tmpindividual->URI="<" + owlnamespace + tmpindividual->shortname + ">";

        //create shape
        tmpindividual->shape = new OntologyIndividualShape();
        tmpindividual->shape->setIdString(tmpindividual->shortname);
        tmpindividual->shape->setLabel(tmpindividual->shortname);
        tmpindividual->shape->setToolTip(tmpindividual->URI);
        tmpindividual->shape->setSize(QSizeF(150,20));
        tmpindividual->shape->setFillColour(this->INDIVIDUAL_SHAPE_COLOR);


        //append tmpindividual to list
        this->individuals.append(tmpindividual);
    }

    //get all ontology classes from JVM
    JavaObjectArray *resGetAllOWLClasses = wp->getAllOWLClasses();
    java::lang::String ** owlclasses = (java::lang::String **)resGetAllOWLClasses->getArrayData();
    for(int i=0;i<resGetAllOWLClasses->getArrayLength();i++)
    {

        //set class name (???prefix,short,full name???)
        OwlClass * tmpclass=new OwlClass();
        tmpclass->shortname=QString(owlclasses[i]->toString());
        tmpclass->URI = "<" + owlnamespace + tmpclass->shortname + ">";

        //create shape
        tmpclass->shape = new OntologyClassShape();
        tmpclass->shape->setIdString(tmpclass->shortname);
        tmpclass->shape->setToolTip(tmpclass->URI);
        tmpclass->shape->setPosAndSize(QPointF(0,i*25),QSizeF(150,20));        
        tmpclass->shape->setMyLabel(tmpclass->shortname);
        tmpclass->shape->setLabelByLevels(1,tmpclass->shortname); //set level 1 label
        tmpclass->shape->setFillColour(this->CLASS_SHAPE_COLOR);

        //connect signals
        connect(tmpclass->shape,SIGNAL(myclick(OntologyClassShape*)),this,SLOT(ontoclass_clicked(OntologyClassShape*)));
        connect(tmpclass->shape,SIGNAL(myDoubleClick(OntologyClassShape*)),this,SLOT(ontoclass_doubleclicked(OntologyClassShape*)));
        connect(tmpclass->shape,SIGNAL(myRightClick(OntologyClassShape*)),this,SLOT(ontoclass_rightclicked(OntologyClassShape*)));


        //get equivalent class (Warn!!! only 1 or more???)
        QString tmpstr = QString(wp->getEquivalentClasses(tmpclass->shortname.toLocal8Bit().data()));
        tmpclass->equivalentclass = tmpstr;
        tmpclass->shape->setLabelByLevels(6,"[EQU]:"+ this->getFormula(tmpstr)); //set level 6 label

        //append tmpclass to list
        this->classes.append(tmpclass);
    }

    //get and set the subclasses, superclasses, disjointclasses and individuals of classes
    for(int i=0;i<classes.length();i++)
    {
        //get&set subclasses
        QString substr = "[SUB]:"; //for level 2 label
        JavaObjectArray *resGetSubClasses = wp->getSubClasses(owlclasses[i]->toString());
        if(resGetSubClasses!=NULL){
            java::lang::String ** owlsubclasses = (java::lang::String **)resGetSubClasses->getArrayData();

            for(int j=0;j<resGetSubClasses->getArrayLength();j++){
                substr += QString(owlsubclasses[j]->toString())+" "; //add to level 2 label
                int idx = getIndexOfClasses(owlsubclasses[j]->toString());
                if(idx!=-1)classes[i]->subclasses<<classes[idx];

                //create connectors
                Connector * conn = new Connector();
                conn->initWithConnection(classes[idx]->shape,classes[i]->shape);
                conn->setDirected(true);
                conn->setColour(CLASS_CONNECTOR_COLOR);

                classes[i]->classesconnectors<<conn;
                classes[idx]->classesconnectors<<conn;
            }            
        }
        classes[i]->shape->setLabelByLevels(3,substr); //set level 3 label

        //get&set superclasses
        QString supstr= "[SUP]:";
        JavaObjectArray *resGetSuperClasses = wp->getSuperClasses(owlclasses[i]->toString());
        if(resGetSuperClasses!=NULL){
            java::lang::String ** owlsuperclasses = (java::lang::String **)resGetSuperClasses->getArrayData();
            for(int j=0;j<resGetSuperClasses->getArrayLength();j++){
                supstr += QString(owlsuperclasses[j]->toString())+" "; //add to level 3 label
                int idx = getIndexOfClasses(owlsuperclasses[j]->toString());
                if(idx!=-1)classes[i]->superclasses<<classes[idx];
            }            
        }
        classes[i]->shape->setLabelByLevels(4,supstr); //set level 4 label

        //get&set disjointclasses
        QString disstr= "[DIS]:";
        JavaObjectArray *resGetDisjointClasses = wp->getDisjointClasses(owlclasses[i]->toString());
        if(resGetDisjointClasses!=NULL){
            java::lang::String ** owldisjointclasses = (java::lang::String **)resGetDisjointClasses->getArrayData();
            for(int j=0;j<resGetDisjointClasses->getArrayLength();j++){
                disstr += QString(owldisjointclasses[j]->toString())+" "; //add to level 4 label
                int idx = getIndexOfClasses(owldisjointclasses[j]->toString());
                if(idx!=-1)classes[i]->disjointclasses<<classes[idx];
            }            
        }
        classes[i]->shape->setLabelByLevels(5,disstr); //set level 5 label

        //get&set individuals
        QString indstr = "[IND]:";
        JavaObjectArray *resGetIndividuals = wp->getIndividuals(owlclasses[i]->toString());
        if(resGetIndividuals!=NULL){
            java::lang::String ** owlclassindividuals = (java::lang::String **)resGetIndividuals->getArrayData();
            for(int j=0;j<resGetIndividuals->getArrayLength();j++){
                indstr += QString(owlclassindividuals[j]->toString())+ " "; //add to level 2 label
                int idx = getIndexOfIndividuals(owlclassindividuals[j]->toString());
                //if found the individual in the list
                if(idx!=-1){
                    //add this class[i] to individual's owner classes list
                    this->individuals[idx]->ownerclasses.append(classes[i]->shortname);
                    //add individuals[ind] to class[i] individuals list
                    classes[i]->individuals<<this->individuals[idx];
                    //create connectors
                    Connector * conn = new Connector();
                    conn->initWithConnection(individuals[idx]->shape, classes[i]->shape);
                    conn->setDirected(true);
                    conn->setColour(this->INDIVIDUAL_CONNECTOR_COLOR);

                    classes[i]->individualconnectors<<conn;
                    individuals[idx]->classesconnectors<<conn;
                }
            }            
        }
        classes[i]->shape->setLabelByLevels(2,indstr); //set level 2 label
    }

    //get all ontology properties name and type encoded string from JVM
    JavaObjectArray *resGetAllPropertiesNameAndSubType = wp->getAllPropertiesNameAndSubType();
    if(resGetAllPropertiesNameAndSubType!=NULL){
        java::lang::String ** owlproperties = (java::lang::String **)resGetAllPropertiesNameAndSubType->getArrayData();
        for(int i=0;i<resGetAllPropertiesNameAndSubType->getArrayLength();i++){
            //set the property type, name, encoded string,and URI
            QString encodedstr = QString(owlproperties[i]->toString());
            OwlProperty * tmpproperty = new OwlProperty();

            tmpproperty->encodedPropertyNameAndType = encodedstr;
            tmpproperty->shortname = tmpproperty->getPropertyShortNameByEncodedString(encodedstr);
            tmpproperty->URI = "<" + owlnamespace + tmpproperty->shortname + ">";
            tmpproperty->propertytype = tmpproperty->getPropertyTypeByEncodedString(encodedstr);
            //create shape for property
            tmpproperty->shape = new OntologyPropertyShape();
            tmpproperty->shape->setIdString(tmpproperty->encodedPropertyNameAndType);
            tmpproperty->shape->setLabel("["+tmpproperty->propertytype +"]\n" + tmpproperty->shortname);
            tmpproperty->shape->setToolTip(tmpproperty->URI);
            tmpproperty->shape->setPosAndSize(QPointF(0,i*40),QSizeF(200,35));
            tmpproperty->shape->setFillColour(this->PROPERTY_SHAPE_COLOR);

            //add to property list
            this->properties.append(tmpproperty);
        }
    }

    //get the domains, ranges, sub/super/disjoint properties and make the internal connection of the pointers
    for(int i=0;i<this->properties.length();i++){
        //data or object property
        QString basictype="";
        if(properties[i]->isDataProperty())basictype=wp->ENTITIY_TYPE_DATA_PROPERTY;
        if(properties[i]->isObjectProperty())basictype=wp->ENTITIY_TYPE_OBJECT_PROPERTY;

        //domains -- get and connect the pointers of classes
        JavaObjectArray *resGetPropertyDomainsByName = wp->getPropertyDomainsByName(basictype.toLocal8Bit().data(),properties[i]->shortname.toLocal8Bit().data());
        if(resGetPropertyDomainsByName!=NULL){
            java::lang::String ** owlpropertydomains = (java::lang::String **)resGetPropertyDomainsByName->getArrayData();
            for(int j=0;j<resGetPropertyDomainsByName->getArrayLength();j++){
                QString domainname = owlpropertydomains[j]->toString();
                //deal with no domain
                if(domainname.trimmed()=="")continue;
                //get the domain class index
                int domainclassidx = this->getIndexOfClasses(domainname);
                //if found the class, insert the domain class pointer
                if(domainclassidx!=-1) properties[i]->domains<<classes[domainclassidx];
            }
        }

        //ranges -- only get the string since DataProperty does not has range of classes.
        //ObjectProperty ranges are the class shortnames. DataProperty range is a datatype definition
        JavaObjectArray *resGetPropertyRangesByName = wp->getPropertyRangesByName(basictype.toLocal8Bit().data(),properties[i]->shortname.toLocal8Bit().data());
        if(resGetPropertyRangesByName!=NULL){
            java::lang::String ** owlpropertyranges = (java::lang::String **)resGetPropertyRangesByName->getArrayData();
            for(int j=0;j<resGetPropertyRangesByName->getArrayLength();j++){
                QString range = owlpropertyranges[j]->toString();
                if(range.trimmed()=="")continue;
                else properties[i]->ranges<<range;
            }
        }
        //sub properties
        JavaObjectArray *resGetSubProperites = wp->getSubProperites(basictype.toLocal8Bit().data(),properties[i]->shortname.toLocal8Bit().data());
        if(resGetSubProperites!=NULL){
            java::lang::String ** owlsubproperties = (java::lang::String **)resGetSubProperites->getArrayData();
            for(int j=0;j<resGetSubProperites->getArrayLength();j++){
                int idx = getIndexOfProperties(owlsubproperties[j]->toString());
                //if found, add to list
                if(idx!=-1)properties[i]->subproperties<<properties[idx];
            }
        }
        //super properties
        JavaObjectArray *resGetSuperProperites = wp->getSuperProperites(basictype.toLocal8Bit().data(),properties[i]->shortname.toLocal8Bit().data());
        if(resGetSuperProperites!=NULL){
            java::lang::String ** owlsuperproperties = (java::lang::String **)resGetSuperProperites->getArrayData();
            for(int j=0;j<resGetSuperProperites->getArrayLength();j++){
                int idx = getIndexOfProperties(owlsuperproperties[j]->toString());
                //if found, add to list
                if(idx!=-1)properties[i]->superproperties<<properties[idx];
            }
        }
        //disjoint properties
        JavaObjectArray *resGetDisjointProperites = wp->getDisjointProperties(basictype.toLocal8Bit().data(),properties[i]->shortname.toLocal8Bit().data());
        if(resGetDisjointProperites!=NULL){
            java::lang::String ** owldisjointproperties = (java::lang::String **)resGetDisjointProperites->getArrayData();
            for(int j=0;j<resGetDisjointProperites->getArrayLength();j++){
                int idx = getIndexOfProperties(owldisjointproperties[j]->toString());
                //if found, add to list
                if(idx!=-1)properties[i]->disjointproperties<<properties[idx];
            }
        }
    }



}

//draw the ontology classes
void OwlOntology::drawClassView(Canvas *canvas)
{
    cout<<"Start drawing classes..."<<endl;
    //draw classes
    for(int i=0;i<classes.length();i++)
    {
        cout<<"Node "<<i<<". ";
        //**** do not draw thing ****
        if(classes[i]->shortname.toLower()!="thing")
            canvas->addItem(classes[i]->shape);       
    }

    cout<<"\nStart drawing edges..."<<endl;
    //draw subclasses connections
    for(int i=0;i<classes.length();i++)
    {
        cout<<"\nedges of class["<<i<<"]("
            <<classes[i]->shortname.toStdString()<<"):"
            <<"total connectors="<<classes[i]->classesconnectors.size()<<endl;
        //**** do not draw thing ****
        if(classes[i]->shortname.toLower()!="thing")
        {           
            for(int j=0;j<classes[i]->classesconnectors.size();j++)
            {   cout<<"<"<<j<<">";
                canvas->addItem(classes[i]->classesconnectors[j]);
            }
        }
    }

    //adjust postions -- level only
//    stack<OwlClass *> sts;
//    for(int i=0;i<classes.length();i++)
//    {
//        if((classes[i]->superclasses.length()==0)||(classes[i]->superclasses.length()==1&&classes[i]->superclasses[0]->shortname=="Thing"))
//        {
//            classes[i]->shape->setPosAndSize(QPointF(0,i*25),QSizeF(150,20));
//            sts.push(classes[i]);
//            //cout<<"-->Stack size:" <<sts.size() << "Top: " << classes[i]->shortname.toStdString() <<endl;
//        }
//        if(classes[i]->shortname=="Thing"){
//            classes[i]->shape->setPosAndSize(QPointF(-200,i*25),QSizeF(150,20));
//        }
//    }
//    while(!sts.empty()){
//        OwlClass * tmp = sts.top();
//        sts.pop();
//        qreal x = tmp->shape->pos().x();

//        for(int i=0;i<tmp->subclasses.length();i++)
//        {
//            qreal y = tmp->subclasses[i]->shape->pos().y();
//            tmp->subclasses[i]->shape->setPosAndSize(QPointF(x+200,y),QSizeF(150,20));
//            sts.push(tmp->subclasses[i]);
//            //cout<<"==>Stack size:" <<sts.size() << "Top: " << tmp->subclasses[i]->shortname.toStdString() <<endl;
//        }

//    }

}

//draw the ontology individuals
void OwlOntology::drawIndividualView(Canvas *canvas)
{
    Connector *conn;
    //draw individuals
    for(int i=0;i<individuals.length();i++)
    {
        canvas->addItem(individuals[i]->shape);
        //draw owner classes
        for(int j=0;j<individuals[i]->ownerclasses.length();j++)
        {
            int ownerclassidx = this->getIndexOfClasses(individuals[i]->ownerclasses[j]);
            if(ownerclassidx!=-1) canvas->addItem(this->classes[ownerclassidx]->shape);
        }
        //draw connectors
        for(int j=0;j<individuals[i]->classesconnectors.size();j++)
        {
            canvas->addItem(individuals[i]->classesconnectors[j]);
        }
    }
}

//draw the ontology properties
void OwlOntology::drawPropertyView(Canvas *canvas)
{
    Connector *conn;
    //draw properties
    for(int i=0;i<properties.length();i++){
        canvas->addItem(properties[i]->shape);
    }
    //draw domains, ranges, sub relations (super,disjoint not handled)
    for(int i=0;i<properties.length();i++){
        //domains
        for(int j=0;j<properties[i]->domains.length();j++)
        {
            //draw the domain class on the canvas(if it is duplicated, canvas will handle it?)
            canvas->addItem(properties[i]->domains[j]->shape);
            //draw a connection between domain->property
            conn = new Connector();
            conn->initWithConnection(properties[i]->domains[j]->shape, properties[i]->shape);
            canvas->addItem(conn);
            conn->setDirected(true);
            conn->setDotted(true);
            conn->setColour(this->PROPERTY_CONNECT_TO_CLASS_COLOR);
        }

        //ranges
        //dataproperty add a line to label
        if(properties[i]->isDataProperty()){
            QString lbl = properties[i]->shape->idString();
            //lbl += "\n--------------------";
            for(int j=0;j<properties[i]->ranges.length();j++)
            {
                lbl += "\n(" + properties[i]->ranges[j]+")";
            }
            properties[i]->shape->setSize(QSizeF(200,52.5));
            properties[i]->shape->setLabel(lbl);
        }
        //object property: add the range classes and connect them
        if(properties[i]->isObjectProperty()){
            for(int j=0;j<properties[i]->ranges.length();j++)
            {
                //draw the range class on the canvas(if it is duplicated, canvas will handle it?)
                int idx = this->getIndexOfClasses(properties[i]->ranges[j]);
                if(idx!=-1){
                    canvas->addItem(classes[idx]->shape);
                    //draw a connection between property->range classes
                    conn = new Connector();
                    conn->initWithConnection(properties[i]->shape, classes[idx]->shape);
                    canvas->addItem(conn);
                    conn->setDirected(true);
                    conn->setDotted(true);
                    conn->setColour(this->PROPERTY_CONNECT_TO_CLASS_COLOR);
                }
            }
        }
        //sub
        for(int j=0;j<properties[i]->subproperties.length();j++)
        {
            conn = new Connector();
            conn->initWithConnection(properties[i]->subproperties[j]->shape,properties[i]->shape);
            canvas->addItem(conn);
            conn->setDirected(true);
            conn->setColour(this->PROPERTY_CONNECTOR_COLOR);
        }
        //super, disjoint -- Need to deal with?
    }
}

//draw the ontology classes
void OwlOntology::drawClassOverview(Canvas *canvas)
{
    //draw classes
    for(int i=0;i<classes.length();i++)
    {
        canvas->addItem(classes[i]->shape);
        classes[i]->shape->setLabel(".");
        classes[i]->shape->setToolTip(classes[i]->shortname);
        classes[i]->shape->setSize(QSizeF(10,10));
    }

    //draw connections
    Connector *conn;
    for(int i=0;i<classes.length();i++)
    {
        //subclasses
        for(int j=0;j<classes[i]->subclasses.length();j++){
             conn = new Connector();
             conn->initWithConnection(classes[i]->subclasses[j]->shape,classes[i]->shape);
             canvas->addItem(conn);
             conn->setDirected(true);
         }
//        //superclasses ???
//        for(int j=0;j<classes[i]->superclasses.length();j++)
//        {
//            conn = new Connector();
//            conn->initWithConnection(classes[i]->superclasses[j]->shape,classes[i]->shape);
//            canvas->addItem(conn);
//            conn->setDirected(true);
//            conn->setColour(this->CLASS_CONNECTOR_COLOR);
//        }
    }


}

//String output
QString OwlOntology::toQString()
{
    QString res;
    //output name
    res.append("============= Ontology : ");
    res.append(ontologyname);
    res.append(" ===============");

    //output classes
    res.append("\nOntology Classes :\n");
    for(int i=0;i<classes.length();i++)res.append(classes[i]->toQString());

    //output individuals
    res.append("\nOntology individuals :\n");
    for(int i=0;i<individuals.length();i++)res.append(individuals[i]->toQString());

    //output properties
    res.append("\nOntology properties :\n");
    for(int i=0;i<properties.length();i++)res.append(properties[i]->toQString());
    return res;
}


//methods to get the logical representation.
QList<QString> OwlOntology::splitFormula(QString str)
{
    QList<QString> res;
    int s = 0;
    int e;
    int count = -1;
    for (int i = 0; i < str.length(); i++) {
        if (str.at(i) == '(' || str.at(i) == '<') {
            if (count == -1) {
                count = 1;
            } else {
                count++;
            }
        }
        if (str.at(i) == ')' || str.at(i) == '>') {
            count--;
        }

        if ((str.at(i) == ' ' || i == str.length() - 1) && count == -1) {
            e = i + 1;
            if (s != e && str.at(s) != ' ') {
                QString tmp = str.mid(s, e-s).trimmed();
                res.append(tmp);
            }

            s = i + 1;
        }
        if (count == 0) {
            e = i + 1;
            QString tmp = str.mid(s, e-s).trimmed();
            res.append(tmp);
            s = i + 1;
            count = -1;
        }
    }
    return res;
}

QString OwlOntology::getFormula(QString qstr)
{
    if(qstr.trimmed()=="")return "";
    /** Need to be verified all syntax!! **/

    QString r = "";

    int startpos=0;
    QString marker="";

    if(qstr.startsWith("Object")){
        startpos=6;
        marker="";

    }
    else if(qstr.startsWith("Data"))
    {
        startpos=4;
        marker="[D]";
    }
    else
    {
        startpos=0;
        marker="";
    }

    QString str = qstr.right(qstr.length()-startpos);
    //<> element

    if (str.startsWith("<"))
    {
        int i = str.indexOf("#",0);
        r = "<"+str.mid(i+1,str.length() - 2-i)+">";
    }

    //Syntax--both data and object
    else if (str.startsWith("OneOf"))
    {
        QString fstr = str.mid(6, str.length() - 7);
        QList<QString> substrs = splitFormula(fstr);
        r = marker + "OneOf("+getFormula(substrs.at(0));
        for (int i = 1; i < substrs.size(); i++) {
            r += " , " + getFormula(substrs.at(i));
        }
        r+=")";
    }

    else if (str.startsWith("IntersectionOf"))
    {
        QString fstr = str.mid(15, str.length() - 16);
        QList<QString> substrs = splitFormula(fstr);
        r = marker + "("+getFormula(substrs.at(0));
        for (int i = 1; i < substrs.size(); i++) {
            r += " AND " + getFormula(substrs.at(i));
        }
        r+=")";
    }

    else if (str.startsWith("UnionOf"))
    {
        QString fstr = str.mid(8, str.length() - 9);
        QList<QString> substrs = splitFormula(fstr);
        r = marker+"("+getFormula(substrs.at(0));
        for (int i = 1; i < substrs.size(); i++) {
            r += " OR " + getFormula(substrs.at(i));
        }
        r+=")";
    }

    else if (str.startsWith("ComplementOf"))
    {
        /** Need to be verified!! **/
        QString fstr = str.mid(13, str.length() - 14);
        QList<QString> substrs = splitFormula(fstr);
        r = marker+"NOT("+getFormula(substrs.at(0)) + ")";
    }

    else if (str.startsWith("AllValuesFrom"))
    {
        QString fstr = str.mid(14, str.length() - 15);
        QList<QString> substrs = splitFormula(fstr);
        r = "(" + getFormula(substrs.at(0)) +" "+marker+"ONLY " + getFormula(substrs.at(1)) +")";
    }

    else if (str.startsWith("SomeValuesFrom"))
    {
        QString fstr = str.mid(15, str.length() - 16);
        QList<QString> substrs = splitFormula(fstr);
        r = "(" + getFormula(substrs.at(0)) + " "+marker+"SOME " + getFormula(substrs.at(1)) +")";
    }

    else if (str.startsWith("HasValue"))
    {
        QString fstr = str.mid(9, str.length() - 10);
        QList<QString> substrs = splitFormula(fstr);
        r = "(" + getFormula(substrs.at(0)) + " "+marker+"VALUE " + getFormula(substrs.at(1)) +")";
    }

    else if (str.startsWith("ExactCardinality"))
    {
        QString fstr = str.mid(17, str.length() - 18);
        QList<QString> substrs = splitFormula(fstr);
        r = "(" + getFormula(substrs.at(1))+" "+marker+"EXACTLY " +getFormula(substrs.at(0)) + getFormula(substrs.at(2)) +")";
    }

    else if (str.startsWith("MinCardinality"))
    {
        QString fstr = str.mid(15, str.length() - 16);
        QList<QString> substrs = splitFormula(fstr);
        r = "(" + getFormula(substrs.at(1))+" "+marker+"MIN " +getFormula(substrs.at(0)) + getFormula(substrs.at(2)) +")";
    }

    else if (str.startsWith("MaxCardinality"))
    {
        QString fstr = str.mid(15, str.length() - 16);
        QList<QString> substrs = splitFormula(fstr);
        r = "(" + getFormula(substrs.at(1))+" "+marker+"MAX " +getFormula(substrs.at(0)) + getFormula(substrs.at(2)) +")";
    }

    //others
    else {
        r = "<"+str +">";
    }
    return r;
}

ShapeObj * OwlOntology::drawEquivalentClass(QString qstr,Canvas *canvas)
{
    ShapeObj * rshape=NULL;

    /** Need to be verified all syntax!! **/

    int startpos=0;
    QString marker="";

    if(qstr.startsWith("Object")){
        startpos=6;
        marker="[O]";

    }
    else if(qstr.startsWith("Data"))
    {
        startpos=4;
        marker="[D]";
    }
    else
    {
        startpos=0;
        marker="";
    }

    QString str = qstr.right(qstr.length()-startpos);

    //<> element ??? consider about multiple namespace!!!
    if (str.startsWith("<"))
    {
        int cidx = str.indexOf("#",0);
        QString sname = str.mid(cidx+1,str.length() - 2-cidx);

        ShapeObj * entityshape;

        int idx = -1;
        //not found entity type, check class
        if(idx==-1){
            idx=this->getIndexOfClasses(sname);
            if(idx != -1)
            {
                OntologyClassShape * classshape = new OntologyClassShape();
                classshape->setIdString(">"+classes[idx]->shortname);
                classshape->setToolTip(classes[idx]->URI);
                classshape->setSize(QSizeF(150,20));
                classshape->setMyLabel(classes[idx]->shortname);
                classshape->setFillColour(this->CLASS_SHAPE_COLOR);
                /** set level labels
                    1 shortname
                    2 individuals
                    3 sub
                    4 super
                    5 disjoint
                    6 equivalent
                **/
                for(int i=0;i<6;i++)
                    classshape->setLabelByLevels(i+1,classes[idx]->shape->levelLabels[i]);

                entityshape = classshape;
            }
        }

        //not class, check individual
        if(idx==-1){
            idx=this->getIndexOfIndividuals(sname);
            if(idx != -1)
            {
                OntologyIndividualShape * individualshape = new OntologyIndividualShape();
                individualshape->setIdString(">" + individuals[idx]->shortname);
                individualshape->setLabel(individuals[idx]->shortname);
                individualshape->setToolTip(individuals[idx]->URI);
                individualshape->setSize(QSizeF(150,20));
                individualshape->setFillColour(this->INDIVIDUAL_SHAPE_COLOR);

                entityshape = individualshape;
            }
        }

        //not class or individual, check properties
        if(idx==-1){
            idx=this->getIndexOfProperties(sname);
            if(idx != -1)
            {
                OntologyPropertyShape * propertyshape = new OntologyPropertyShape();
                propertyshape->setIdString(">"+properties[idx]->encodedPropertyNameAndType);
                propertyshape->setLabel("["+properties[idx]->propertytype +"]\n" + properties[idx]->shortname);
                propertyshape->setToolTip(properties[idx]->URI);
                propertyshape->setSize(QSizeF(200,35));
                propertyshape->setFillColour(this->PROPERTY_SHAPE_COLOR);

                entityshape = propertyshape;
            }
        }

        //not class, individual, or property
        if(idx==-1){
            entityshape = new RectangleShape();
            entityshape->setLabel(sname);
            entityshape->setFillColour(QColor("orange"));
        }
        entityshape->setSize(QSizeF(150,30));
        canvas->addItem(entityshape);
        rshape = entityshape;

    }

    //syntax -- both Object and Data
    else if (str.startsWith("OneOf"))
    {
        QString fstr = str.mid(6, str.length() - 7);
        QList<QString> substrs = splitFormula(fstr);

        ShapeObj * mshape = new RectangleShape();
        mshape->setLabel(marker + "OneOf");
        mshape->setSize(QSizeF(80,30));
        mshape->setFillColour(QColor("gray"));
        canvas->addItem(mshape);

        Connector *conn;
        for (int i = 0; i < substrs.size(); i++) {
            conn = new Connector();
            conn->initWithConnection(mshape,drawEquivalentClass(substrs.at(i),canvas));
            canvas->addItem(conn);
            conn->setDirected(true);
            conn->setColour(QColor("blue"));
        }
        rshape=mshape;

    }

    else if (str.startsWith("IntersectionOf")) {
        QString fstr = str.mid(15, str.length() - 16);
        QList<QString> substrs = splitFormula(fstr);

        ShapeObj * mshape = new RectangleShape();
        mshape->setLabel(marker+"AND");
        mshape->setSize(QSizeF(80,30));
        mshape->setFillColour(QColor("gray"));
        canvas->addItem(mshape);

        Connector *conn;
        for (int i = 0; i < substrs.size(); i++) {
            conn = new Connector();
            conn->initWithConnection(mshape,drawEquivalentClass(substrs.at(i),canvas));
            canvas->addItem(conn);
            conn->setDirected(true);
            conn->setColour(QColor("blue"));
        }
        rshape=mshape;
    }

    else if (str.startsWith("UnionOf"))
    {
        QString fstr = str.mid(8, str.length() - 9);
        QList<QString> substrs = splitFormula(fstr);

        ShapeObj * mshape = new RectangleShape();
        mshape->setLabel(marker+"OR");
        mshape->setSize(QSizeF(80,30));
        mshape->setFillColour(QColor("gray"));
        canvas->addItem(mshape);

        Connector *conn;
        for (int i = 0; i < substrs.size(); i++) {
            conn = new Connector();
            conn->initWithConnection(mshape,drawEquivalentClass(substrs.at(i),canvas));
            canvas->addItem(conn);
            conn->setDirected(true);
            conn->setColour(QColor("blue"));
        }
        rshape=mshape;
    }

    else if (str.startsWith("ComplementOf")){
        QString fstr = str.mid(13, str.length() - 14);
        QList<QString> substrs = splitFormula(fstr);

        ShapeObj * mshape = new RectangleShape();
        mshape->setLabel(marker+"NOT");
        mshape->setSize(QSizeF(80,30));
        mshape->setFillColour(QColor("gray"));
        canvas->addItem(mshape);

        Connector *conn;
        for (int i = 0; i < substrs.size(); i++) {
            conn = new Connector();
            conn->initWithConnection(mshape,drawEquivalentClass(substrs.at(i),canvas));
            canvas->addItem(conn);
            conn->setDirected(true);
            conn->setColour(QColor("blue"));
        }
        rshape=mshape;
    }

    else if (str.startsWith("AllValuesFrom")) {
        QString fstr = str.mid(14, str.length() - 15);
        QList<QString> substrs = splitFormula(fstr);

        ShapeObj * mshape = new RectangleShape();
        mshape->setLabel(marker+"ONLY");
        mshape->setSize(QSizeF(80,30));
        mshape->setFillColour(QColor("gray"));
        canvas->addItem(mshape);

        ShapeObj * shape1 = drawEquivalentClass(substrs.at(0),canvas);
        ShapeObj * shape2 = drawEquivalentClass(substrs.at(1),canvas);
        Connector *conn;
        conn = new Connector();
        conn->initWithConnection(mshape,shape1);
        canvas->addItem(conn);
        conn->setDirected(true);
        conn->setColour(QColor("blue"));

        conn = new Connector();
        conn->initWithConnection(mshape,shape2);
        canvas->addItem(conn);
        conn->setLabel("Only");
        conn->setDirected(true);
        conn->setColour(QColor("blue"));

        rshape = mshape;
    }

    else if (str.startsWith("SomeValuesFrom")) {
        QString fstr = str.mid(15, str.length() - 16);
        QList<QString> substrs = splitFormula(fstr);

        ShapeObj * mshape = new RectangleShape();
        mshape->setLabel(marker+"SOME");
        mshape->setSize(QSizeF(80,30));
        mshape->setFillColour(QColor("gray"));
        canvas->addItem(mshape);

        ShapeObj * shape1 = drawEquivalentClass(substrs.at(0),canvas);
        ShapeObj * shape2 = drawEquivalentClass(substrs.at(1),canvas);
        Connector *conn;
        conn = new Connector();
        conn->initWithConnection(mshape,shape1);
        canvas->addItem(conn);
        conn->setDirected(true);
        conn->setColour(QColor("blue"));

        conn = new Connector();
        conn->initWithConnection(mshape,shape2);
        canvas->addItem(conn);
        conn->setLabel("Some");
        conn->setDirected(true);
        conn->setColour(QColor("blue"));

        rshape = mshape;
    }

    else if (str.startsWith("HasValue")) {
        QString fstr = str.mid(9, str.length() - 10);
        QList<QString> substrs = splitFormula(fstr);

        ShapeObj * mshape = new RectangleShape();
        mshape->setLabel(marker+"VALUE");
        mshape->setSize(QSizeF(80,30));
        mshape->setFillColour(QColor("gray"));
        canvas->addItem(mshape);

        ShapeObj * shape1 = drawEquivalentClass(substrs.at(0),canvas);
        ShapeObj * shape2 = drawEquivalentClass(substrs.at(1),canvas);
        Connector *conn;
        conn = new Connector();
        conn->initWithConnection(mshape,shape1);
        canvas->addItem(conn);
        conn->setDirected(true);
        conn->setColour(QColor("blue"));

        conn = new Connector();
        conn->initWithConnection(mshape,shape2);
        canvas->addItem(conn);
        conn->setLabel("Has");
        conn->setDirected(true);
        conn->setColour(QColor("blue"));

        rshape = mshape;
    }

    else if (str.startsWith("ExactCardinality")) {
        QString fstr = str.mid(17, str.length() - 18);
        QList<QString> substrs = splitFormula(fstr);

        ShapeObj * mshape = new RectangleShape();
        mshape->setLabel(marker+"EXACTLY");
        mshape->setSize(QSizeF(80,30));
        mshape->setFillColour(QColor("gray"));
        canvas->addItem(mshape);

        ShapeObj * shape1 = drawEquivalentClass(substrs.at(1),canvas);
        ShapeObj * shape2 = drawEquivalentClass(substrs.at(2),canvas);

        Connector *conn;
        conn = new Connector();
        conn->initWithConnection(mshape,shape1);
        canvas->addItem(conn);
        conn->setDirected(true);
        conn->setColour(QColor("blue"));

        conn = new Connector();
        conn->initWithConnection(mshape,shape2);
        canvas->addItem(conn);
        conn->setDirected(true);
        conn->setLabel("Exactly " + substrs.at(0));
        conn->setColour(QColor("blue"));

        rshape = mshape;
    }

    else if (str.startsWith("MinCardinality")) {
        QString fstr = str.mid(15, str.length() - 16);
        QList<QString> substrs = splitFormula(fstr);

        ShapeObj * mshape = new RectangleShape();
        mshape->setLabel(marker+"MIN");
        mshape->setSize(QSizeF(80,30));
        mshape->setFillColour(QColor("gray"));
        canvas->addItem(mshape);

        ShapeObj * shape1 = drawEquivalentClass(substrs.at(1),canvas);
        ShapeObj * shape2 = drawEquivalentClass(substrs.at(2),canvas);

        Connector *conn;
        conn = new Connector();
        conn->initWithConnection(mshape,shape1);
        canvas->addItem(conn);
        conn->setDirected(true);
        conn->setColour(QColor("blue"));

        conn = new Connector();
        conn->initWithConnection(mshape,shape2);
        canvas->addItem(conn);
        conn->setDirected(true);
        conn->setLabel("Min " + substrs.at(0));
        conn->setColour(QColor("blue"));

        rshape = mshape;
    }

    else if (str.startsWith("MaxCardinality")) {
        QString fstr = str.mid(15, str.length() - 16);
        QList<QString> substrs = splitFormula(fstr);

        ShapeObj * mshape = new RectangleShape();
        mshape->setLabel(marker+"MAX");
        mshape->setSize(QSizeF(80,30));
        mshape->setFillColour(QColor("gray"));
        canvas->addItem(mshape);

        ShapeObj * shape1 = drawEquivalentClass(substrs.at(1),canvas);
        ShapeObj * shape2 = drawEquivalentClass(substrs.at(2),canvas);

        Connector *conn;
        conn = new Connector();
        conn->initWithConnection(mshape,shape1);
        canvas->addItem(conn);
        conn->setDirected(true);
        conn->setColour(QColor("blue"));

        conn = new Connector();
        conn->initWithConnection(mshape,shape2);
        canvas->addItem(conn);
        conn->setDirected(true);
        conn->setLabel("Max " + substrs.at(0));
        conn->setColour(QColor("blue"));

        rshape = mshape;
    }

    //others
    else {
        QString sname = str;
        ShapeObj * entityshape = new RectangleShape();
        entityshape->setLabel(sname);
        entityshape->setSize(QSizeF(150,30));
        entityshape->setFillColour(QColor("orange"));
        canvas->addItem(entityshape);
        rshape = entityshape;
    }

    if(rshape!=NULL)rshape->setToolTip(getFormula(qstr));
    return rshape;
}

void OwlOntology::drawLogicalView(Canvas *canvas){
    for(int i=0;i<classes.size();i++){
        if(classes[i]->equivalentclass.trimmed()!=""){
            ShapeObj * tp;

//            tp = new RectangleShape();
//            tp->setLabel(classes[i]->shortname);
//            tp->setFillColour(QColor("pink"));
//            tp->setSize(QSizeF(150,30));

             tp= classes[i]->shape;

            tp->setToolTip(getFormula(classes[i]->equivalentclass));
            canvas->addItem(tp);


            ShapeObj * lgcview = drawEquivalentClass(classes[i]->equivalentclass,canvas);
            Connector * conn = new Connector();
            conn->initWithConnection(tp,lgcview);
            conn->setColour(QColor("black"));
            conn->setDirected(true);
            canvas->addItem(conn);

        }
    }
}

/** SLOTS to handle the shape signals **/
void OwlOntology::ontoclass_clicked(OntologyClassShape *classshape)
{
    if(this->currentfocusedclassidx!=-1)this->ontoclass_doubleclicked(classes[currentfocusedclassidx]->shape);
    int idx = this->getIndexOfClasses(classshape->idString());
    if(idx!=-1){
        classes[idx]->showIndividuals(this->maincanvas);
        classes[idx]->setFocused(true,this->maincanvas);
        this->maincanvas->fully_restart_graph_layout();
        this->currentfocusedclassidx = idx;
    }
}

void OwlOntology::ontoclass_doubleclicked(OntologyClassShape *classshape)
{
    int idx = this->getIndexOfClasses(classshape->idString());
    if(idx!=-1){
        classes[idx]->hideIndividuals(this->maincanvas);
        classes[idx]->setFocused(false,this->maincanvas);
        this->maincanvas->fully_restart_graph_layout();
    }
    this->currentfocusedclassidx = -1;
}

void OwlOntology::ontoclass_rightclicked(OntologyClassShape *classshape)
{
    int idx = this->getIndexOfClasses(classshape->idString());
    if(idx!=-1&&classes[idx]->equivalentclass.trimmed()!="")
    {
        wid->clearall();
        //root class
        OntologyClassShape * classshape = new OntologyClassShape();
        classshape->setIdString(">"+classes[idx]->shortname);
        classshape->setToolTip(classes[idx]->URI);
        classshape->setSize(QSizeF(150,20));
        classshape->setMyLabel(classes[idx]->shortname);
        classshape->setFillColour(this->CLASS_SHAPE_COLOR);
        for(int i=0;i<6;i++)
            classshape->setLabelByLevels(i+1,classes[idx]->shape->levelLabels[i]);
        wid->my_canvas->addItem(classshape);
        wid->setWindowTitle("[EQU]"+classes[idx]->shortname);
        //formula view
        ShapeObj * fshape = drawEquivalentClass(classes[idx]->equivalentclass,wid->my_canvas);
        //connection
        Connector * conn = new Connector();
        conn->initWithConnection(classshape,fshape);
        conn->setColour(QColor("brown"));
        conn->setDirected(true);
        conn->setLabel("[EQU]");
        Connector * conn1 = new Connector();
        conn1->initWithConnection(fshape,classshape);
        conn1->setColour(QColor("brown"));
        conn1->setDirected(true);

        wid->my_canvas->addItem(conn);
        wid->my_canvas->addItem(conn1);
        wid->my_canvas->setOptAutomaticGraphLayout(true);
        wid->my_canvas->setOptLayoutMode(1);
        wid->my_canvas->setOptPreventOverlaps(true);
        wid->my_canvas->fully_restart_graph_layout();
        //wid->setGeometry(100,0,600,600);

    }
}

void OwlOntology::ontoindividual_clicked(OntologyIndividualShape *individualshape)
{

}

void OwlOntology::ontoproperty_clicked(OntologyPropertyShape *propertyshape)
{

}
