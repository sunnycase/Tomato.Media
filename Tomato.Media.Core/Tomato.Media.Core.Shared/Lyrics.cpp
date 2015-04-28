//
// Tomato Media
// 歌词（LRC）解析
// 
// (c) SunnyCase 
// 创建日期 2015-04-28
#include "pch.h"
#include "../include/Lyrics.h"
#include <sstream>

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace NS_TOMATO_MEDIA::details::lyrics;

void throwhelper::throw_unexpected_end()
{
	throw std::exception("unexpected end of input.");
}

class unexpected_exception : public std::exception
{
public:
	unexpected_exception(std::string&& str)
		:str(std::move(str))
	{

	}

	virtual char const* what() const
	{
		return str.c_str();
	}
private:
	std::string str;
};

void throwhelper::throw_unexpected(const AnaylizeContext& context, char chr)
{
	std::stringstream ss("unexpected char '");
	ss << chr << "' at line " << context.line << ", position " << context.pos << '.';
	throw unexpected_exception(ss.str());
}