//
// Tomato Media
// 分块缓冲
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "../include/tomato.media.core.h"
#include <list>

namespace Tomato {
	// 分块缓冲
	template<class elem_t>
	class block_buffer
	{
	private:
		struct tagBlock
		{
			std::unique_ptr<elem_t[]> data;

			tagBlock(size_t capacity)
				:data(std::make_unique<elem_t[]>(capacity))
			{
			}

			tagBlock(const tagBlock&) = delete;

			tagBlock(tagBlock&& block)
				:data(std::move(block.data))
			{

			}

			elem_t* get()
			{
				return data.get();
			}
		};
	public:
		enum
		{
			elem_size = sizeof(elem_t)
		};

		block_buffer()
		{
		}

		void init(size_t block_size)
		{
			blocks.clear();
			this->block_size = block_size;
			total_size = 0;
			read_pos = read_offset = write_pos = 0;
			make_last_block_not_full();
			read_it = blocks.begin();
		}

		size_t blocks_count() const noexcept
		{
			return blocks.size();
		}

		size_t count() const noexcept
		{
			return total_size;
		}

		bool empty() const noexcept
		{
			return total_size == 0;
		}

		size_t tell_not_get() const noexcept
		{
			return total_size - (size_t)read_pos;
		}

		void write(const elem_t* data, size_t count)
		{
			while (count)
			{
				auto& block = make_last_block_not_full();
				auto filled = get_last_block_filled();
				auto to_write = std::min(count, block_size - filled);

				write(block.get() + filled, to_write, data, to_write);
				data += to_write;
				count -= to_write;
			}
		}

		size_t read(elem_t* buffer, size_t count)
		{
			count = std::min(count, total_size - (size_t)read_pos);
			size_t has_read = 0;

			while (count)
			{
				auto to_read = std::min(block_size - read_offset, count);
				read(buffer, to_read, read_it->get(), to_read);

				buffer += to_read;
				count -= to_read;
				has_read += to_read;
				read_offset += to_read;
				validate_read_offset();
			}
			return has_read;
		}

		void clear()
		{
			init(block_size);
		}
	private:
		tagBlock& acquire_block()
		{
			blocks.emplace_back(tagBlock(block_size));
			return blocks.back();
		}

		void write(elem_t* dest, size_t dest_size, const elem_t* src, size_t max_count)
		{
			memcpy_s(dest, dest_size, src, max_count);
			write_pos += max_count;
			total_size += max_count;
		}

		void read(elem_t* dest, size_t dest_size, const elem_t* src, size_t max_count)
		{
			memcpy_s(dest, dest_size, src, max_count);
			read_pos += max_count;
		}

		tagBlock& make_last_block_not_full()
		{
			if (block_size * blocks.size() > total_size)
				return blocks.back();
			return acquire_block();
		}

		// 最后一个分块的已用空间
		size_t get_last_block_filled() const
		{
			return total_size % block_size;
		}

		void validate_read_offset()
		{
			while (read_offset >= block_size)
			{
				read_it++;
				read_offset -= block_size;
			}
		}
	private:
		typedef std::list<tagBlock> blocks_t;

		size_t block_size;		// 分块大小
		blocks_t blocks;				// 分块数据
		size_t total_size = 0;			// 数据总大小

		int64_t read_pos = 0;		// 读指针
		int64_t write_pos = 0;		// 写指针

		typename blocks_t::iterator read_it;
		size_t read_offset = 0;
	};
}