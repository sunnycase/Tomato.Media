#pragma once

namespace Tomato
{
	namespace Media
	{
		///<summary>媒体源</summary>
		public ref class MediaSource sealed
		{
		public:
			static Windows::Foundation::IAsyncOperation<MediaSource^>^ CreateFromFile(Windows::Storage::StorageFile^ file);

			///<summary>标题</summary>
			property Platform::String^ Title {Platform::String^ get(); }
			///<summary>专辑</summary>
			property Platform::String^ Album {Platform::String^ get(); }
			///<summary>专辑艺术家</summary>
			property Platform::String^ AlbumArtist {Platform::String^ get(); }
			///<summary>艺术家</summary>
			property Platform::String^ Artist {Platform::String^ get(); }
		internal:
			IMediaSource* Get();
		private:
			MediaSource(Windows::Storage::Streams::IRandomAccessStream^ stream);
			concurrency::task<void> Initialize();
		private:
			std::unique_ptr<IMediaSource> nativeSource;
		};
	}
}