#ifndef AOCLOCKLABEL_H
#define AOCLOCKLABEL_H

#include <QLabel>
#include <QBasicTimer>
#include <QTimerEvent>
#include <QDateTime>
#include <QDebug>

class AOClockLabel : public QLabel {
  Q_OBJECT

public:
  AOClockLabel(QWidget *parent);
  void start();
  void start(qint64 msecs);
  void set(qint64 msecs, bool update_text = false);
  void pause();
  void stop();
  void set_format(QString formatting, qint64 timer_value);
  void skip(qint64 msecs);
  bool active();

protected:
  void timerEvent(QTimerEvent *event) override;

private:
    QBasicTimer timer;
    QDateTime target_time;
    QString time_format = "hh:mm:ss";
};

#endif // AOCLOCKLABEL_H
