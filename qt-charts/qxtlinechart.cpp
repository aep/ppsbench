/*
Copyright (c) 2010 Arvid Picciani

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "qxtlinechart.h"
#include <QPainter>
#include <QPalette>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QFontMetrics>

class QxtLineChart::Private{
public:
    Private(QxtLineChart * p_)
        :p(p_)
        ,ydensity(3)
        ,rx_min(0)
        ,rx_max(1)
        ,ry_min(0)
        ,ry_max(1)
        ,s_x(1)
        ,s_y(1)
        ,headersize_x(0)
        ,headersize_y(0)
        ,textpadding(12)
        ,graphpadding(10)
        ,sizeHint(1,1)
    {
    }
    QxtLineChart * p;

    QList<QPoint> data;
    QList<QString> xhead;
    int ydensity;

    int rx_min,rx_max,ry_min,ry_max;
    qreal s_x;
    qreal s_y;
    int headersize_x,headersize_y;


    int fonth;
    int textpadding;
    int graphpadding;

    QString yprefix,ysuffix;

    QSize sizeHint;

    void recalc_matrix(){
        QFontMetrics metrics(p->font());
        fonth=metrics.height();

        if(ydensity>0){
            headersize_x=metrics.width(yprefix+"-"+QString::number(ry_max)+ysuffix)+(textpadding*2);
        }else{
            headersize_x=0;
        }

        if(xhead.isEmpty()){
            headersize_y=0;
        }else{
            headersize_y=fonth+(textpadding*2);
        }

        s_x=(p->width()-headersize_x-(graphpadding*2))/(qreal)(rx_max-rx_min);
        s_y=(p->height()-headersize_y-(graphpadding*2)  )/(qreal)(ry_max-ry_min);


        //calc sizehint
        int h=20;
        int w=headersize_x+(data.size()*6);
        if(!xhead.isEmpty()){
            int w_=headersize_x;
            foreach(QString s,xhead){
                w+=metrics.width(s)+textpadding;
            }
            w=qMax(w,w_);
        }
        sizeHint=QSize(w,h);
    }

    qreal mapX(qreal x){
        return (x-rx_min)*s_x;
    }
    qreal mapY(qreal y){
        return p->height()-((y-ry_min)*s_y)-graphpadding;
    }

};


QxtLineChart::QxtLineChart(QWidget * parent)
    :QWidget(parent)
    ,d(new Private(this))
    {
        setBackgroundRole(QPalette::Base);
    }

QSize QxtLineChart::sizeHint () const{
    return d->sizeHint;
}

void QxtLineChart::setData(QList<QPoint> list){
    d->data=list;
    update();
}
void QxtLineChart::setXHeaderData(QList<QString> head){
    d->xhead=head;
    d->recalc_matrix();
    update();
}
void QxtLineChart::setYHeaderDensity(int v){
    d->ydensity=v;
    d->recalc_matrix();
    update();
}

void QxtLineChart::setXRange(int min,int max){
    d->rx_min=min;
    d->rx_max=max;
    d->recalc_matrix();
    update();
}
void QxtLineChart::setYRange(int min,int max){
    d->ry_min=min;
    d->ry_max=max;
    d->recalc_matrix();
    update();
}

void QxtLineChart::setYSuffix(QString s){
    d->ysuffix=s;
    d->recalc_matrix();
    update();
}

void QxtLineChart::setYPrefix(QString s){
    d->yprefix=s;
    d->recalc_matrix();
    update();
}


void QxtLineChart::resizeEvent ( QResizeEvent * event ){
    d->recalc_matrix();
}

void QxtLineChart::paintEvent ( QPaintEvent * event ){
    QPainter painter(this);
    QFontMetrics metrics(font());

    //draw y header
    if(d->ydensity>0){
        int snum=(height()-d->headersize_y)/(d->fonth+4)/d->ydensity;
        int step=(d->ry_max-d->ry_min)/(qreal)snum;
        if(step<=0){
            return;
        }
        int i=0;
        bool down=false;
        forever{
            int y=d->mapY(i)-d->headersize_y;
            painter.setPen(palette().color(QPalette::AlternateBase));
            painter.drawLine(d->headersize_x,y,width(),y);
            //painter.drawLine(d->headersize_x,0,d->headersize_x,height()-d->headersize_y);
            painter.setPen(palette().color(QPalette::Text));
            painter.drawText(QRect(0,y-(d->fonth/2),d->headersize_x-(d->textpadding*2),d->fonth),Qt::AlignRight,d->yprefix+QString::number(i)+d->ysuffix);

            if(down){
                i-=step;
                if(i<d->ry_min){
                    break;
                }
            }else{
                i+=step;
                if(i>d->ry_max){
                    down=true;
                    i=0;
                }
            }
        }
    }

    QPen pen(palette().color(QPalette::Highlight));
    pen.setWidth(4);


    QPainterPath path;
    for(int i=0;i<d->data.size();i++){
        QPointF p(
                  d->mapX(d->data.at(i).x())+d->headersize_x,
                  d->mapY(d->data.at(i).y())-d->headersize_y
                  );
        painter.setPen(pen);
        painter.drawPoint(p);
        if(i==0){
            path.moveTo(p);
        }else{
            path.lineTo(p);
        }



        if(!d->xhead.isEmpty()){
            //draw x header
            painter.setPen(palette().color(QPalette::Text));
            QString text=d->xhead.at(i);
            painter.drawText(p.x()-(metrics.width(text)/2),height()-d->fonth,text);
        }

    }

    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(path);
}
