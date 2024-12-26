#include "aographicsview.h"

AOGraphicsView::AOGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
{
  setInteractive(false);

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  setFrameShape(QFrame::NoFrame);
  setFrameStyle(0);

  // Currently only used for video playback so always assume smooth transform for now
  setRenderHints(QPainter::SmoothPixmapTransform);

  setScene(m_scene);

  setBackgroundBrush(Qt::transparent);
}

AOGraphicsView::~AOGraphicsView()
{}

void AOGraphicsView::resizeEvent(QResizeEvent *event)
{
  QGraphicsView::resizeEvent(event);
  for (QGraphicsItem *i_item : scene()->items())
  {
    auto l_object = dynamic_cast<QObject *>(i_item);
    if (l_object)
    {
      l_object->setProperty("size", event->size());
    }
  }
  m_scene->setSceneRect(rect());
  setSceneRect(rect());
}
