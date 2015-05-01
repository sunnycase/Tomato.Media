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

	// 填充 Tag
	std::vector<LyricsTag> tagsTmp;
	for (auto& id : ids)
		tagsTmp.emplace_back(LyricsTag{ ref new String(id.key.c_str(), id.key.size()),
		ref new String(id.value.c_str(), id.value.size()) });
	tags = ref new Vector<LyricsTag>(std::move(tagsTmp));

	// 填充 Row
	std::vector<LyricsRow> rowsTmp;
	for (auto& line : lines)
	{
		auto text = ref new String(line.text.c_str(), line.text.size());
		for (auto time : line.time)
		{
			TimeSpan timeSpan;
			timeSpan.Duration = static_cast<long long>((time.minutes * 60 + time.seconds) * 1e7);
			rowsTmp.emplace_back(LyricsRow{ timeSpan, text });
		}
	}
	// 按时间排序
	std::sort(rowsTmp.begin(), rowsTmp.end(), [](const LyricsRow& left, const LyricsRow& right)
	{
		return left.Time.Duration < right.Time.Duration;
	});
	rows = ref new Vector<LyricsRow>(std::move(rowsTmp));
}
