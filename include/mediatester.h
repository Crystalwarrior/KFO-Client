#include <QMediaPlayer>
#include <QObject>

class MediaTester : public QObject
{
  Q_OBJECT

public:
  MediaTester(QObject *parent = nullptr);
  ~MediaTester();

signals:
  void done();

private:
  QMediaPlayer m_player;

private slots:
  void p_check_status(QMediaPlayer::MediaStatus p_status);
};