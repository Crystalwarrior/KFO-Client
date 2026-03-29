#include "mediatester.h"

#include <QAudioOutput>
#include <QUrl>

#include "debug_functions.h"

MediaTester::MediaTester(QObject *parent) : QObject(parent)
{
  QAudioOutput *audioOutput = new QAudioOutput(this);
  m_player.setAudioOutput(audioOutput);
  audioOutput->setMuted(true);

  connect(&m_player, &QMediaPlayer::mediaStatusChanged, this,
          &MediaTester::p_check_status);

  m_player.setSource(QUrl("qrc:/resource/data_sample.avi"));
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
