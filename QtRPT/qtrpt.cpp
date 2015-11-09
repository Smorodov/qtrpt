/*
Name: QtRpt
Version: 1.5.5
Web-site: http://www.qtrpt.tk
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2015 Aleksey Osipov

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "qtrpt.h"
#include <QBuffer>
#include <QApplication>
#include <QAction>
#include <QTime>
#include <QFile>
#include <QPrintPreviewDialog>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QPrinterInfo>
#include <QTextCursor>
#include <QTextBlock>
#include <QPrintDialog>
#include <QUrl>
#include <QToolBar>
#include "chart.h"
#include "CommonClasses.h"
#include "RptSql.h"
#include "Barcode.h"

/*!
 \namespace QtRptName
 \inmodule qtrptnamespace
 \brief Contains miscellaneous identifiers used throughout the QtRPT library.
*/

/*!
 \inmodule qtrptnamespace
 \enum QtRptName::BandType

 This enum describes the type of band.

 \value ReportTitle
 \value PageHeader
 \value DataGroupHeader
 \value MasterHeader
 \value MasterData
 \value MasterFooter
 \value DataGroupFooter
 \value ReportSummary
 \value PageFooter
*/

/*!
 \inmodule qtrptnamespace
 \enum QtRptName::FieldType

 This enum describes the type of Field.

 \value Text
 \value TextImage
 \value TextRich
 \value Image
 \value Diagram
 \value Reactangle
 \value RoundedReactangle
 \value Circle
 \value Triangle
 \value Rhombus
 \value Line
 \value Barcode
*/

/*!
 \inmodule qtrptnamespace
 \enum QtRptName::BorderStyle

 This enum describes the border's style.

 \value Dashed
 \value Dot_dash
 \value Dot_dot_dash
 \value Dotted
 \value Double
 \value Groove
 \value Inset
 \value Outset
 \value Ridge
 \value Solid
 \value BorderNone
*/

/*!
 \inmodule qtrptnamespace
 \enum QtRptName::Command

 This enum describes the command, used in QtRptDesigner.
 \value None
 \value Name
 \value Bold
 \value Italic
 \value Underline
 \value Strikeout
 \value Font
 \value FontSize
 \value FontColor
 \value FontName
 \value Frame
 \value FrameLeft
 \value FrameRight
 \value FrameTop
 \value FrameBottom
 \value FrameNo
 \value FrameAll
 \value FrameStyle
 \value FrameWidth
 \value AligmentH
 \value AligmentV
 \value Left
 \value Top
 \value Width
 \value Height
 \value Length
 \value BackgroundColor
 \value BorderColor
 \value Printing
 \value StartNewNumeration
 \value ShowInGroup
 \value StartNewPage
 \value AutoHeight
 \value ArrowStart
 \value ArrowEnd
 \value IgnoreRatioAspect
 \value BarcodeType
 \value BarcodeFrameType
 \value TextWrap
*/

QtRPT* createQtRPT() {
     QtRPT *z = new QtRPT();
     return z;
}

/*!
 \class QtRPT
 \inmodule qtrpt
 \brief The QtRPT class is the base class of the QtRPT.

  QtRPT is the easy-to-use print report engine written in C++ QtToolkit.
  It allows combining several reports in one XML file.

  For separately taken field, you can specify some condition depending on which this
  field will display in different font and background color, etc.
*/

/*!
 \fn QtRPT::QtRPT(QObject *parent)
    Constructs a QtRPT object with the given \a parent.
*/
QtRPT::QtRPT(QObject *parent) : QObject(parent) {
    xmlDoc = QDomDocument("Reports");
    m_backgroundImage = 0;
    m_orientation = 0;
    rptSql = 0;
    m_printMode = QtRPT::Printer;
    m_resolution = QPrinter::HighResolution;
    painter = 0;
    printer = 0;
    m_xlsx = 0;
}

/*!
  \fn QtRPT::setResolution(QPrinter::PrinterMode resolution)
  Sets \a resolution of the printer
*/
void QtRPT::setResolution(QPrinter::PrinterMode resolution) {
    m_resolution = resolution;
}

/*!
 \fn QtRPT::loadReport(QString fileName)
  Loads report from XML file with \a fileName.
 Returns \c true if loading is success
 */
bool QtRPT::loadReport(QString fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    else {
        listOfPair.clear();
        listIdxOfGroup.clear();
    }
    if (!xmlDoc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();
    makeReportObjectStructure();
    return true;
}

/*! \overload
 Loads report from QDomDocument \a xmlDoc.
 Returns \c true if loading is success
*/
bool QtRPT::loadReport(QDomDocument xmlDoc) {
    QtRPT::xmlDoc = xmlDoc;
    listOfPair.clear();
    listIdxOfGroup.clear();
    makeReportObjectStructure();
    return true;
}

/*!
 \fn QtRPT::setPainter(QPainter *painter)
  Set the \a painter that will be used for the report to draw.
  Returns \c true if assignment is success
 */
bool QtRPT::setPainter(QPainter *painter) {
    if (this->painter == 0) {
        this->painter = painter;
        return true;
    }
    return false;
}

/*!
 \fn QtRPT::setPrinter(QPrinter *printer)
  Set the \a printer that will be used for the report to printing.
  Returns \c true if assignment is success
 */
bool QtRPT::setPrinter(QPrinter *printer) {
    if (this->printer == 0){
        this->printer = printer;
        return true;
    };
    return false;
}

void QtRPT::makeReportObjectStructure() {
    clearObject();
    for (int i = 0; i < xmlDoc.documentElement().childNodes().count(); i++) {
        QDomElement docElem = xmlDoc.documentElement().childNodes().at(i).toElement();
        RptPageObject *pageObject = new RptPageObject();
        pageObject->setProperty(this,docElem);
        pageList.append(pageObject);
    }
}

/*!
 \fn QtRPT::~QtRPT()
  Destroys the object, deleting all its child objects.
 */
QtRPT::~QtRPT() {
    clearObject();
}

/*!
 \fn QtRPT::clearObject()
 Destroy all objects and clear the report.
 */
void QtRPT::clearObject() {
    for (int i=0; i<pageList.size(); i++)
        delete pageList.at(i);
    pageList.clear();
}

QDomNode QtRPT::getBand(BandType type, QDomElement docElem) {
    QString s_type;
    if (type == ReportTitle)     s_type = "ReportTitle";
    if (type == PageHeader)      s_type = "PageHeader";
    if (type == MasterData)      s_type = "MasterData";
    if (type == PageFooter)      s_type = "PageFooter";
    if (type == ReportSummary)   s_type = "ReportSummary";
    if (type == MasterFooter)    s_type = "MasterFooter";
    if (type == MasterHeader)    s_type = "MasterHeader";
    if (type == DataGroupHeader) s_type = "DataGroupHeader";
    if (type == DataGroupFooter) s_type = "DataGroupFooter";
    //QDomElement docElem = xmlDoc.documentElement();  //get root element
    QDomNode n = docElem.firstChild();//.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if ((!e.isNull()) && (e.tagName() == "ReportBand")) {
            if (e.attribute("type") == s_type) {
                return n;
            }
        }
        n = n.nextSibling();
    }
    return n;
}

void QtRPT::setFont(RptFieldObject *fieldObject) {
    if (painter->isActive()) {
        painter->setFont(fieldObject->font);
        painter->setPen(Qt::black);
    }
}

Qt::Alignment QtRPT::getAligment(QDomElement e) {
    Qt::Alignment al;
    Qt::Alignment alH, alV;
    if (e.attribute("aligmentH") == "hRight")   alH = Qt::AlignRight;
    if (e.attribute("aligmentH") == "hLeft")    alH = Qt::AlignLeft;
    if (e.attribute("aligmentH") == "hCenter")  alH = Qt::AlignHCenter;
    if (e.attribute("aligmentH") == "hJustify") alH = Qt::AlignJustify;
    if (e.attribute("aligmentV") == "vTop")     alV = Qt::AlignTop;
    if (e.attribute("aligmentV") == "vBottom")  alV = Qt::AlignBottom;
    if (e.attribute("aligmentV") == "vCenter")  alV = Qt::AlignVCenter;
    return al = alH | alV;
}

QPen QtRPT::getPen(RptFieldObject *fieldObject) {
    QPen pen;
    if (painter->isActive())
        pen = painter->pen();
    //Set border width
    pen.setWidth(fieldObject->borderWidth*5);
    //Set border style
    QString borderStyle = fieldObject->borderStyle;
    pen.setStyle(getPenStyle(borderStyle));
    return pen;
}

/*!
 \fn Qt::PenStyle QtRPT::getPenStyle(QString value)
 Convert and return Pen style of field for given \a value
 */
Qt::PenStyle QtRPT::getPenStyle(QString value) {
    Qt::PenStyle style;
    if (value == "dashed") style = Qt::DashLine;
    else if (value == "dotted") style = Qt::DotLine;
    else if (value == "dot-dash") style = Qt::DashDotLine;
    else if (value == "dot-dot-dash") style = Qt::DashDotDotLine;
    else style = Qt::SolidLine;
    return style;
}

/*!
 \fn QtRPT::getFieldType(QDomElement e)
 Return field's type of given QDomElement \a e which represents a field
 \sa getFieldTypeName()
 */
FieldType QtRPT::getFieldType(QDomElement e) {
    if (e.attribute("type","label") == "barcode") {
        return Barcode;
    } else if (e.attribute("type","label") == "reactangle") {
        return Reactangle;
    } else if (e.attribute("type","label") == "roundedReactangle") {
        return RoundedReactangle;
    } else if (e.attribute("type","label") == "circle") {
        return Circle;
    } else if (e.attribute("type","label") == "triangle") {
        return Triangle;
    } else if (e.attribute("type","label") == "rhombus") {
        return Rhombus;
    } else if (e.attribute("type","label") == "textRich") {
        return TextRich;
    } else if (e.attribute("type","label") == "label") {
        return Text;
    } else if (e.attribute("type","label") == "labelImage") {
        return TextImage;
    } else if (e.attribute("type","label") == "image" || e.attribute("picture","text") != "text") {
        return Image;
    } else if (e.attribute("type","label") == "diagram") {
        return Diagram;
    } else if (e.attribute("type","label") == "line") {
        return Line;
    } else if (e.attribute("type","label") == "DatabaseImage") {
        return DatabaseImage;
    } else if (e.attribute("type","label") == "crossTab") {
        return CrossTab;
    } else return Text;
}

