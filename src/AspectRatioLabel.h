#pragma once
#include <QLabel>
#include <QResizeEvent>
#include <QPainter>

class AspectRatioLabel : public QLabel
{
public:
    explicit AspectRatioLabel(QWidget *parent = nullptr)
        : QLabel(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setAlignment(Qt::AlignCenter);
        setStyleSheet("background-color: #404040; color: #CCCCCC;"); // dark gray bg, light text
    }

    void clearPreview()
    {
        setPixmap(QPixmap());
        update();             
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QLabel::resizeEvent(event);

        int w = width();
        int h = height();

        int targetHeight = w * 2 / 3;
        int targetWidth = h * 3 / 2;

        if (targetHeight <= h)
        {
            setMaximumSize(QWIDGETSIZE_MAX, h);
            setMinimumSize(w, 0);
        }
        else
        {
            setMaximumSize(w, QWIDGETSIZE_MAX);
            setMinimumSize(0, h);
        }
    }

    QSize sizeHint() const override
    {
        int w = width();
        int h = height();
        int targetHeight = w * 2 / 3;
        int targetWidth = h * 3 / 2;

        if (targetHeight <= h)
            return QSize(w, targetHeight);
        else
            return QSize(targetWidth, h);
    }

    void paintEvent(QPaintEvent *event) override
    {
        if (!pixmap().isNull())
        {

            QLabel::paintEvent(event);
        }
        else
        {
            QPainter painter(this);
            painter.fillRect(rect(), QColor("#404040")); 

            painter.setPen(QColor("#CCCCCC")); 
            painter.setFont(font());
            painter.drawText(rect(), Qt::AlignCenter, "No Preview");

            painter.setPen(QPen(QColor(200,200,200,80), 2));
            painter.drawLine(rect().topLeft(), rect().bottomRight());
            painter.drawLine(rect().topRight(), rect().bottomLeft());
        }
    }
};