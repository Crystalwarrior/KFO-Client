/**************************************************************************
**
** mk2
** Copyright (C) 2022 Tricky Leifa
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Affero General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
**************************************************************************/
#pragma once

#include "aoapplication.h"
#include "options.h"

#include <QVideoWidget>
#include <QMediaPlayer>

class VideoScreen : public QVideoWidget
{
  Q_OBJECT

public:
  VideoScreen(QWidget *parent, AOApplication *ao_app);
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
