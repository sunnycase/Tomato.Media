//
// Tomato Media
// 媒体元数据
// 
// (c) SunnyCase 
// 创建日期 2015-03-18
#include "pch.h"
#include "../include/MediaMetadata.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace concurrency;

#define DEFINE_MEDIAMETA(name) const std::wstring DefaultMediaMetadatas:: ## name ## ::Name = L#name

DEFINE_MEDIAMETA(Title);
DEFINE_MEDIAMETA(Album);
DEFINE_MEDIAMETA(Artist);
DEFINE_MEDIAMETA(AlbumArtist);
DEFINE_MEDIAMETA(Year);
DEFINE_MEDIAMETA(TrackNumber);
DEFINE_MEDIAMETA(Genre);
DEFINE_MEDIAMETA(Lyrics);