/*!
 \fn QString QtRPT::getFieldTypeName(FieldType type)
 Return the field's type name for given \a type
 \sa getFieldType()
 */
QString QtRPT::getFieldTypeName(FieldType type) {
    switch (type) {
        case Reactangle: return "reactangle";
        case RoundedReactangle: return "roundedReactangle";
        case Circle: return "circle";
        case Triangle: return "triangle";
        case Rhombus: return "rhombus";
        case TextRich: return "textRich";
        case Text: return "label";
        case TextImage: return "labelImage";
        case Image: return "image";
        case Diagram: return "diagram";
        case Line: return "line";
        case Barcode: return "barcode";
        case DatabaseImage: return "DatabaseImage";
        case CrossTab: return "crossTab";
        default: return "label";
    }
}

/*!
 \fn QList<FieldType> QtRPT::getDrawingFields()
 Return the QList contains the type of fields which acts as a drawing fields.
 The following fields are a drawing:
 \list
    \li Circle
    \li Triangle
    \li Rhombus
    \li RoundedReactangle
    \li Reactangle
 \endlist
 */
QList<FieldType> QtRPT::getDrawingFields() {
    QList<FieldType> set;
    set<<Circle<<Triangle<<Rhombus<<RoundedReactangle<<Reactangle;
    return set;
}

void QtRPT::drawFields(RptFieldObject *fieldObject, int bandTop, bool draw) {
    fieldObject->m_recNo = m_recNo;
    fieldObject->m_reportPage = m_pageReport;
    if (draw)
        fieldObject->updateHighlightingParam();

    emit setField(*fieldObject);

    int left_ = fieldObject->rect.x()*koefRes_w;
    int width_ = (fieldObject->rect.width()-1)*koefRes_w;
    int height_ = fieldObject->rect.height()*koefRes_h;
    int top_ = (bandTop+fieldObject->rect.y())*koefRes_h;

    fieldObject->setTop(top_/koefRes_h);

    if (fieldObject->autoHeight == 1) {
        if (fieldObject->parentBand != 0)
            height_ = fieldObject->parentBand->realHeight*koefRes_h;
    }

    FieldType fieldType = fieldObject->fieldType;
    QPen pen = getPen(fieldObject);

	if (draw) {
        if (!getDrawingFields().contains(fieldType) && fieldType != Barcode) {
            //Fill background
            if ( fieldObject->backgroundColor  != QColor(255,255,255,0)) {
                if (painter->isActive())
                    painter->fillRect(left_+1,top_+1,width_-2,height_-2,fieldObject->backgroundColor);
            }
            //Draw frame
            if (fieldObject->borderTop != QColor(255,255,255,0) ) {
                pen.setColor(fieldObject->borderTop);
                if (painter->isActive()) {
                    painter->setPen(pen);
                    painter->drawLine(left_, top_, left_ + width_, top_);
                }
            }
            if (fieldObject->borderBottom != QColor(255,255,255,0) ) {
                pen.setColor(fieldObject->borderBottom);
                if (painter->isActive()) {
                    painter->setPen(pen);
                    painter->drawLine(left_, top_ + height_, left_ + width_, top_ + height_);
                }
            }
            if (fieldObject->borderLeft != QColor(255,255,255,0) ) {
                pen.setColor(fieldObject->borderLeft);
                if (painter->isActive()) {
                    painter->setPen(pen);
                    painter->drawLine(left_, top_, left_, top_ + height_);
                }
            }
            if (fieldObject->borderRight != QColor(255,255,255,0) ) {
                pen.setColor(fieldObject->borderRight);
                if (painter->isActive()) {
                    painter->setPen(pen);
                    painter->drawLine(left_ + width_, top_, left_ + width_, top_ + height_);
                }
            }
        }
        if (fieldType == Rhombus) {
            qreal pointX1 = width_/2+left_;
            qreal pointY1 = height_-1+top_;

            qreal pointX2 = width_ + left_;
            qreal pointY2 = height_/2+top_;

            qreal pointX3 = width_/2+left_;
            qreal pointY3 = top_+1;  //

            qreal pointX4 = left_+1;  //
            qreal pointY4 = height_/2+top_;

            QPainterPath path;
            path.moveTo (pointX1, pointY1);
            path.lineTo (pointX2, pointY2);
            path.lineTo (pointX3, pointY3);
            path.lineTo (pointX4, pointY4);
            path.lineTo (pointX1, pointY1);

            QBrush brush(fieldObject->backgroundColor);
            pen.setColor(fieldObject->borderColor);
            if (painter->isActive()) {
                painter->drawPath(path);
                painter->fillPath (path, brush);
            }
        }
        if (fieldType == Triangle) {
            qreal pointX1 = left_;
            qreal pointY1 = height_-1+top_;

            qreal pointX2 = width_+left_;
            qreal pointY2 = height_-1+top_;

            qreal pointX3 = width_/2+left_;
            qreal pointY3 = top_;

            QPainterPath path;
            path.moveTo (pointX1, pointY1);
            path.lineTo (pointX2, pointY2);
            path.lineTo (pointX3, pointY3);
            path.lineTo (pointX1, pointY1);

            QBrush brush(fieldObject->backgroundColor);
            pen.setColor(fieldObject->borderColor);
            if (painter->isActive()) {
                painter->drawPath(path);
                painter->fillPath (path, brush);
            }
        }
        if (fieldType == RoundedReactangle) {
            QRectF rect(left_,top_,width_-2,height_-2);
            QBrush brush(fieldObject->backgroundColor);
            pen.setColor(fieldObject->borderColor);
            if (painter->isActive()) {
                painter->setBrush(brush);
                painter->setPen(pen);
                painter->drawRoundedRect(rect, 40, 40);
            }
        }
        if (fieldType == Reactangle) {
            QRectF rect(left_,top_,width_-2,height_-2);
            QBrush brush(fieldObject->backgroundColor);
            pen.setColor(fieldObject->borderColor);
            if (painter->isActive()) {
                painter->setBrush(brush);
                painter->setPen(pen);
                painter->drawRect(rect);
            }
        }
        if (fieldType == Circle) {
            QBrush brush(fieldObject->backgroundColor);
            pen.setColor(fieldObject->borderColor);
            if (painter->isActive()) {
                painter->setBrush(brush);
                painter->setPen(pen);
                painter->drawEllipse(left_, top_, width_, height_);
            }
        }
        if (fieldType == TextImage || fieldType == DatabaseImage) { //Proccess field as ImageField
            QImage image = (fieldType == TextImage) ? sectionValueImage(fieldObject->value) : sectionFieldImage(fieldObject->value);

            if (!image.isNull()) {
                QImage scaledImage = image.scaled(QSize(width_,height_),Qt::KeepAspectRatio);
                QPoint point(left_, top_);
                Qt::Alignment alignment = fieldObject->aligment;
                // Horizontal Center
                if (alignment.testFlag(Qt::AlignHCenter)) {
                    int offsetX = (width_ - scaledImage.width()) / 2;
                    point.setX(left_ + offsetX);
                }
                // Vertical Center
                if (alignment.testFlag(Qt::AlignVCenter)) {
                    int offsetY = (height_ - scaledImage.height()) / 2;
                    point.setY(top_ + offsetY);
                }
                if (painter->isActive())
                    painter->drawImage(point,scaledImage);

                if (m_printMode == QtRPT::Html) {
                    QByteArray ba;
                    QBuffer buffer(&ba);
                    buffer.open(QIODevice::WriteOnly);
                    scaledImage.save(&buffer, "PNG"); // writes image into ba in PNG format

                    fieldObject->picture = ba;
                    m_HTML.append(fieldObject->getHTMLStyle());
                }
            }
        }
        if (fieldType == Image) {  //Proccess as static ImageField
            QImage image = QImage::fromData(fieldObject->picture, fieldObject->imgFormat.toLatin1().data());
            if (fieldObject->ignoreAspectRatio == 1) {
                if (painter->isActive())
                    painter->drawImage(QRectF(left_,top_,width_,height_),image);
            } else {
                if (painter->isActive())
                    painter->drawImage(QRectF(left_,top_,image.width()*koefRes_w,image.height()*koefRes_h),image);
            }

            if (m_printMode == QtRPT::Html) {
                m_HTML.append(fieldObject->getHTMLStyle());
            }
        }
        if (fieldType == Diagram) {
            Chart *chart = new Chart();
            chart->setObjectName(fieldObject->name);
            chart->setParams(fieldObject->showGrid,
                             fieldObject->showLegend,
                             fieldObject->showCaption,
                             fieldObject->showGraphCaption,
                             fieldObject->showPercent,
                             fieldObject->caption,
                             fieldObject->autoFillData
                             );
            chart->clearData();
            chart->setKoef(koefRes_w, koefRes_h, left_, top_);
            chart->resize(width_,height_);
            if (fieldObject->autoFillData == 0) {
                emit setValueDiagram(*chart);
            } else {
                fieldObject->updateDiagramValue();
                for (int h=0; h<fieldObject->graphList.size(); h++) {
                    chart->setData(fieldObject->graphList.at(h));
                }
            }
            if (painter->isActive())
                chart->paintChart(painter);
        }
        if (fieldType == Barcode) {
            #ifndef NO_BARCODE
                BarCode br;
                br.setObjectName(fieldObject->name);
                QString txt = sectionField(fieldObject->parentBand, fieldObject->value, false, false, "");
                br.setValue(txt);
                BarCode::BarcodeTypes m_barcodeType = (BarCode::BarcodeTypes)fieldObject->barcodeType;
                br.setBarcodeType(m_barcodeType);
                BarCode::FrameTypes m_barcodeFrameType = (BarCode::FrameTypes)fieldObject->barcodeFrameType;
                br.setFrameType(m_barcodeFrameType);
                br.setHeight(fieldObject->barcodeHeight);
                br.drawBarcode(painter,left_,top_,width_,height_);
            #endif
        }
    }
    if (fieldType == TextRich) {
        QString txt = fieldObject->value;

        QTextDocument document;
        document.setHtml(txt);
        document.setDefaultFont(painter->font());

        QTextBlock block = document.firstBlock();
        while (block.isValid()) {
            for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
                QTextFragment currentFragment = it.fragment();
                if (!currentFragment.isValid())
                    continue;

                if ((currentFragment.text().contains("[") && currentFragment.text().contains("]")) ||
                    (currentFragment.text().contains("<") && currentFragment.text().contains(">")))
                {
                    QString tmpTxt = sectionField(fieldObject->parentBand, currentFragment.text(), false, false, "");
                    QTextCursor c = document.find(currentFragment.text(),0,QTextDocument::FindWholeWords);
                    if (tmpTxt.isEmpty() || tmpTxt.isNull())
                        tmpTxt = " ";
                    if (tmpTxt.toLower().contains("<body") && tmpTxt.toLower().contains("</body>")) {
                        int start = tmpTxt.toLower().indexOf("<body");
                        int end = tmpTxt.toLower().indexOf("</body>")+1;
                        c.insertHtml(tmpTxt.mid(start,end));
                    } else
                        c.insertText(tmpTxt);
                }
            }
            block = block.next();
        }

        QRectF rect = QRectF(left_+10, top_, width_-15, height_);
        document.setTextWidth( rect.width() );
        if (painter->isActive()) {
            painter->save();
            painter->translate( rect.topLeft() );
        }
        if (draw) {
            document.drawContents( painter, rect.translated( -rect.topLeft() ) );
        }
        if (painter->isActive())
            painter->restore();
    }
    if (fieldType == Text) { //NOT Proccess if field set as ImageField
        setFont(fieldObject);
        QString txt = sectionField(fieldObject->parentBand, fieldObject->value, false, false, fieldObject->formatString);
        pen.setColor(fieldObject->fontColor);
        if (painter->isActive())
            painter->setPen(pen);
        int flags = fieldObject->aligment | Qt::TextDontClip; //getAligment(e);
        if (fieldObject->textWrap == 1)
            flags = flags | Qt::TextWordWrap;
        if (draw) {
            if (painter->isActive())
                painter->drawText(left_+10,top_,width_-15,height_, flags, txt);

            if (m_printMode == QtRPT::Html) {
                m_HTML.append("<div "+fieldObject->getHTMLStyle()+">"+txt+"</div>\n");
            }
            if (m_printMode == QtRPT::Xlsx) {
                m_xlsx->write("A3", txt);
            }
        } else {
            QRect boundRect = painter->boundingRect(left_+10,top_,width_-15,height_, flags, txt);
            if (boundRect.height() > height_ && fieldObject->autoHeight == 1) {
                /*To correct adjust and display a height of the band we use a param 'realHeight'.
                 Currently this param used only to correct a MasterBand. If will be needed, possible
                 correct also another bands.
                */
                fieldObject->parentBand->realHeight = qRound(boundRect.height()/koefRes_h);
            }
        }
    }
    if (fieldType == CrossTab) {
        fieldObject->crossTab->makeFielMatrix();
        const int bandTop_ = bandTop;
        foreach(RptFieldObject *field, fieldObject->crossTab->fieldList) {
            drawFields(field,bandTop_,true);
        }
    }
}

