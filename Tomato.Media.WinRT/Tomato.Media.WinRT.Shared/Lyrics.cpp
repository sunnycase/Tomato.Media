//
// Tomato Media
// 歌词（LRC）解析
// 
// (c) SunnyCase 
// 创建日期 2015-04-29
#include "pch.h"
#include "Lyrics.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Tomato::Media;

LyricsAnalyzer::LyricsAnalyzer(String ^ source)
{
	Analyze(source);
}

void LyricsAnalyzer::Analyze(String ^ source)
{
	if (!source)
		throw ref new InvalidArgumentException(L"source cannot be null。");
	
	std::vector<LyricsId> ids;
	std::vector<LyricsLine> lines;
	AnaylizeLyrics(begin(source), end(source), std::back_inserter(ids) , std::back_inserter(lines));

}
