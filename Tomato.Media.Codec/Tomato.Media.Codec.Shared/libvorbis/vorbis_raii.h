//
// Tomato Media Codec
// Vorbis RAII 辅助
// 
// 作者：SunnyCase
// 创建时间：2015-09-14
#pragma once
#include "common.h"
#include "codec.h"

DEFINE_NS_MEDIA_CODEC

struct vorbis_info_raii : public vorbis_info
{
	vorbis_info_raii() noexcept
	{
		vorbis_info_init(this);
	}

	~vorbis_info_raii()
	{
		vorbis_info_clear(this);
	}

	void reset()
	{
		vorbis_info_clear(this);
	}
};

struct vorbis_comment_raii : public vorbis_comment
{
	vorbis_comment_raii() noexcept
	{
		vorbis_comment_init(this);
	}

	~vorbis_comment_raii()
	{
		vorbis_comment_clear(this);
	}

	void reset()
	{
		vorbis_comment_clear(this);
	}
};

struct vorbis_dsp_state_raii : public vorbis_dsp_state
{
	vorbis_dsp_state_raii() noexcept
	{
		memset(this, 0, sizeof(vorbis_dsp_state_raii));
	}

	~vorbis_dsp_state_raii()
	{
		vorbis_dsp_clear(this);
	}

	void reset()
	{
		vorbis_dsp_clear(this);
	}

	void assign(vorbis_info& info)
	{
		reset();
		ThrowIfNot(vorbis_synthesis_init(this, &info) == 0, L"Cannot init vorbis dsp state.");
	}
};

struct vorbis_block_raii : public vorbis_block
{
	vorbis_block_raii() noexcept
	{
		memset(this, 0, sizeof(vorbis_block_raii));
	}

	~vorbis_block_raii()
	{
		vorbis_block_clear(this);
	}

	void reset()
	{
		vorbis_block_clear(this);
	}

	void assign(vorbis_dsp_state_raii& state)
	{
		reset();
		ThrowIfNot(vorbis_block_init(&state, this) == 0, L"Cannot init vorbis block.");
	}
};

END_NS_MEDIA_CODEC