void QtRPT::drawLines(RptFieldObject *fieldObject, int bandTop) {
    int startX = fieldObject->lineStartX*koefRes_w;
    int endX = fieldObject->lineEndX*koefRes_w;

    int startY = (bandTop+fieldObject->lineStartY)*koefRes_h;
    int endY = (bandTop+fieldObject->lineEndY)*koefRes_h;

    FieldType fieldType = fieldObject->fieldType;
    QPen pen = getPen(fieldObject);
    pen.setColor(fieldObject->borderColor);
    if (painter->isActive())
        painter->setPen(pen);
    if (fieldType == Line) {
        if (painter->isActive())
            painter->drawLine(startX, startY, endX, endY);
    }

    QLineF line(startX,startY,endX,endY);

    //Draw arrows
    static const double Pi = 3.14159265358979323846264338327950288419717;
    static double TwoPi = 2.0 * Pi;
    double angle = ::acos(line.dx() / line.length());
    if (line.dy() >= 0)
         angle = TwoPi - angle;

     QPointF sourcePoint = line.p1();
     QPointF destPoint = line.p2();
     int arrowSize= 10*koefRes_w;

     if (painter->isActive())
        painter->setBrush(fieldObject->borderColor);

     if (fieldObject->arrowStart == 1) {
         QPointF sourceArrowP1 = sourcePoint + QPointF(sin(angle + Pi / 3) * arrowSize,
                                                       cos(angle + Pi / 3) * arrowSize);
         QPointF sourceArrowP2 = sourcePoint + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                                       cos(angle + Pi - Pi / 3) * arrowSize);
         if (painter->isActive())
            painter->drawPolygon(QPolygonF() << line.p1() << sourceArrowP1 << sourceArrowP2);
     }
     if (fieldObject->arrowEnd == 1) {
         QPointF destArrowP1 = destPoint + QPointF(sin(angle - Pi / 3) * arrowSize,
                                                   cos(angle - Pi / 3) * arrowSize);
         QPointF destArrowP2 = destPoint + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
                                                   cos(angle - Pi + Pi / 3) * arrowSize);
         if (painter->isActive())
            painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
     }
}

void QtRPT::drawBandRow(RptBandObject *band, int bandTop, bool allowDraw) {
    band->realHeight = band->height; //set a 'realHeight' to default value
    /*First pass used to determine a max height of the band*/
    for (int i=0; i<band->fieldList.size(); i++) {
        if (band->fieldList.at(i)->fieldType != Line && isFieldVisible(band->fieldList.at(i))) {
            drawFields(band->fieldList.at(i),bandTop,false);
        }
    }

    /*Second pass used for drawing*/
    if (allowDraw) {
        for (int i=0; i<band->fieldList.size(); i++) {
            if (isFieldVisible(band->fieldList.at(i))) {
                if (band->fieldList.at(i)->fieldType != Line) {
                    drawFields(band->fieldList.at(i),bandTop,true);
                } else {
                    drawLines(band->fieldList.at(i),bandTop);
                }
            }
        }
    }
}

QVariant QtRPT::processHighligthing(RptFieldObject *field, HiType type) {
    if (field->highlighting.isEmpty() || field->highlighting.isNull()) {
        switch (type) {
            case FntBold: {
                return field->font.bold();
                break;
            }
            case FntItalic: {
                return field->font.italic();
                break;
            }
            case FntUnderline: {
                return field->font.underline();
                break;
            }
            case FntStrikeout: {
                return field->font.strikeOut();
                break;
            }
            case FntColor: {
                return colorToString(field->m_fontColor);
                break;
            }
            case BgColor: {
                return colorToString(field->m_backgroundColor);
                break;
            }
        }
    } else {
        if (type == BgColor && !field->highlighting.contains("backgroundColor") ) {
            return colorToString(field->m_backgroundColor);
        }

        QStringList list = field->highlighting.split(";");
        const QString cond = list.at(0);
        for (int i = 1; i < list.size(); i++) {
            if (list.at(i).isEmpty()) continue;
            QString exp = list.at(i);
            if (list.at(i).contains("bold") && type == FntBold) {
                exp.remove("bold=");
                QString formulaStr = exp.insert(0,cond);
                formulaStr = sectionField(field->parentBand,formulaStr,true);
                QScriptEngine myEngine;
                return myEngine.evaluate(formulaStr).toInteger();
            }
            if (list.at(i).contains("italic") && type == FntItalic) {
                exp.remove("italic=");
                QString formulaStr = exp.insert(0,cond);
                formulaStr = sectionField(field->parentBand,formulaStr,true);
                QScriptEngine myEngine;
                return myEngine.evaluate(formulaStr).toInteger();
            }
            if (list.at(i).contains("underline") && type == FntUnderline) {
                exp.remove("underline=");
                QString formulaStr = exp.insert(0,cond);
                formulaStr = sectionField(field->parentBand,formulaStr,true);
                QScriptEngine myEngine;
                return myEngine.evaluate(formulaStr).toInteger();
            }
            if (list.at(i).contains("strikeout") && type == FntStrikeout) {
                exp.remove("strikeout=");
                QString formulaStr = exp.insert(0,cond);
                formulaStr = sectionField(field->parentBand,formulaStr,true);
                QScriptEngine myEngine;
                return myEngine.evaluate(formulaStr).toInteger();
            }
            if (list.at(i).contains("fontColor") && type == FntColor) {
                exp.remove("fontColor=");
                QString formulaStr = exp.insert(1,"'");
                formulaStr = exp.insert(0,cond);
                formulaStr = sectionField(field->parentBand,formulaStr,true)+"':'"+colorToString(field->m_fontColor)+"'";
                QScriptEngine myEngine;
                return myEngine.evaluate(formulaStr).toString();
            }
            if (list.at(i).contains("backgroundColor") && type == BgColor) {
                exp.remove("backgroundColor=");
                QString formulaStr = exp.insert(1,"'");
                formulaStr = exp.insert(0,cond);
                //qDebug()<<field->name;
                //qDebug()<<colorToString(field->backgroundColor);
                formulaStr = sectionField(field->parentBand,formulaStr,true)+"':'"+colorToString(field->m_backgroundColor)+"'";
                //formulaStr = sectionField(field->parentBand,formulaStr,true)+"':'rgba(255,0,255,255)'";
                QScriptEngine myEngine;
                //qDebug()<<formulaStr;
                //qDebug()<<myEngine.evaluate(formulaStr).toString();
                //qDebug()<<"---";
                return myEngine.evaluate(formulaStr).toString();
            }
        }
    }
    return QVariant();
}

