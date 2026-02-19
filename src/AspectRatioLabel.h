#pragma once
#include <QLabel>
#include <QResizeEvent>

class AspectRatioLabel : public QLabel
{
public:
    explicit AspectRatioLabel(QWidget *parent = nullptr)
        : QLabel(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setAlignment(Qt::AlignCenter);
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
};
