#include "mediatester.h"

#include <QUrl>

#include "debug_functions.h"

MediaTester::MediaTester(QObject *parent)
    : QObject(parent)
{
  m_player.setMuted(true);

  connect(&m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(p_check_status(QMediaPlayer::MediaStatus)));

  m_player.setMedia(QUrl("qrc:/resource/data_sample.avi"));
}

MediaTester::~MediaTester()
{}

void MediaTester::p_check_status(QMediaPlayer::MediaStatus p_status)
{
  switch (p_status)
  {
  case QMediaPlayer::InvalidMedia:
    call_notice(tr("Your operating system appears to not support video playback!"
                 "<br /><br />In order for videos to play properly, you "
                 "will need to install additional codecs, such as "
                 "<br /><a href=\"https://codecguide.com/download_k-lite_codec_pack_basic.htm\">the K-Lite Basic Codec Pack</a>."));
    emit done();
    break;

  case QMediaPlayer::LoadedMedia:
    emit done();
    break;

  default:
    break;
  }
}