bool QtRPT::isFieldVisible(RptFieldObject *fieldObject) {
    bool visible;
    QString formulaStr = fieldObject->printing;
    if (fieldObject->printing.size() > 1) {
        formulaStr = sectionField(fieldObject->parentBand,fieldObject->printing,true);
        QScriptEngine myEngine;
        //myEngine.globalObject().setProperty("quant1","3");
        //qDebug()<<myEngine.evaluate("quant1;").toString();
        visible = myEngine.evaluate(formulaStr).toInteger();

        //QScriptValue fun = myEngine.evaluate("(function(a, b) { return a == b; })");
        //QScriptValue fun = myEngine.evaluate("if (1>2) true else false");
        //QScriptValueList args;
        /*args << "k" << "k";
        QScriptValue threeAgain = fun.call(QScriptValue(), args);
        qDebug()<<threeAgain.toString();*/

    } else {
        visible = formulaStr.toInt();
    }
    return visible;
}

QScriptValue funcAggregate(QScriptContext *context, QScriptEngine *engine) {
    Q_UNUSED(engine);

    QScriptValue self = context->thisObject();
    int funcMode = context->argument(0).toInteger();
    QString paramName = context->argument(1).toString();
    double total = 0;
    double min = 0;
    double max = 0;
    int count = 0;

    for (int i=0; i<listOfPair.size(); i++) {
        if (i == 0) //set initial value for Min
            min = listOfPair.at(i).paramValue.toDouble();
        if (listOfPair.at(i).paramName == paramName) {
            if (listIdxOfGroup.size() > 0 && self.property("showInGroup").toBool() == true) {
                for (int k = 0; k < listIdxOfGroup.size(); k++) {
                    if (listIdxOfGroup.at(k) == listOfPair.at(i).lnNo) {
                        total += listOfPair.at(i).paramValue.toDouble();
                        count += 1;
                        if (max < listOfPair.at(i).paramValue.toDouble())
                            max = listOfPair.at(i).paramValue.toDouble();
                        if (min > listOfPair.at(i).paramValue.toDouble())
                            min = listOfPair.at(i).paramValue.toDouble();
                    }
                }
            } else {
                if (!listOfPair.at(i).paramValue.toString().isEmpty()) {
                    total += listOfPair.at(i).paramValue.toDouble();
                    count += 1;
                    if (max < listOfPair.at(i).paramValue.toDouble())
                        max = listOfPair.at(i).paramValue.toDouble();
                    if (min > listOfPair.at(i).paramValue.toDouble())
                        min = listOfPair.at(i).paramValue.toDouble();
                }
            }
        }
    }

    switch (funcMode) {
    case 0:  //SUM
        return total;
        break;
    case 1:  //AVG
        if (count > 0)
            return total/count;
        else
            return 0;
        break;
    case 2:  //COUNT
        return count;
        break;
    case 3:  //MAX
        return max;
        break;
    case 4:  //MIN
        return min;
        break;
    default: return 0;
    }
    return 0;
}

QScriptValue funcText(QScriptContext *context, QScriptEngine *engine) {
    Q_UNUSED(engine);
    QString paramLanguage = context->argument(0).toString();
    double value = context->argument(1).toString().toDouble();
    QString paramValue;
    if (paramLanguage == "ENG")
        paramValue = double2Money(value,paramLanguage);
    if (paramLanguage == "UKR")
        paramValue = double2Money(value,paramLanguage);
    if (paramLanguage == "GER")
        paramValue = double2Money(value,paramLanguage);

    return paramValue;
}

QScriptValue funcFrac(QScriptContext *context, QScriptEngine *engine) {
    Q_UNUSED(engine);
    double value = context->argument(0).toString().toDouble();
    int b;
    b = qFloor(value);
    b = (value-b) * 100;
    return b;
}

QScriptValue funcFloor(QScriptContext *context, QScriptEngine *engine) {
    Q_UNUSED(engine);
    double value = context->argument(0).toString().toDouble();
    return qFloor(value);
}

QScriptValue funcCeil(QScriptContext *context, QScriptEngine *engine) {
    Q_UNUSED(engine);
    double value = context->argument(0).toString().toDouble();
    return qCeil(value);
}

QScriptValue funcRound(QScriptContext *context, QScriptEngine *engine) {
    Q_UNUSED(engine);
    double value = context->argument(0).toString().toDouble();
    return qRound(value);
}

QStringList QtRPT::splitValue(QString value) {
    QString tmpStr;
    QStringList res;
    for (int i = 0; i < value.size(); ++i) {
        if (value.at(i) != '[' && value.at(i) != ']')
            tmpStr += value.at(i);
        else {
            if (value.at(i) == ']') {
                tmpStr += value.at(i);
                res << tmpStr;
                tmpStr.clear();
            }
            if (value.at(i) == '[') {
                if (!tmpStr.isEmpty())
                    res << tmpStr;
                tmpStr.clear();
                tmpStr += value.at(i);
            }
        }
    }
    if (!tmpStr.isEmpty()) res << tmpStr;
    return res;
}

QString QtRPT::sectionField(RptBandObject *band, QString value, bool exp, bool firstPass, QString formatString) {
    QString tmpStr;
    QStringList res;
    bool aggregate = false;

    for (int i = 0; i < value.size(); ++i) {
        if (value.at(i) != '[' && value.at(i) != ']' &&
            value.at(i) != '<' && value.at(i) != '>' && !aggregate)
            tmpStr += value.at(i);
        else if ((value.at(i) == '[' || value.at(i) == ']') && aggregate)
             tmpStr += value.at(i);
        else if (value.at(i) != '<' && value.at(i) != '>' && aggregate)
             tmpStr += value.at(i);
        else {
            if (exp && (value.at(i) == '<' || value.at(i) == '>') )
                tmpStr += value.at(i);
            if (value.at(i) == ']' && !aggregate) {
                tmpStr += value.at(i);
                res << tmpStr;
                tmpStr.clear();
            }
            if (value.at(i) == '[' && !aggregate) {
                if (!tmpStr.isEmpty())
                    res << tmpStr;
                tmpStr.clear();
                tmpStr += value.at(i);
            }
            if (!exp && value.at(i) == '<') {
                aggregate = true;
                if (!tmpStr.isEmpty())
                    res << tmpStr;
                tmpStr.clear();
                tmpStr += value.at(i);
            }
            if (!exp && value.at(i) == '>') {
                aggregate = false;
                tmpStr += value.at(i);
                res << tmpStr;
                tmpStr.clear();
            }
        }
    }
    if (!tmpStr.isEmpty()) res << tmpStr;    

    tmpStr.clear();
    for (int i = 0; i < res.size(); ++i) {
        if (res.at(i).contains("[") && res.at(i).contains("]") && !res.at(i).contains("<") ) {
            QString tmp;
            if (rptSql != 0 ) {//if we have Sql DataSource
                if (res.at(i).contains(rptSql->objectName())) {
                    QString fieldName = res.at(i);
                    fieldName.replace("[","");
                    fieldName.replace("]","");
                    fieldName.replace(rptSql->objectName()+".","");
                    tmp = rptSql->getFieldValue(fieldName, m_recNo);
                }
            } else
                tmp = sectionValue(res.at(i));
            bool ok;
            if (exp) { //Process highlighting and visibility
                tmp.toDouble(&ok);
                if (!ok) tmp.toFloat(&ok);
                if (!ok) tmp.toInt(&ok);
                if (!ok) tmpStr += "'"+tmp+"'";  //Not a number
                else tmpStr += tmp;
            } else { //Process usuall field
                if (firstPass) { //Process during first pass to calculate aggregate values
                    AggregateValues av;
                    av.paramName = res.at(i);
                    av.paramValue = tmp;
                    av.lnNo = m_recNo;
                    av.pageReport = m_pageReport;
                    bool founded = false;
                    for (int j = 0; j < listOfPair.size(); ++j) {
                        if (listOfPair.at(j).pageReport == av.pageReport &&
                            listOfPair.at(j).lnNo == av.lnNo &&
                            listOfPair.at(j).paramName == av.paramName)
                            founded = true;
                    }
                    if (!founded)
                        listOfPair.append(av);
                }
                tmpStr += getFormattedValue(tmp, formatString); //tmp;
            }
        } else {
            if (res[i].contains("<Date>"))
                res[i] = res[i].replace("<Date>",processFunctions("Date").toString());
            if (res[i].contains("<Time>"))
                res[i] = res[i].replace("<Time>",QTime::currentTime().toString());
            if (res[i].contains("<Page>"))
                res[i] = res[i].replace("<Page>",QString::number(curPage));
            if (res[i].contains("<TotalPages>"))
                res[i] = res[i].replace("<TotalPages>",QString::number(totalPage));
            if (res[i].contains("<LineNo>"))
                res[i] = res[i].replace("<LineNo>",processFunctions("LineNo").toString());
            if (res[i].contains("<LineCount>"))
                res[i] = res[i].replace("<LineCount>",processFunctions("LineCount").toString());

            if (res[i].contains("<") && res[i].contains(">")) {
                QString formulaStr=res[i];
                QScriptEngine myEngine;

                QStringList tl = splitValue(formulaStr);
                for (int j = 1; j < tl.size(); ++j) {
                    if (tl.at(j).contains("[") &&
                        tl.at(j).contains("]") &&
                        !tl.at(j-1).toUpper().contains("SUM") &&
                        !tl.at(j-1).toUpper().contains("AVG") &&
                        !tl.at(j-1).toUpper().contains("COUNT") &&
                        !tl.at(j-1).toUpper().contains("MAX") &&
                        !tl.at(j-1).toUpper().contains("MIN") &&
                        !tl.at(j-1).toUpper().contains("NumberToWords") &&
                        !tl.at(j-1).toUpper().contains("Frac") &&
                        !tl.at(j-1).toUpper().contains("Floor") &&
                        !tl.at(j-1).toUpper().contains("Ceil")
                    ) {
                        if (rptSql != 0 ) {  //if we have Sql DataSource
                        /*???   After testing - remove commented
                         * if (tl.at(j).contains("[") && tl.at(j).contains("]") && !tl.at(j).contains("<") ) {
                         */
                            if (tl.at(j).contains(rptSql->objectName())) {
                                QString fieldName = tl.at(j);
                                fieldName.replace("[","");
                                fieldName.replace("]","");
                                fieldName.replace(rptSql->objectName()+".","");
                                QString tmp = rptSql->getFieldValue(fieldName, m_recNo);
                                qDebug()<<"value from DB: "<<tmp;
                                formulaStr.replace(tl.at(j), tmp);
                                qDebug()<<"formula with value: "<<formulaStr;
                            }
                        } else {
                            formulaStr.replace(tl.at(j), sectionValue(tl.at(j)));
                        }
                    }
                }

                myEngine.globalObject().setProperty("showInGroup", band->showInGroup);

                QScriptValue fun = myEngine.newFunction(funcAggregate);
                myEngine.globalObject().setProperty("Sum", fun);

                fun = myEngine.newFunction(funcText);
                myEngine.globalObject().setProperty("NumberToWords", fun);

                fun = myEngine.newFunction(funcFrac);
                myEngine.globalObject().setProperty("Frac", fun);

                fun = myEngine.newFunction(funcFloor);
                myEngine.globalObject().setProperty("Floor", fun);

                fun = myEngine.newFunction(funcCeil);
                myEngine.globalObject().setProperty("Ceil", fun);

                fun = myEngine.newFunction(funcRound);
                myEngine.globalObject().setProperty("Round", fun);

                formulaStr = formulaStr.replace("Sum(","Sum(0,", Qt::CaseInsensitive);
                formulaStr = formulaStr.replace("Avg(","Sum(1,", Qt::CaseInsensitive);
                formulaStr = formulaStr.replace("Count(","Sum(2,", Qt::CaseInsensitive);
                formulaStr = formulaStr.replace("Max(","Sum(3,", Qt::CaseInsensitive);
                formulaStr = formulaStr.replace("Min(","Sum(4,", Qt::CaseInsensitive);
                formulaStr = formulaStr.replace("[","'[");
                formulaStr = formulaStr.replace("]","]'");
                formulaStr = formulaStr.replace("<","");
                formulaStr = formulaStr.replace(">","");                

                QScriptValue result  = myEngine.evaluate(formulaStr);
                res[i] = getFormattedValue(result.toString(), formatString);
            }

            tmpStr += res.at(i);
        }
    }

    return tmpStr;
}

