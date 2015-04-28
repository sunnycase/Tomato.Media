//
// Tomato Media
// 歌词（LRC）解析
// 
// (c) SunnyCase 
// 创建日期 2015-04-28
#pragma once
#include "platform.h"
#include <iterator>
#include <vector>
#include <sstream>

NSDEF_TOMATO_MEDIA

struct LyricsId
{
	std::wstring key;
	std::wstring value;
};

struct LyricsTime
{
	uint32_t minutes;
	float seconds;
};

// LRC 的一行
struct LyricsLine
{
	std::vector<LyricsTime> time;
	std::wstring text;
};

namespace details {
	namespace lyrics
	{
		// 歌词分析状态
		enum LyricsAnaylizeState
		{
			FindNewId,
			FindNewLine
		};

		struct AnaylizeContext
		{
			uint32_t line;
			uint32_t pos;

			void new_line() noexcept
			{
				line++;
				pos = 0;
			}
		};

		struct maybe_time_exception
		{

		};

		namespace throwhelper
		{
			MEDIA_CORE_API __declspec(noreturn) void throw_unexpected_end();
			MEDIA_CORE_API __declspec(noreturn) void throw_unexpected(const AnaylizeContext& context, char chr);
		}

		template<typename InputIterator>
		LyricsId ReadLyricsId(AnaylizeContext& context, InputIterator& inputBegin, InputIterator inputEnd)
		{
			enum State : size_t
			{
				BeginBracket,	// 开头的方括号
				Key,			// 键
				KeyOrColon,		// 键或冒号
				Value,			// 值
				ValueOrEndBracket,	// 值或结尾方括号
				End				// 结束
			} state = BeginBracket;

			using elem_t = std::remove_const_t<std::remove_reference_t<decltype(*inputBegin)>>;

			struct LyricsIdTemp
			{
				std::basic_stringstream<elem_t> key;
				std::basic_stringstream<elem_t> value;
			};

			std::function<State(LyricsIdTemp&, elem_t)> processors[]{
				// BeginBracket
				[&](LyricsIdTemp& temp, elem_t elem)
				{
					if (elem == '[')
						return Key;
					else
						throwhelper::throw_unexpected(context, elem);
				},
				// Key
				[&](LyricsIdTemp& temp, elem_t elem)
				{
					// 歌词时间
					if (elem >= '0' && elem <= '9')
						throw maybe_time_exception();
					if (elem == ':' || elem == ']' || elem == '\r' || elem == '\n')
						throwhelper::throw_unexpected(context, elem);
					else
						temp.key << elem;
					return KeyOrColon;
				},
				// KeyOrColon
				[&](LyricsIdTemp& temp, elem_t elem)
				{
					if (elem == ':')
						return Value;
					else if (elem == ']' || elem == '\r' || elem == '\n')
						throwhelper::throw_unexpected(context, elem);
					else
					{
						temp.key << elem;
						return KeyOrColon;
					}
				},
				// Value
				[&](LyricsIdTemp& temp, elem_t elem)
				{
					if (elem == ']' || elem == '\r' || elem == '\n')
						throwhelper::throw_unexpected(context, elem);
					else
						temp.value << elem;
					return ValueOrEndBracket;
				},
				// ValueOrEndBracket
				[&](LyricsIdTemp& temp, elem_t elem)
				{
					if (elem == ']')
						return End;
					if (elem == '\r' || elem == '\n')
						throwhelper::throw_unexpected(context, elem);
					else
						temp.value << elem;
					return ValueOrEndBracket;
				}
			};

			LyricsIdTemp tmpResult;
			while (inputBegin != inputEnd && *inputBegin)
			{
				assert(state < _countof(processors));
				state = processors[state](tmpResult, *inputBegin);
				++inputBegin; ++context.pos;
				if (state == End)
					return{ tmpResult.key.str(), tmpResult.value.str() };
			}
			throwhelper::throw_unexpected_end();
		}

