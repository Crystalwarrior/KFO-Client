#include <QGraphicsScene>
#include <QGraphicsView>

#include <QGraphicsItem>
#include <QObject>
#include <QResizeEvent>

#include <QDebug>

class AOGraphicsView : public QGraphicsView
{
  Q_OBJECT

public:
  AOGraphicsView(QWidget *parent = nullptr);
  ~AOGraphicsView();

protected:
  void resizeEvent(QResizeEvent *event) final;

private:
  QGraphicsScene *m_scene;
};