QString QtRPT::getFormattedValue(QString value, QString formatString) {
    if (!formatString.isEmpty()) {
        if (formatString.at(0) == 'N') {  //Numeric format
            int precision = formatString.mid(formatString.size()-1,1).toInt();
            QLocale locale;

            if ( formatString.mid(1,formatString.size()-2) == "# ###.##") {
                locale = QLocale(QLocale::C);
                value = locale.toString(value.toDouble(), 'f', precision).replace(","," ");
            }

            if ( formatString.mid(1,formatString.size()-2) == "#,###.##") {
                locale = QLocale(QLocale::C);
                value = locale.toString(value.toDouble(), 'f', precision);
            }

            if ( formatString.mid(1,formatString.size()-2) == "# ###,##") {
                locale = QLocale("fr_FR");
                value = locale.toString(value.toDouble(), 'f', precision);
            }
            if ( formatString.mid(1,formatString.size()-2) == "#.###,##") {
                locale = QLocale(QLocale::German);
                value = locale.toString(value.toDouble(), 'f', precision);
            }
        }
    }
    return value;
}

void QtRPT::fillListOfValue(RptBandObject *bandObject) {
    for (int i=0; i<bandObject->fieldList.size(); i++) {
        if (bandObject->fieldList.at(i)->fieldType == Text && isFieldVisible(bandObject->fieldList.at(i))) {
            QString txt = sectionField(bandObject, bandObject->fieldList.at(i)->value, false, true);
        }
    }
}

QVariant QtRPT::processFunctions(QString value) {
    if (value.contains("Date"))
        return QDate::currentDate().toString("dd.MM.yyyy");
    if (value.contains("Time"))
        return QTime::currentTime().toString();
    if (value.contains("Page"))
        return QString::number(curPage);
    if (value.contains("TotalPages"))
        return QString::number(totalPage);
    if (value.contains("LineNo")) {
        int recNo;
        if (!listOfGroup.isEmpty()) //group processing
            recNo = mg_recNo;
        else //usuall processing
            recNo = m_recNo+1;
        return QString::number(recNo);
    }
    if (value.contains("LineCount")) {
        int maxLnNo = 0;
        for (int i=0; i<listOfPair.size(); i++) {
            if (listOfPair.at(i).pageReport == m_pageReport && listOfPair.at(i).lnNo > maxLnNo)
                maxLnNo = listOfPair.at(i).lnNo;
        }
        return maxLnNo+1;
    }
    return QVariant();
}

QImage QtRPT::sectionFieldImage(QString value) {
    QString fieldName = value;
    fieldName.replace("[","");
    fieldName.replace("]","");
    fieldName.replace(rptSql->objectName()+".","");
    return rptSql->getFieldImage(fieldName, m_recNo);
}

QString QtRPT::sectionValue(QString paramName) {
    QVariant paramValue;
    paramName.replace("[","");
    paramName.replace("]","");
    //callbackFunc(recNo, paramName, paramValue);
    //if (paramValue.isNull())

    //if (!listOfGroup.isEmpty()) //group processing
    //    m_recNo = mg_recNo;

    emit setValue(m_recNo, paramName, paramValue, m_pageReport);        
    return paramValue.toString();
}

QImage QtRPT::sectionValueImage(QString paramName) {
    QImage paramValue;
    paramName.replace("[","");
    paramName.replace("]","");
    emit setValueImage(m_recNo, paramName, paramValue, m_pageReport);
    return paramValue;
}

/*void QtRPT::setCallbackFunc( void (*func)(int &recNo, QString &paramName, QVariant &paramValue) ) {
    callbackFunc=func;
}*/

/*!
 \fn QtRPT::printPDF(const QString &filePath, bool open)
 Print report direct to PDF file by \a filePath.
 Second param \a open indicates open or not after printing a pdf file.

 \sa printExec(), printHTML(), printXLSX()
 */
void QtRPT::printPDF(const QString &filePath, bool open) {
#ifndef QT_NO_PRINTER
    m_printMode = QtRPT::Pdf;
    if (printer == 0){
        printer = new QPrinter(m_resolution);
    };
    printer->setOutputFormat(QPrinter::PdfFormat);
    printer->setOutputFileName(filePath);
    if (painter == 0){
        painter = new QPainter();
    };
    printPreview(printer);
    if (open)
        QDesktopServices::openUrl(QUrl("file:"+filePath));
#endif
}

/*!
 \fn QtRPT::printHTML(const QString &filePath, bool open)
 Print report direct to HTML file by \a filePath.
 Second param \a open indicates open or not after printing a pdf file.

 \sa printExec(), printPDF(), printXLSX()
 */
void QtRPT::printHTML(const QString &filePath, bool open) {
#ifndef QT_NO_PRINTER
    m_printMode = QtRPT::Html;
    m_HTML.clear();
    m_HTML.append("<HTML><HEAD><meta charset=\"UTF-8\"></HEAD><BODY>");

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    if (printer == 0){
        printer = new QPrinter(m_resolution);
    };
    printer->setOutputFormat(QPrinter::PdfFormat);
    if (painter == 0){
        painter = new QPainter();
    };
    printPreview(printer);
    m_HTML.append("</BODY></HTML>");

    out << m_HTML;

    file.close();
    if (open)
        QDesktopServices::openUrl(QUrl("file:"+filePath));
#endif
}

/*!
 \fn QtRPT::printODT(const QString &filePath, bool open)
 Print report direct to ODT file by \a filePath.
 Second param \a open indicates open or not after printing a pdf file.

 \warning This function Under construction!

 \sa printExec(), printHTML(), printPDF()
 */
void QtRPT::printXLSX(const QString &filePath, bool open) {
#ifndef QT_NO_PRINTER  
    m_printMode = QtRPT::Xlsx;
    if (m_xlsx != 0) delete m_xlsx;
    m_xlsx = new QXlsx::Document(this);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    if (printer == 0){
        printer = new QPrinter(m_resolution);
    };
    printer->setOutputFormat(QPrinter::PdfFormat);
    if (painter == 0){
        painter = new QPainter();
    };
    printPreview(printer);


    m_xlsx->write("A1", "Hello Qt!");
    m_xlsx->saveAs(filePath);

    file.close();
    if (open)
        QDesktopServices::openUrl(QUrl("file:"+filePath));
#endif
}

/*!
 \fn QtRPT::printExec(bool maximum, bool direct, QString printerName)
 Open preview of the report in maximize or fitted mode. Or you can print report without preview.
 To open preview in maximize mode, set \a maximum to true.
 \code
 report->printExec(true,false);
 \endcode

 To direct printing without preview dialog, set \a direct to true.
 \code
 report->printExec(true,true);
 \endcode

 If  printer with the \a printerName is not valid,
 the default printer will be used.

 \sa printPDF(), printHTML(), printXLSX()
 */
