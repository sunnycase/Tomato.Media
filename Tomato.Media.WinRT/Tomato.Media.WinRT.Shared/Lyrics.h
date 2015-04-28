//
// Tomato Media
// 歌词（LRC）解析
// 
// (c) SunnyCase 
// 创建日期 2015-04-29
#pragma once

namespace Tomato
{
	namespace Media
	{
		///<summary>歌词标签</summary>
		public value class LyricsTag
		{
		public:
			///<summary>键</summary>
			Platform::String^ Key;
			///<summary>值</summary>
			Platform::String^ Value;
		};

		///<summary>歌词的一行</summary>
		public value class LyricsRow
		{
		public:
			///<summary>歌词显示的时刻</summary>
			Windows::Foundation::TimeSpan Time;
			///<summary>歌词文本</summary>
			Platform::String^ Text;
		};

		///<summary>歌词解析器</summary>
		public ref class LyricsAnalyzer sealed
		{
		public:
			///<summary>分析歌词</summary>
			///<param name="source">LRC 文本内容</param>
			LyricsAnalyzer(Platform::String^ source);

			///<summary>歌词的所有行</summary>
			property Windows::Foundation::Collections::IIterable<LyricsRow>^ Rows
			{
				Windows::Foundation::Collections::IIterable<LyricsRow>^ get()
				{
					return rows;
				}
			}

			///<summary>歌词的标签</summary>
			property Windows::Foundation::Collections::IIterable<LyricsTag>^ Tags
			{
				Windows::Foundation::Collections::IIterable<LyricsTag>^ get()
				{
					return tags;
				}
			}
		private:
			void Analyze(Platform::String^ source);
		private:
			Platform::Collections::Vector<LyricsRow>^ rows;
			Platform::Collections::Vector<LyricsTag>^ tags;
		};
	}
}

namespace std
{
	inline bool operator==(const Windows::Foundation::TimeSpan& left, const Windows::Foundation::TimeSpan& right) noexcept
	{
		return left.Duration == right.Duration;
	}

	inline bool operator==(const Tomato::Media::LyricsRow& left, const Tomato::Media::LyricsRow& right) noexcept
	{
		return left.Time == right.Time && left.Text == right.Text;
	}

	inline bool operator==(const Tomato::Media::LyricsTag& left, const Tomato::Media::LyricsTag& right) noexcept
	{
		return left.Key == right.Key && left.Value == right.Value;
	}
}