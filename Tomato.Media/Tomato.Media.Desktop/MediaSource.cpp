//
// Tomato Media
// 媒体源
// 
// 作者：SunnyCase 
// 创建日期 2015-08-05
#include "pch.h"
#include "MediaSource.h"

using namespace NS_MEDIA;

MediaSource::MediaSource()
{
	
}

HRESULT MediaSource::get_Title(BSTR * title)
{
	try
	{
		*title = bstr_t(mediaSource.Title.c_str());
	}
	CATCH_ALL();
	return S_OK;
}

HRESULT MediaSource::get_Album(BSTR * album)
{
	try
	{
		*album = bstr_t(mediaSource.Album.c_str());
	}
	CATCH_ALL();
	return S_OK;
}

HRESULT MediaSource::Open(IStream * pStream)
{
	try
	{
		mediaSource.Open(pStream);
	}
	CATCH_ALL();
	return S_OK;
}
