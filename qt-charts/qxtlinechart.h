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
#ifndef QXT_LINE_CHART_H
#define QXT_LINE_CHART_H

#include <QWidget>
#include <QMap>
#include <memory>

class QxtLineChart : public QWidget {
public:
    QxtLineChart(QWidget * parent=0);
    virtual QSize sizeHint () const;
    void setData(QList<QPoint> list);
    void setXHeaderData(QList<QString> head);
    void setYHeaderDensity(int);
    void setXRange(int min,int max);
    void setYRange(int min,int max);
    void setYSuffix(QString s);
    void setYPrefix(QString s);
protected:
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void paintEvent ( QPaintEvent * event );
private:
    class Private;
    std::auto_ptr<Private> d;
};

#endif