void QtRPT::printExec(bool maximum, bool direct, QString printerName) {
#ifndef QT_NO_PRINTER
    m_printMode = QtRPT::Printer;

    if (printer == 0){
        printer = new QPrinter(m_resolution);
    };

    if (!printerName.isEmpty() && !printerName.isNull()) {
        printer->setPrinterName(printerName);
        if (!printer->isValid())
            printer->setPrinterName(QPrinterInfo::defaultPrinter().printerName());
    }

    if (painter == 0){
        painter = new QPainter();
    };

    if (!direct) {
        QPrintPreviewDialog preview(printer, qobject_cast<QWidget *>(this->parent()), Qt::Window);

        if (maximum) {
            QList<QPrintPreviewWidget *> list = preview.findChildren<QPrintPreviewWidget *>();
            if(!list.isEmpty()) // paranoiac safety check
                list.first()->setZoomMode(QPrintPreviewWidget::FitToWidth);
        }

        connect(&preview, SIGNAL(paintRequested(QPrinter*)), SLOT(printPreview(QPrinter*)));
        //preview.setWindowState(Qt::WindowMaximized); //Qt BUG https://bugreports.qt-project.org/browse/QTBUG-14517
        QRect geom = QApplication::desktop()->availableGeometry();
        geom.setTop(30);
        geom.setLeft(5);
        geom.setHeight(geom.height()-6);
        geom.setWidth(geom.width()-6);
        preview.setGeometry(geom);

        pr = preview.findChild<QPrintPreviewWidget *>();
        lst = preview.findChildren<QAction *>();

        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/pdf.png")), QIcon::Normal, QIcon::On);
        QAction *actExpToPdf = new QAction(icon,tr("Save as PDF"),this);
        actExpToPdf->setObjectName("actExpToPdf");
        connect(actExpToPdf, SIGNAL(triggered()), SLOT(exportTo()));

        icon.addPixmap(QPixmap(QString::fromUtf8(":/html.png")), QIcon::Normal, QIcon::On);
        QAction *actExpToHtml = new QAction(icon,tr("Save as HTML"),this);
        actExpToHtml->setObjectName("actExpToHtml");
        connect(actExpToHtml, SIGNAL(triggered()), SLOT(exportTo()));

        QIcon icon1;
        icon1.addPixmap(QPixmap(QString::fromUtf8(":/excel.png")), QIcon::Normal, QIcon::On);
        QAction *actExpToXlsx = new QAction(icon1,tr("Save as XLSX"),this);
        actExpToXlsx->setObjectName("actExpToXlsx");
        connect(actExpToXlsx, SIGNAL(triggered()), SLOT(exportTo()));

        QList<QToolBar *> l1 = preview.findChildren<QToolBar *>();
        l1.at(0)->addAction(actExpToPdf);
        l1.at(0)->addAction(actExpToHtml);
        l1.at(0)->addAction(actExpToXlsx);

        //preview.addActions(lst);
        pr->installEventFilter(this);
        //curPage = 1;
        preview.exec();
    } else
        printPreview(printer);  ///print without preview dialog
#endif
}

#include <QFileDialog>
void QtRPT::exportTo() {
    if (sender()->objectName() == "actExpToPdf") {
        QString fileName = QFileDialog::getSaveFileName(qobject_cast<QWidget *>(this->parent()), tr("Save File"), "", tr("PDF Files (*.pdf)"));
        if (fileName.isEmpty() || fileName.isNull() ) return;
        printPDF(fileName,false);
    }
    if (sender()->objectName() == "actExpToHtml") {
        QString fileName = QFileDialog::getSaveFileName(qobject_cast<QWidget *>(this->parent()), tr("Save File"), "", tr("HTML Files (*.html)"));
        if (fileName.isEmpty() || fileName.isNull() ) return;
        printHTML(fileName,true);
    }
    if (sender()->objectName() == "actExpToXlsx") {
        QString fileName = QFileDialog::getSaveFileName(qobject_cast<QWidget *>(this->parent()), tr("Save File"), "", tr("XLSX Files (*.xlsx)"));
        if (fileName.isEmpty() || fileName.isNull() ) return;
        printXLSX(fileName,true);
    }
}

bool lessThan(const AggregateValues a, const AggregateValues b) {
    if (a.paramValue.toString() == b.paramValue.toString())
        return a.lnNo < b.lnNo;
    else
        return a.paramValue.toString() < b.paramValue.toString();
}

/*!
 \fn QtRPT::printPreview(QPrinter *printer)
 Starts print or draw report at \a printer.
 Commonly, the purpose of this slot is using with QPrintPreviewWidget

 Example:
 \code
    QPrinter *printer = new QPrinter;
    printer->setOutputFormat(QPrinter::PdfFormat);
    printer->setOrientation(QPrinter::Portrait);
    printer->setPaperSize(QPrinter::A4);
    printer->setFullPage(true);

    QPrintPreviewWidget *preview = new QPrintPreviewWidget(printer, this);
    connect(preview, SIGNAL(paintRequested(QPrinter*)), report, SLOT(printPreview(QPrinter*)));
 \endcode
 */
void QtRPT::printPreview(QPrinter *printer) {
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    if (pageList.size() == 0) return;
    setPageSettings(printer,0);
    painter->begin(printer);

    fromPage = printer->fromPage();
    toPage =   printer->toPage();

    /*Make a two pass report
     *First pass calculate total pages
     *Second pass draw a report
     */
    curPage = 1;
    for (int i=0; i<pageList.size(); i++) {
        openDataSource(i);
        //listOfPair.clear();
        listIdxOfGroup.clear();
        m_recNo = 0;
        m_pageReport = i;
        //First pass
        processReport(printer,false,i);
        totalPage = curPage;
    }

    m_orientation = 0;
    painter->resetTransform();
    //setPageSettings(printer,0);
    curPage = 1;
    for (int i=0; i<pageList.size(); i++) {
        //listOfPair.clear();
        listIdxOfGroup.clear();
        m_recNo = 0;
        m_pageReport = i;
        //Second pass
        processReport(printer,true,i);
    }
    painter->end();
    //pr->setWindowState(pr->windowState() ^ Qt::WindowFullScreen);
#endif
}

void QtRPT::setPageSettings(QPrinter *printer, int pageReport) {
    ph = pageList.at(pageReport)->ph;
    pw = pageList.at(pageReport)->pw;
    ml = pageList.at(pageReport)->ml;
    mr = pageList.at(pageReport)->mr;
    mt = pageList.at(pageReport)->mt;
    mb = pageList.at(pageReport)->mb;
    int orientation = pageList.at(pageReport)->orientation;

    QSizeF paperSize;
    paperSize.setWidth(pw/4);
    paperSize.setHeight(ph/4);
    if (printer->printerState() != QPrinter::Active) {
        if (orientation == 1) {
            paperSize.setWidth(ph/4);
            paperSize.setHeight(pw/4);
        }
        printer->setPaperSize(paperSize,QPrinter::Millimeter);
    }
    printer->setPageMargins(ml/4+0.01, mt/4+0.01, mr/4+0.01, mb/4+0.01, QPrinter::Millimeter);

    QRect r = printer->pageRect();
    //pageList.at(pageReport)->border = true;
    //Draw page's border
    if (pageList.at(pageReport)->border) {
        if (painter->isActive()) {
            const QPen cpen= painter->pen();
            QPen pen(pageList.at(pageReport)->borderColor);
            pen.setWidth(pageList.at(pageReport)->borderWidth*5);
            pen.setStyle(getPenStyle(pageList.at(pageReport)->borderStyle));
            painter->setPen(pen);
            painter->drawRect(0-r.left()+92,0-r.top()+92,
                              printer->paperRect().width()-192,
                              printer->paperRect().height()-192);   //Rect around page
            painter->setPen(cpen);
        }
    }

    if (m_orientation != orientation) {
        m_orientation = orientation;
        //painter->resetTransform();
        if (orientation == 1) {
            if (painter->isActive()) {
                painter->rotate(90); // поворачиваем относительно (0,0)
                painter->translate(0, -painter->viewport().width()); // смещаемся на ширину вьюпорта влево
            }
        }
    }

    koefRes_h = static_cast<double>(r.height()) / (ph - mt - mb);
    koefRes_w = static_cast<double>(r.width())  / (pw - ml - mr);
    if (orientation == 1) {
        koefRes_h = static_cast<double>(r.width()) / (ph - mt - mb);
        koefRes_w = static_cast<double>(r.height())  / (pw - ml - mr);
    }
}

void QtRPT::processReport(QPrinter *printer, bool draw, int pageReport) {
    setPageSettings(printer, pageReport);
    int y = 0;

    drawBackground();
    if (pageReport > 0) {
        newPage(printer, y, draw, true);
    } else {
        processRTitle(y,draw);
        processPHeader(y,draw);
    }

    //processRTitle(y,draw);
    //processPHeader(y,draw);

        //processMHeader(y,draw);
        //processPFooter(draw);
    processGroupHeader(printer,y,draw,pageReport);
        //processMasterData(printer,y,draw,pageReport);
        //processMFooter(printer,y,draw);

    processRSummary(printer,y,draw);
}

/*!
 \fn bool QtRPT::eventFilter(QObject *obj, QEvent *e)
  Filters events if this object has been installed as an event
  filter for the \a obj.

  In your reimplementation of this function, if you want to filter
  the \a e out, i.e. stop it b

  Returns \c value from reimplemented function.

  Reimplemented from QObject::eventFilter()
 */
bool QtRPT::eventFilter(QObject *obj, QEvent *e) {
    if (obj == pr && e->type()==QEvent::Show)  {
        for (int i = 0; i < lst.size(); i++) {
            if (lst.at(i)->text().contains("Previous page", Qt::CaseInsensitive))
                lst.at(i)->trigger();
        }

        pr->setCurrentPage(0);
        return true;
    }
    return QObject::eventFilter(obj,e);
}

bool QtRPT::allowPrintPage(bool draw, int curPage_) {
    if (draw) {
        if (curPage_ < fromPage )
            draw = false;
        if ((toPage!=0) && (curPage_ > toPage ))
            draw = false;
        return draw;
    } else return false;
}

