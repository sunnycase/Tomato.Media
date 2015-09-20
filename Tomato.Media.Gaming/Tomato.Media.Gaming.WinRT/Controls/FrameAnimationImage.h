//
// Tomato Media Gaming
// 帧动画 Image
// 
// 作者：SunnyCase 
// 创建日期 2015-09-20
#pragma once
#include "common.h"

DEFINE_NS_MEDIA_GAMING

namespace Controls
{
	///<summary>帧动画 Image</summary>
	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class FrameAnimationImage sealed : public Windows::UI::Xaml::Controls::UserControl
	{
	public:
		static property Windows::UI::Xaml::DependencyProperty^ SourceProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return _sourceProperty; }
		}

		static property Windows::UI::Xaml::DependencyProperty^ CurrentFrameProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return _currentFrameProperty; }
		}

		static property Windows::UI::Xaml::DependencyProperty^ FrameSizeProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return _frameSizeProperty; }
		}

		static property Windows::UI::Xaml::DependencyProperty^ FramesCountProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return _framesCountProperty; }
		}

		static property Windows::UI::Xaml::DependencyProperty^ ColumnsCountProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return _columnsCountProperty; }
		}

		static property Windows::UI::Xaml::DependencyProperty^ AutoPlayProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return _autoPlayProperty; }
		}

		static property Windows::UI::Xaml::DependencyProperty^ IsLoopingProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return _isLoopingProperty; }
		}

		static property Windows::UI::Xaml::DependencyProperty^ CurrentFrameIndexProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return _currentFrameIndexProperty; }
		}

		static property Windows::UI::Xaml::DependencyProperty^ FrameRateProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get() { return _frameRateProperty; }
		}
	public:
		property Microsoft::Graphics::Canvas::CanvasBitmap^ Source
		{
			Microsoft::Graphics::Canvas::CanvasBitmap^ get();
			void set(Microsoft::Graphics::Canvas::CanvasBitmap^ value);
		}

		property Windows::UI::Xaml::Media::ImageSource^ CurrentFrame
		{
			Windows::UI::Xaml::Media::ImageSource^ get();
		private: void set(Windows::UI::Xaml::Media::ImageSource^ value);
		}

		property Windows::Foundation::Size FrameSize
		{
			Windows::Foundation::Size get();
			void set(Windows::Foundation::Size value);
		}

		property int FramesCount
		{
			int get();
			void set(int value);
		}

		property int ColumnsCount
		{
			int get();
			void set(int value);
		}

		property bool AutoPlay
		{
			bool get();
			void set(bool value);
		}

		property bool IsLooping
		{
			bool get();
			void set(bool value);
		}

		property int CurrentFrameIndex
		{
			int get();
			void set(int value);
		}

		property float FrameRate
		{
			float get();
			void set(float value);
		}

		FrameAnimationImage();

		void Play();
		void Stop();

		event Windows::Foundation::EventHandler<Windows::UI::Xaml::RoutedEventArgs^>^ AnimationEnded;
	private:
		property Microsoft::Graphics::Canvas::UI::Xaml::CanvasImageSource^ RenderTarget
		{
			Microsoft::Graphics::Canvas::UI::Xaml::CanvasImageSource^ get();
			void set(Microsoft::Graphics::Canvas::UI::Xaml::CanvasImageSource^ value);
		}

		void InitializeComponents();
		void ResetSource(Microsoft::Graphics::Canvas::CanvasBitmap^ source);
		void OnAnimationTick(Platform::Object ^sender, Platform::Object ^args);
		void OnLoaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
		void OnAnimationEnded();
		void SetFrameRate(float value);

		static void OnSourcePropertyChanged(Windows::UI::Xaml::DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
		static void OnFrameRatePropertyChanged(Windows::UI::Xaml::DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
	private:
		Windows::UI::Xaml::Controls::Image^ _framePresenter;
		Windows::UI::Xaml::DispatcherTimer^ _animationTimer;
		static Windows::UI::Xaml::DependencyProperty^ _sourceProperty;
		static Windows::UI::Xaml::DependencyProperty^ _currentFrameProperty;
		static Windows::UI::Xaml::DependencyProperty^ _frameSizeProperty;
		static Windows::UI::Xaml::DependencyProperty^ _framesCountProperty;
		static Windows::UI::Xaml::DependencyProperty^ _columnsCountProperty;
		static Windows::UI::Xaml::DependencyProperty^ _autoPlayProperty;
		static Windows::UI::Xaml::DependencyProperty^ _isLoopingProperty;
		static Windows::UI::Xaml::DependencyProperty^ _currentFrameIndexProperty;
		static Windows::UI::Xaml::DependencyProperty^ _frameRateProperty;

		bool _isLoaded = false;
		bool _hasResource = false;
	};
}

END_NS_MEDIA_GAMING