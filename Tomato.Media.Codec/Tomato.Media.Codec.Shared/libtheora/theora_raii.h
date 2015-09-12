//
// Tomato Media Codec
// Theora RAII 辅助
// 
// 作者：SunnyCase
// 创建时间：2015-09-12
#pragma once
#include "common.h"
#include "theoradec.h"

DEFINE_NS_MEDIA_CODEC

struct th_setup_info_deleter
{
	void operator()(th_setup_info* ptr) const noexcept
	{
		th_setup_free(ptr);
	}
};

struct th_comment_raii : public th_comment
{
	th_comment_raii() noexcept
	{
		th_comment_init(this);
	}

	~th_comment_raii()
	{
		th_comment_clear(this);
	}

	void reset()
	{
		th_comment_clear(this);
	}
};

struct th_info_raii : public th_info
{
	th_info_raii() noexcept
	{
		th_info_clear(this);
	}

	~th_info_raii()
	{
		th_info_clear(this);
	}

	void reset()
	{
		th_info_clear(this);
	}
};

struct th_dec_ctx_deleter
{
	void operator()(th_dec_ctx* ptr) const noexcept
	{
		th_decode_free(ptr);
	}
};

END_NS_MEDIA_CODEC