bool QtRPT::allowNewPage(bool draw, int curPage_) {
    if (draw) {
        if (curPage-fromPage < 0) return false;
        if (curPage_ < fromPage )
            draw = false;
        if ((toPage!=0) && (curPage_ > toPage ))
            draw = false;
        return draw;
    } else return false;
}

void QtRPT::newPage(QPrinter *printer, int &y, bool draw, bool newReportPage) {
    //curPage += 1;
    if (allowNewPage(draw, curPage+1)) {
        printer->newPage();
        drawBackground();
    }
    curPage += 1;
    if (draw)
      emit newPage(curPage);

    if (m_printMode != QtRPT::Html)
        y = 0;
    if (newReportPage)
        processRTitle(y,draw);
    processPHeader(y,draw);
    processPFooter(draw);
}

/*!
 \fn QtRPT::setBackgroundImage(QPixmap &image)
 Sets background image from \a image
 */
void QtRPT::setBackgroundImage(QPixmap &image) {
    m_backgroundImage = &image;
}

/*! \overload
 Sets background image from \a image
*/
void QtRPT::setBackgroundImage(QPixmap image) {
    m_backgroundImage = &image;
}

void QtRPT::drawBackground() {
    if (painter->isActive())
        painter->setBackgroundMode(Qt::TransparentMode);
    if (m_backgroundImage != 0) {
        if (painter->isActive())
            painter->drawPixmap(-ml*koefRes_w,
                               -mt*koefRes_h,
                               pw*koefRes_w,
                               ph*koefRes_h, *m_backgroundImage);
    }
}

void QtRPT::processGroupHeader(QPrinter *printer, int &y, bool draw, int pageReport) {
    m_recNo = 0;
    if (pageList.at(m_pageReport)->getBand(DataGroupHeader) == 0) {
        processMHeader(y,draw);
        processPFooter(draw);
        processMasterData(printer,y,draw,pageReport);
        processMFooter(printer,y,draw);
    } else {
        if (pageList.at(pageReport)->getBand(MasterData) != 0)
            fillListOfValue(pageList.at(pageReport)->getBand(MasterData));
        if (listOfPair.size() > 0) {
            if (recordCount.size()>=pageReport+1) {
                for (int i = 0; i < recordCount.at(pageReport); i++) {
                    m_recNo = i;
                    if (pageList.at(pageReport)->getBand(DataGroupHeader) != 0) {
                        sectionField(pageList.at(pageReport)->getBand(DataGroupHeader),
                                     pageList.at(pageReport)->getBand(DataGroupHeader)->groupingField, false, true);
                    }
                }
            }

            listOfGroup.clear();
            mg_recNo = 0;

            RptBandObject *bandObj = pageList.at(pageReport)->getBand(MasterHeader);
            if (bandObj != 0 && bandObj->showInGroup == 0)
                processMHeader(y,draw);

            int grpNo = 0;
            for (int j = 0; j < listOfPair.size(); ++j) {
                if (pageList.at(pageReport)->getBand(DataGroupHeader) !=0 && listOfPair.at(j).pageReport == pageReport && listOfPair.at(j).paramName == pageList.at(pageReport)->getBand(DataGroupHeader)->groupingField) {
                    bool founded = false;
                    for (int i=0; i < listOfGroup.size(); ++i) {
                        if (listOfGroup.at(i) == listOfPair.at(j).paramValue)
                            founded = true;
                    }

                    listIdxOfGroup.clear();
                    for (int k=0; k < listOfPair.size(); ++k) {
                        if (listOfPair.at(k).paramName == pageList.at(pageReport)->getBand(DataGroupHeader)->groupingField &&
                            listOfPair.at(k).pageReport == pageReport &&
                            listOfPair.at(k).paramValue.toString() == listOfPair.at(j).paramValue.toString()
                           ) {
                            //fill the idx for current group
                            listIdxOfGroup << listOfPair.at(k).lnNo;
                        }
                    }

                    if (!founded) { //Start new group
                        grpNo += 1;

                        //-----------Added codes here. Thanks to puterk
                        int yPF = 0;

                        if (pageList.at(pageReport)->getBand(PageFooter) != 0) {
                            yPF = pageList.at(pageReport)->getBand(PageFooter)->height;
                        }

                        int yMF = 0;
                        if (pageList.at(pageReport)->getBand(MasterFooter) != 0) {
                            yMF = pageList.at(pageReport)->getBand(MasterFooter)->height;
                        }
                        //-----------ends here. Thanks to puterk

                        listOfGroup << listOfPair.at(j).paramValue.toString();

                        if (pageList.at(pageReport)->getBand(DataGroupHeader)->startNewPage == 1 && grpNo != 1) //Start new page for each Data group
                            newPage(printer, y, draw);

                        m_recNo = listOfPair.at(j).lnNo;

                        //Start new numeration for group
                        if (pageList.at(pageReport)->getBand(DataGroupHeader)->startNewNumeration != 0)
                            mg_recNo = 0;

                        //-----------Added codes here. Thanks to puterk
                        if (y + pageList.at(pageReport)->getBand(DataGroupHeader)->height > ph-mb-mt-yPF-yMF)
                            newPage(printer, y, draw);
                        //-----------ends here. Thanks to puterk

                        if (allowPrintPage(draw,curPage))  //Draw header of the group
                            drawBandRow(pageList.at(pageReport)->getBand(DataGroupHeader), y);
                        y += pageList.at(pageReport)->getBand(DataGroupHeader)->height;

                        bandObj = pageList.at(pageReport)->getBand(MasterHeader);
                        if (bandObj != 0 && bandObj->showInGroup == 1)
                            processMHeader(y,draw);
                        processPFooter(draw);
                        processMasterData(printer,y,draw,pageReport);

                        bandObj = pageList.at(pageReport)->getBand(MasterFooter);
                        if (bandObj != 0 && bandObj->showInGroup == 1)
                            processMFooter(printer,y,draw);
                        //---
                        m_recNo = listOfPair.at(j).lnNo;

                        //-----------Added codes here. Thanks to puterk
                        if (y + pageList.at(pageReport)->getBand(DataGroupFooter)->height > ph-mb-mt-yPF-yMF)
                            newPage(printer, y, draw);
                        //-----------ends here. Thanks to puterk

                        if (pageList.at(pageReport)->getBand(DataGroupFooter) != 0) {
                            if (allowPrintPage(draw,curPage)) { //Draw footer of the group
                                drawBandRow(pageList.at(pageReport)->getBand(DataGroupFooter), y);
                            }
                            y += pageList.at(pageReport)->getBand(DataGroupFooter)->height;
                        }
                    }
                }
            }
            if (pageList.at(pageReport)->getBand(MasterFooter) != 0 && pageList.at(pageReport)->getBand(MasterFooter)->showInGroup == 0)
                processMFooter(printer,y,draw);
        }
    }
}

void QtRPT::processMasterData(QPrinter *printer, int &y, bool draw, int pageReport) {
    if (!recordCount.isEmpty()) {
        if (pageReport < recordCount.size() && recordCount.at(pageReport) > 0) {
            if (pageList.at(m_pageReport)->getBand(MasterData) != 0) {
                for (int i = 0; i < recordCount.at(pageReport); i++) {
                    m_recNo = i;

                    bool found = false;
                    //If report with groups, we checking that current line in the current group
                    if (listIdxOfGroup.size() > 0) {
                        for (int k = 0; k < listIdxOfGroup.size(); k++) {
                            if (listIdxOfGroup.at(k) == i)
                                found = true;
                        }
                    } else {
                        found = true;
                    }

                    if (found) {
                        mg_recNo += 1;
                        int yPF = 0;
                        if (pageList.at(pageReport)->getBand(PageFooter) != 0) {
                            yPF = pageList.at(m_pageReport)->getBand(PageFooter)->height;
                        }

                        int yMF = 0;
                        if (pageList.at(pageReport)->getBand(MasterFooter) != 0) {
                            yMF = pageList.at(pageReport)->getBand(MasterFooter)->height;
                        }

                        drawBandRow(pageList.at(pageReport)->getBand(MasterData), y, false);
                        if (y + pageList.at(pageReport)->getBand(MasterData)->realHeight > ph-mb-mt-yPF-yMF) {
                            if (m_printMode != QtRPT::Html) {
                                newPage(printer, y, draw);
                                processMHeader(y,draw);
                            }
                        }

                        if (allowPrintPage(draw,curPage)) {
                            drawBandRow(pageList.at(pageReport)->getBand(MasterData), y, true);
                        } else
                            fillListOfValue(pageList.at(pageReport)->getBand(MasterData));
                        y += pageList.at(m_pageReport)->getBand(MasterData)->realHeight;
                    }
                }
            }
        }
    }
}

void QtRPT::processMHeader(int &y, bool draw) {
    if (pageList.at(m_pageReport)->getBand(MasterHeader) == 0) return;
    if (allowPrintPage(draw,curPage)) drawBandRow(pageList.at(m_pageReport)->getBand(MasterHeader), y);
    y += pageList.at(m_pageReport)->getBand(MasterHeader)->height;
    //painter.drawLine(0,y*koefRes_h,r.width(),y*koefRes_h);
}

void QtRPT::processRTitle(int &y, bool draw) {
    if (pageList.at(m_pageReport)->getBand(ReportTitle) == 0) return;
    if (allowPrintPage(draw,curPage)) drawBandRow(pageList.at(m_pageReport)->getBand(ReportTitle), y);
    y += pageList.at(m_pageReport)->getBand(ReportTitle)->height;
    //painter.drawLine(0,y*koefRes_h,r.width(),y*koefRes_h);
}

void QtRPT::processPHeader(int &y, bool draw) {
    if (pageList.at(m_pageReport)->getBand(PageHeader) == 0) return;
    if (m_printMode == QtRPT::Html) return;
    if (allowPrintPage(draw,curPage)) drawBandRow(pageList.at(m_pageReport)->getBand(PageHeader), y);
    y += pageList.at(m_pageReport)->getBand(PageHeader)->height;
    //painter.drawLine(0,y*koefRes_h,pw*koefRes_h,y*koefRes_h);
}