		template<typename InputIterator>
		LyricsLine ReadLyricsLine(AnaylizeContext& context, InputIterator& inputBegin, InputIterator inputEnd)
		{
			enum State : size_t
			{
				BeginBracket,	// 开头的方括号
				Minutes,		// 分钟数
				MinutesOrColon,	// 分钟数或冒号
				Seconds,		// 秒数
				SecondsOrDot,	// 秒数或点
				HundredsSec,	// 百分秒数
				HundredsSecOrEndBracket,	// 百分秒数或结尾方括号
				BeginBracketOrText,	// 开头的方括号或文本
				Text,			// 文本
				End				// 结束
			} state = BeginBracket;

			using elem_t = std::remove_const_t<std::remove_reference_t<decltype(*inputBegin)>>;

			struct LyricsLineTemp
			{
				std::vector<LyricsTime> time;
				std::vector<LyricsTime>::iterator cntTime;
				float cntHundredSecScale;
				std::basic_stringstream<elem_t> text;
			};

			std::function<State(LyricsLineTemp&, elem_t)> processors[]{
				// BeginBracket
				[&](LyricsLineTemp& temp, elem_t elem)
				{
					if (elem == '[')
					{
						temp.time.emplace_back(LyricsTime{ 0, 0.f });
						temp.cntTime = --temp.time.end();
						return Minutes;
					}
					else
						throwhelper::throw_unexpected(context, elem);
				},
				// Minutes
				[&](LyricsLineTemp& temp, elem_t elem)
				{
					if (elem >= '0' && elem <= '9')
					{
						auto value = temp.cntTime->minutes;
						value = value * 10 + static_cast<uint32_t>(elem - '0');
						temp.cntTime->minutes = value;
						return MinutesOrColon;
					}
					else
						throwhelper::throw_unexpected(context, elem);
				},
				// MinutesOrColon
				[&](LyricsLineTemp& temp, elem_t elem)
				{
					if (elem == ':')
						return Seconds;
					else if (elem >= '0' && elem <= '9')
					{
						auto value = temp.cntTime->minutes;
						value = value * 10 + static_cast<uint32_t>(elem - '0');
						temp.cntTime->minutes = value;
						return MinutesOrColon;
					}
					else
						throwhelper::throw_unexpected(context, elem);
				},
				// Seconds
				[&](LyricsLineTemp& temp, elem_t elem)
				{
					if (elem >= '0' && elem <= '9')
					{
						auto value = temp.cntTime->seconds;
						value = value * 10 + static_cast<float>(elem - '0');
						temp.cntTime->seconds = value;
						return SecondsOrDot;
					}
					else
						throwhelper::throw_unexpected(context, elem);
				},
				// SecondsOrDot
				[&](LyricsLineTemp& temp, elem_t elem)
				{
					if (elem == '.')
					{
						temp.cntHundredSecScale = 0.1f;
						return HundredsSec;
					}
					else if (elem >= '0' && elem <= '9')
					{
						auto value = temp.cntTime->seconds;
						value = value * 10 + static_cast<float>(elem - '0');
						temp.cntTime->seconds = value;
						return SecondsOrDot;
					}
					else
						throwhelper::throw_unexpected(context, elem);
				},
				// HundredsSec
				[&](LyricsLineTemp& temp, elem_t elem)
				{
					if (elem >= '0' && elem <= '9')
					{
						temp.cntTime->seconds += static_cast<float>(elem - '0')
							* temp.cntHundredSecScale;
						temp.cntHundredSecScale /= 10.f;
						return HundredsSecOrEndBracket;
					}
					else
						throwhelper::throw_unexpected(context, elem);
				},
				// HundredsSecOrEndBracket
				[&](LyricsLineTemp& temp, elem_t elem)
				{
					if (elem == ']')
						return BeginBracketOrText;
					else if (elem >= '0' && elem <= '9')
					{
						temp.cntTime->seconds += static_cast<float>(elem - '0')
							* temp.cntHundredSecScale;
						temp.cntHundredSecScale /= 10.f;
						return HundredsSecOrEndBracket;
					}
					else
						throwhelper::throw_unexpected(context, elem);
				},
				// BeginBracketOrText
				[&](LyricsLineTemp& temp, elem_t elem)
				{
					if (elem == '[')
					{
						temp.time.emplace_back(LyricsTime{ 0, 0.f });
						temp.cntTime = --temp.time.end();
						return Minutes;
					}
					else if (elem == '\r' || elem == '\n')
						return End;
					else
					{
						temp.text << elem;
						return Text;
					}
				},
				// Text
				[&](LyricsLineTemp& temp, elem_t elem)
				{
					if (elem == '\r' || elem == '\n')
						return End;
					else
					{
						temp.text << elem;
						return Text;
					}
				}
			};

			LyricsLineTemp tmpResult;
			while (inputBegin != inputEnd && *inputBegin)
			{
				assert(state < _countof(processors));
				state = processors[state](tmpResult, *inputBegin);
				++inputBegin; ++context.pos;
				if (state == End)
					return { tmpResult.time, tmpResult.text.str() };
			}
			throwhelper::throw_unexpected_end();
		}
	}
}

///<summary>分析 LRC 歌词</summary>
template<typename InputIterator, typename IdOutputIterator, typename LineOutputIterator>
void AnaylizeLyrics(InputIterator inputBegin, InputIterator inputEnd, IdOutputIterator idOutputInserter, LineOutputIterator lineOutputInserter)
{
	using namespace details::lyrics;

	AnaylizeContext context{ 0,0 };

	auto state = LyricsAnaylizeState::FindNewId;
	while (inputBegin != inputEnd)
	{
		auto cnt = *inputBegin;
		if (!cnt)
			break;
		if (cnt == '\n')
		{
			context.new_line();
			++inputBegin;
		}
		else if (cnt == '\r')
		{
			++context.pos;
			++inputBegin;
		}
		else
		{
			// 读取 Id
			if (state == LyricsAnaylizeState::FindNewId)
			{
				auto cntInput = inputBegin;
				auto cntContext = context;
				try
				{
					idOutputInserter++ = ReadLyricsId(cntContext, cntInput, inputEnd);
					inputBegin = cntInput;
					context = cntContext;
				}
				catch (maybe_time_exception)
				{
					// 结束 Id 读取，开始读取歌词
					state = LyricsAnaylizeState::FindNewLine;
				}
			}
			// 读取歌词
			else if (state == LyricsAnaylizeState::FindNewLine)
			{
				lineOutputInserter++ = ReadLyricsLine(context, inputBegin, inputEnd);
			}
		}
	}
}

NSED_TOMATO_MEDIA