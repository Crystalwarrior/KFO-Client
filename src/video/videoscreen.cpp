#include "video/videoscreen.h"

#include <QAudioOutputSelectorControl>
#include <QMediaService>
#include <QStyleOptionGraphicsItem>
#include <QUrl>

VideoScreen::VideoScreen(AOApplication *p_ao_app, QGraphicsItem *parent)
    : QGraphicsVideoItem(parent)
    , m_scanned(false)
    , m_video_available(false)
    , m_running(false)
    , m_player(new QMediaPlayer(this, QMediaPlayer::LowLatency))
{
  ao_app = p_ao_app;
  setAspectRatioMode(Qt::KeepAspectRatioByExpanding);

  m_player->setVideoOutput(this);

  connect(m_player, SIGNAL(videoAvailableChanged(bool)), this, SLOT(update_video_availability(bool)));
  connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(check_status(QMediaPlayer::MediaStatus)));
  connect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(check_state(QMediaPlayer::State)));

  update_audio_output();
}

VideoScreen::~VideoScreen()
{}

QString VideoScreen::get_file_name() const
{
  return m_file_name;
}

void VideoScreen::set_file_name(QString p_file_name)
{
  if (m_file_name == p_file_name)
  {
    return;
  }
  stop();
  qInfo() << "loading media file" << p_file_name;
  m_scanned = false;
  m_video_available = false;
  m_file_name = p_file_name;
  if (m_file_name.isEmpty())
  {
    m_scanned = true;
  }
  m_player->setMedia(QUrl::fromLocalFile(m_file_name));
}

void VideoScreen::play_character_video(QString p_charname, QString p_video)
{
  QVector<VPath> pathlist { // cursed character path resolution vector
      ao_app->get_character_path(
          p_charname, "videos/" + p_video),
      ao_app->get_character_path(
          p_charname, p_video),
      VPath(p_video) // The path by itself after the above fail
  };

  const QString l_filepath = ao_app->get_asset_path(pathlist);
  if (l_filepath.isEmpty())
  {
    qWarning() << "error: no character media file" << p_charname << p_video;
    finish_playback();
    return;
  }
  set_file_name(l_filepath);
  play();
}

void VideoScreen::play()
{
  stop();
  m_running = true;
  if (!m_scanned)
  {
    return;
  }
  if (!m_video_available)
  {
    finish_playback();
    return;
  }
  start_playback();
}

void VideoScreen::stop()
{
  m_running = false;
  if (m_player->state() != QMediaPlayer::StoppedState)
  {
    m_player->stop();
  }
}

void VideoScreen::update_video_availability(bool p_video_available)
{
  m_video_available = p_video_available;
}

void VideoScreen::check_status(QMediaPlayer::MediaStatus p_status)
{
  if (m_running)
  {
    switch (p_status)
    {
    case QMediaPlayer::InvalidMedia:
      m_scanned = true;
      qWarning() << "error: media file is invalid:" << m_file_name;
      finish_playback();
      break;

    case QMediaPlayer::NoMedia:
      m_scanned = true;
      finish_playback();
      break;

    case QMediaPlayer::LoadedMedia:
    case QMediaPlayer::BufferedMedia:
      m_scanned = true;
      start_playback();
      break;

    default:
      break;
    }
  }
}

void VideoScreen::check_state(QMediaPlayer::State p_state)
{
  switch (p_state)
  {
  case QMediaPlayer::PlayingState:
    emit started();
    break;

  case QMediaPlayer::StoppedState:
    if (m_running)
    {
      finish_playback();
    }
    break;

  default:
    break;
  }
}

void VideoScreen::start_playback()
{
  this->show();
  if (m_player->state() == QMediaPlayer::StoppedState)
  {
    update_audio_output();
    m_player->play();
  }
}

void VideoScreen::finish_playback()
{
  stop();
  emit finished();
}

void VideoScreen::update_audio_output()
{
  QMediaService *l_service = m_player->service();
  if (!l_service)
  {
    qWarning() << "error: missing media service, device unchanged";
    return;
  }

  QAudioOutputSelectorControl *l_control = l_service->requestControl<QAudioOutputSelectorControl *>();
  if (!l_control)
  {
    qWarning() << "error: missing audio output control, device unchanged";
  }
  else
  {
    const QStringList l_device_name_list = l_control->availableOutputs();
    for (const QString &i_device_name : l_device_name_list)
    {
      const QString l_device_description = l_control->outputDescription(i_device_name);
      if (i_device_name == Options::getInstance().audioOutputDevice())
      {
        l_control->setActiveOutput(i_device_name);
        qDebug() << "Media player changed audio device to" << i_device_name;
        break;
      }
    }
    return;
  }
  l_service->releaseControl(l_control);
}

void VideoScreen::set_volume(int p_value)
{
  if (m_player->volume() == p_value)
  {
    return;
  }
  m_player->setVolume(p_value);
}
