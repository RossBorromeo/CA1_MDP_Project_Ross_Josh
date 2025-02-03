//Ross - D00241095 | Josh - D00238448
#include "MusicPlayer.hpp"

//Ross - added new Music and In game music for our game.
MusicPlayer::MusicPlayer()
	: m_volume(7.f)
{
	m_filenames[MusicThemes::kMenuTheme] = "Media/Music/MenuMusic.mp3";
	m_filenames[MusicThemes::kMissionTheme] = "Media/Music/GameMusic.mp3";
}

void MusicPlayer::Play(MusicThemes theme)
{
	std::string filename = m_filenames[theme];

	if (!m_music.openFromFile(filename))
		throw std::runtime_error("Music " + filename + " could not be loaded.");

	m_music.setVolume(m_volume);
	m_music.setLoop(true);
	m_music.play();
}

void MusicPlayer::Stop()
{
	m_music.stop();
}

void MusicPlayer::SetVolume(float volume)
{
	m_volume = volume;
}

void MusicPlayer::SetPaused(bool paused)
{
	if (paused)
		m_music.pause();
	else
		m_music.play();
}