void QtRPT::processMFooter(QPrinter *printer, int &y, bool draw) {
    if (pageList.at(m_pageReport)->getBand(MasterFooter) == 0) return;
    if (y > ph-mb-mt-pageList.at(m_pageReport)->getBand(MasterFooter)->height)
        newPage(printer, y, draw);
    if (allowPrintPage(draw,curPage))
        drawBandRow(pageList.at(m_pageReport)->getBand(MasterFooter), y);
    y += pageList.at(m_pageReport)->getBand(MasterFooter)->height;
}

void QtRPT::processPFooter(bool draw) {
    if (pageList.at(m_pageReport)->getBand(PageFooter) == 0) return;
    if (m_printMode == QtRPT::Html) return;
    int y1 = ph-mb-mt-pageList.at(m_pageReport)->getBand(PageFooter)->height;
    if (allowPrintPage(draw,curPage)) drawBandRow(pageList.at(m_pageReport)->getBand(PageFooter), y1);
    //painter.drawLine(0,y1*koefRes_h,pw*koefRes_h,y1*koefRes_h);
}

void QtRPT::processRSummary(QPrinter *printer, int &y, bool draw) {
    if (pageList.at(m_pageReport)->getBand(ReportSummary) == 0) return;
    if (y + pageList.at(m_pageReport)->getBand(ReportSummary)->height > ph-mb-mt-pageList.at(m_pageReport)->getBand(ReportSummary)->height)
        newPage(printer, y, draw);
    if (allowPrintPage(draw,curPage)) drawBandRow(pageList.at(m_pageReport)->getBand(ReportSummary), y);
    y += pageList.at(m_pageReport)->getBand(ReportSummary)->height;
    //painter.drawLine(0,y*koefRes_h,pw*koefRes_h,y*koefRes_h);
}

void QtRPT::openDataSource(int pageReport) {
    RptSqlConnection SqlConnection;

    getUserSqlConnection(pageReport, SqlConnection);

    if (SqlConnection.m_bIsActive) {
        // If user connection is active, use their parameters
        QString sqlQuery = SqlConnection.m_sqlQuery;
        rptSql = new RptSql(SqlConnection.m_dbType,SqlConnection.m_dbName,SqlConnection.m_dbHost,SqlConnection.m_dbUser,SqlConnection.m_dbPassword,SqlConnection.m_dbPort,SqlConnection.m_dbConnectionName,this);
        rptSql->setObjectName(SqlConnection.m_dsName);
        if (!m_sqlQuery.isEmpty())
            sqlQuery = m_sqlQuery;
        if (!rptSql->openQuery(sqlQuery,SqlConnection.m_dbCoding,SqlConnection.m_charsetCoding)) {
            recordCount << 0;
            return;
        }

        recordCount << rptSql->getRecordCount();
    }
    else {
        QDomElement docElem = xmlDoc.documentElement().childNodes().at(pageReport).toElement();
        QDomNode n = docElem.firstChild();
        QDomElement dsElement;
        while(!n.isNull()) {
            QDomElement e = n.toElement();
            if ((!e.isNull()) && (e.tagName() == "DataSource")) {
                dsElement = e;
            }
            n = n.nextSibling();
        }

        if (!dsElement.isNull() && dsElement.attribute("type") == "SQL") {
            QString dsName = dsElement.attribute("name");
            QString dbType = dsElement.attribute("dbType");
            QString dbName = dsElement.attribute("dbName");
            QString dbHost = dsElement.attribute("dbHost");
            QString dbUser = dsElement.attribute("dbUser");
            QString dbPassword = dsElement.attribute("dbPassword");
            QString dbCoding = dsElement.attribute("dbCoding");
            QString charsetCoding = dsElement.attribute("charsetCoding");
            QString sqlQuery = dsElement.text().trimmed();
            int dbPort = dsElement.attribute("dbPort").toInt();
            QString dbConnectionName = dsElement.attribute("dbConnectionName");
            rptSql = new RptSql(dbType,dbName,dbHost,dbUser,dbPassword,dbPort,dbConnectionName,this);
            rptSql->setObjectName(dsName);
            if (!m_sqlQuery.isEmpty())
                sqlQuery = m_sqlQuery;
            if (!rptSql->openQuery(sqlQuery,dbCoding,charsetCoding)) {
                recordCount << 0;
                return;
            }

            recordCount << rptSql->getRecordCount();
        }
        if (!dsElement.isNull() && dsElement.attribute("type") == "XML") {

        }
    }
}

/*!
 \fn QtRPT::setSqlQuery(QString sqlString)
  Sets SQL query \a sqlString.
  If in XML was defined SQL query, it will be replaced.
 */
void QtRPT::setSqlQuery(QString sqlString) {
    m_sqlQuery = sqlString;
}

/*!
 \fn void QtRPT::newPage(int page)
  This signal is emitted when the new page generated.
  \a page is a number of new page.
 */

/*!
 \fn void QtRPT::setField(RptFieldObject &fieldObject)
  This signal is emitted when QtRPT request a Field's data from user application.
  Pass \a fieldObject to user's application as a reference to requested RptFieldObject.
  User should set the appropriate properties of the \a fieldObject.
  This signal is emitted before following signals: setValue, setValueImage, setValueDiagram
  \sa setValue(), setValueImage(), setValueDiagram(), RptFieldObject
 */

/*!
 \fn void QtRPT::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage)
  This signal is emitted when QtRPT request a Field's data from user application.
  And pass to the user's application the following variables
    \list
    \li \a recNo - record number (if field placed on DataBand)
    \li \a paramName - name of requested field
    \li \a reportPage - number of the report
    \endlist
  From user's applications, QtRPT expects the data in the variable \a paramValue, which acts as a references
    \list
    \li \a paramValue - data, which will processed by QtRPT and printed
    \endlist
  This signal is emitted after following signal: setField()
  \sa setField(), setValueImage(), setValueDiagram()
 */

/*!
 \fn void QtRPT::setValueImage(const int recNo, const QString paramName, QImage &paramValue, const int reportPage);
  This signal is emitted when QtRPT request a Field's data from user application.
  And pass to the user's application the following variables
    \list
    \li \a recNo - record number (if field placed on DataBand)
    \li \a paramName - name of requested field
    \li \a reportPage - number of the report
    \endlist
  From user's applications, QtRPT expects the QImage in the variable \a paramValue, which acts as a references
    \list
    \li \a paramValue - QImage, which will processed by QtRPT and printed
    \endlist
  This signal is emitted after following signal: setField()
  \sa setField(), setValue(), setValueDiagram()
 */

/*!
 \fn void QtRPT::setValueDiagram(Chart &chart);
  This signal is emitted when QtRPT request a Chart's data from user application.
  Pass \a chart to user's application as a reference to requested Chart object.
  User should set the appropriate properties of the \a chart.
  This signal is emitted after following signal: setField()
  \sa setField(), setValue(), setValueImage()
 */

//-----------------------------
/*!
  \page qtrptproject.html
  \title QtRptProject
  \list
  \li Version 1.5.4
  \li Programmer: Aleksey Osipov
  \li Web-site: \l {http://www.aliks-os.tk} {http://www.aliks-os.tk}
  \li Email: \l {mailto:aliks-os@ukr.net} {aliks-os@ukr.net}
  \li Web-site: \l {http://www.qtrpt.tk} {http://www.qtrpt.tk}
  \li Address in Facebook \l {https://www.facebook.com/qtrpt} {https://www.facebook.com/qtrpt}
  \endlist
  QtRPT is the easy-to-use print report engine written in C++ QtToolkit. It allows combining several reports in one XML file. For separately taken field, you can specify some condition depending on which this field will display in different font and background color, etc. The project consists of two parts: report library QtRPT and report designer application QtRptDesigner. Report file is a file in XML format. The report designer makes easy to create report XML file. Thanks to Qt library, our project can be used in programs for work in the operating systems Windows, Linux, MacOS

  \contentspage {Basic Qt} {Contents}
  Table of contents:
  \list
  \li \l {QtRptName} {QtRptName Namespace}
  \li \l {QtRPT} {QtRPT class}
  \li \l {RptPageObject} {RptPageObject class}
  \li \l {RptBandObject} {RptBandObject class}
  \li \l {RptFieldObject} {RptFieldObject class}
  \endlist
*/

void QtRPT::setUserSqlConnection(int pageReport, const RptSqlConnection & SqlConnection)
{
    if (userSqlConnection.count() <= pageReport)
    {
        // If page does not exist, add new connection for this page
        userSqlConnection.resize(pageReport);
        userSqlConnection.insert(pageReport, SqlConnection);
    }
    else
    {
        // If page exists, replace connection for this page
        userSqlConnection.replace(pageReport, SqlConnection);
    }
}

void QtRPT::getUserSqlConnection(int pageReport, RptSqlConnection & SqlConnection)
{
    if (userSqlConnection.count() <= pageReport)
    {
        // Return inactive connection
        SqlConnection.reset();
    }
    else
    {
        SqlConnection = userSqlConnection.at(pageReport);
    }
}

void QtRPT::setUserSqlConnection(int pageReport, QString dsName, QString dbType, QString dbName, QString dbHost, QString dbUser, QString dbPassword, int dbPort, QString dbConnectionName, QString sqlQuery, QString dbCoding, QString charsetCoding)
{
    // Create enabled RptSqlConnection object with all parameters
    RptSqlConnection SqlConnection(dsName, dbType, dbName, dbHost, dbUser, dbPassword, dbPort, dbConnectionName, sqlQuery, dbCoding, charsetCoding);

    setUserSqlConnection(pageReport, SqlConnection);
}

void QtRPT::activateUserSqlConnection(int pageReport, bool bActive)
{
    RptSqlConnection SqlConnection;

    // Enable or disable connection
    getUserSqlConnection(pageReport, SqlConnection);

    SqlConnection.m_bIsActive = bActive;

    setUserSqlConnection(pageReport, SqlConnection);
}

