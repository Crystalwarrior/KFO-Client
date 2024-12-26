#include "aoapplication.h"
#include "options.h"


#include <QGraphicsVideoItem>
#include <QVideoWidget>
#include <QMediaPlayer>

class VideoScreen : public QGraphicsVideoItem
{
  Q_OBJECT

public:
  VideoScreen(AOApplication *p_ao_app, QGraphicsItem *parent = nullptr);
  ~VideoScreen();

  QString get_file_name() const;

  void update_audio_output();

  void set_volume(int p_value);

public slots:
  void set_file_name(QString file_name);

  void play_character_video(QString character, QString video);

  void play();

  void stop();

signals:
  void started();

  void finished();

private:
  QWidget *m_parent;

  AOApplication *ao_app;

  QString m_file_name;

  bool m_scanned;

  bool m_video_available;

  bool m_running;

  QMediaPlayer *m_player;

  void start_playback();

  void finish_playback();

private slots:
  void update_video_availability(bool);

  void check_status(QMediaPlayer::MediaStatus);

  void check_state(QMediaPlayer::State);
};
