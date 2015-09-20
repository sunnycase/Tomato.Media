//
// Tomato Media Gaming
// 帧动画 Image
// 
// 作者：SunnyCase 
// 创建日期 2015-09-20
#include "pch.h"
#include "FrameAnimationImage.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_GAMING;
using namespace NS_MEDIA_GAMING::Controls;
using namespace WRL;
using namespace default;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Controls;
using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI::Xaml;

namespace
{
	void CheckFrameSize(Size size)
	{
		if (size.Width < 1.f || size.Height < 1.f)
			throw ref new InvalidArgumentException(L"FrameSize is not valid.");
	}

	void CheckFramesCount(int count)
	{
		if (count <= 0)
			throw ref new InvalidArgumentException(L"FramesCount is not valid.");
	}
}

DependencyProperty^ FrameAnimationImage::_sourceProperty = DependencyProperty::Register(L"Source", CanvasBitmap::typeid,
	FrameAnimationImage::typeid, ref new PropertyMetadata(DependencyProperty::UnsetValue, ref new PropertyChangedCallback(FrameAnimationImage::OnSourcePropertyChanged)));

DependencyProperty^ FrameAnimationImage::_currentFrameProperty = DependencyProperty::Register(L"CurrentFrame", ImageSource::typeid,
		FrameAnimationImage::typeid, ref new PropertyMetadata(DependencyProperty::UnsetValue));

DependencyProperty^ FrameAnimationImage::_frameSizeProperty = DependencyProperty::Register(L"FrameSize", Size::typeid,
		FrameAnimationImage::typeid, ref new PropertyMetadata(Size()));

DependencyProperty^ FrameAnimationImage::_framesCountProperty = DependencyProperty::Register(L"FramesCount", int::typeid,
		FrameAnimationImage::typeid, ref new PropertyMetadata((Object^)(int())));

DependencyProperty^ FrameAnimationImage::_columnsCountProperty = DependencyProperty::Register(L"ColumnsCount", int::typeid,
		FrameAnimationImage::typeid, ref new PropertyMetadata((Object^)(int())));

DependencyProperty^ FrameAnimationImage::_autoPlayProperty = DependencyProperty::Register(L"AutoPlay", bool::typeid,
	FrameAnimationImage::typeid, ref new PropertyMetadata((Object^)(true)));

DependencyProperty^ FrameAnimationImage::_isLoopingProperty = DependencyProperty::Register(L"IsLooping", bool::typeid,
	FrameAnimationImage::typeid, ref new PropertyMetadata((Object^)(false)));

DependencyProperty^ FrameAnimationImage::_currentFrameIndexProperty = DependencyProperty::Register(L"CurrentFrameIndex", int::typeid,
	FrameAnimationImage::typeid, ref new PropertyMetadata((Object^)(int())));

DependencyProperty^ FrameAnimationImage::_frameRateProperty = DependencyProperty::Register(L"FrameRate", float::typeid,
	FrameAnimationImage::typeid, ref new PropertyMetadata((Object^)(25.f), ref new PropertyChangedCallback(FrameAnimationImage::OnFrameRatePropertyChanged)));

CanvasBitmap^ FrameAnimationImage::Source::get()
{
	return safe_cast<CanvasBitmap^>(GetValue(SourceProperty));
}

void FrameAnimationImage::Source::set(CanvasBitmap^ value)
{
	SetValue(SourceProperty, value);
}

ImageSource^ FrameAnimationImage::CurrentFrame::get()
{
	return safe_cast<ImageSource^>(GetValue(CurrentFrameProperty));
}

void FrameAnimationImage::CurrentFrame::set(ImageSource^ value)
{
	SetValue(CurrentFrameProperty, value);
}

Size FrameAnimationImage::FrameSize::get()
{
	return safe_cast<Size>(GetValue(FrameSizeProperty));
}

void FrameAnimationImage::FrameSize::set(Size value)
{
	SetValue(FrameSizeProperty, value);
}

int FrameAnimationImage::FramesCount::get()
{
	return safe_cast<int>(GetValue(FramesCountProperty));
}

void FrameAnimationImage::FramesCount::set(int value)
{
	SetValue(FramesCountProperty, value);
}

int FrameAnimationImage::ColumnsCount::get()
{
	return safe_cast<int>(GetValue(ColumnsCountProperty));
}

void FrameAnimationImage::ColumnsCount::set(int value)
{
	SetValue(ColumnsCountProperty, value);
}

bool FrameAnimationImage::AutoPlay::get()
{
	return safe_cast<bool>(GetValue(AutoPlayProperty));
}

void FrameAnimationImage::AutoPlay::set(bool value)
{
	SetValue(AutoPlayProperty, value);
}

bool FrameAnimationImage::IsLooping::get()
{
	return safe_cast<bool>(GetValue(IsLoopingProperty));
}

void FrameAnimationImage::IsLooping::set(bool value)
{
	SetValue(IsLoopingProperty, value);
}

int FrameAnimationImage::CurrentFrameIndex::get()
{
	return safe_cast<int>(GetValue(CurrentFrameIndexProperty));
}

void FrameAnimationImage::CurrentFrameIndex::set(int value)
{
	SetValue(CurrentFrameIndexProperty, value);
}

float FrameAnimationImage::FrameRate::get()
{
	return safe_cast<float>(GetValue(FrameRateProperty));
}

void FrameAnimationImage::FrameRate::set(float value)
{
	SetValue(FrameRateProperty, value);
}

FrameAnimationImage::FrameAnimationImage()
{
	InitializeComponents();
	Loaded += ref new Windows::UI::Xaml::RoutedEventHandler(this, &Controls::FrameAnimationImage::OnLoaded);
}

void FrameAnimationImage::Play()
{
	if (_hasResource)
		_animationTimer->Start();
}

void FrameAnimationImage::Stop()
{
	_animationTimer->Stop();
	CurrentFrameIndex = 0;
}

CanvasImageSource^ FrameAnimationImage::RenderTarget::get()
{
	return safe_cast<CanvasImageSource^>(CurrentFrame);
}

void FrameAnimationImage::RenderTarget::set(CanvasImageSource^ value)
{
	CurrentFrame = value;
}

void FrameAnimationImage::InitializeComponents()
{
	_framePresenter = ref new Image();
	{
		auto sourceBinding = ref new Binding();
		sourceBinding->Path = ref new PropertyPath(L"CurrentFrame");
		sourceBinding->Source = this;
		_framePresenter->SetBinding(Image::SourceProperty, sourceBinding);
	}
	
	this->Content = _framePresenter;
	_animationTimer = ref new DispatcherTimer();
	SetFrameRate(FrameRate);
	_animationTimer->Tick += ref new EventHandler<Object ^>(this, &FrameAnimationImage::OnAnimationTick);
}

void FrameAnimationImage::ResetSource(CanvasBitmap^ source)
{
	if (!_isLoaded) return;
	_animationTimer->Stop();
	RenderTarget = nullptr;
	_hasResource = false;

	if (source)
	{
		auto frameSize = FrameSize;
		CheckFrameSize(frameSize);
		auto framesCount = FramesCount;
		CheckFramesCount(framesCount);

		auto device = CanvasDevice::GetSharedDevice();
		// 创建 renderTarget
		auto renderTarget = ref new CanvasImageSource(device, frameSize.Width, frameSize.Height,
			source->Dpi, CanvasAlphaMode::Premultiplied);
		RenderTarget = renderTarget;
		_hasResource = true;

		if (AutoPlay)
			_animationTimer->Start();
	}
}

void FrameAnimationImage::OnSourcePropertyChanged(DependencyObject ^ sender, DependencyPropertyChangedEventArgs ^ e)
{
	if (auto image = dynamic_cast<FrameAnimationImage^>(sender))
		image->ResetSource(safe_cast<CanvasBitmap^>(e->NewValue));
}

void FrameAnimationImage::OnFrameRatePropertyChanged(DependencyObject ^ sender, DependencyPropertyChangedEventArgs ^ e)
{
	if (auto image = dynamic_cast<FrameAnimationImage^>(sender))
		image->SetFrameRate((float)e->NewValue);
}

void FrameAnimationImage::OnAnimationTick(Object ^sender, Object ^args)
{
	auto columnsCount = size_t(ColumnsCount);
	const auto frameSize = FrameSize;
	// 如果为 0 则从 source 读取
	const auto maxColumns = columnsCount == 0 ? size_t(Source->Size.Width / frameSize.Width) : columnsCount;
	if (maxColumns == 0)
		throw ref new InvalidArgumentException("FrameSize or Source or ColumnsCount is invalid.");

	auto cntFrameIdx = std::max(0u, size_t(CurrentFrameIndex));
	if (cntFrameIdx + 1 > size_t(FramesCount))
	{
		CurrentFrameIndex = cntFrameIdx = 0;
		if (!IsLooping)
		{
			_animationTimer->Stop();
			OnAnimationEnded();
			return;
		}
	}
	else
		CurrentFrameIndex = cntFrameIdx + 1;
	if (Visibility == Xaml::Visibility::Visible)
	{
		const auto cntRowIdx = cntFrameIdx / maxColumns;
		const auto cntColumnIdx = cntFrameIdx % maxColumns;
		Point cntLeftTop(cntColumnIdx * frameSize.Width, cntRowIdx * frameSize.Height);
		Rect src(cntLeftTop, frameSize);

		auto session = RenderTarget->CreateDrawingSession(Colors::Transparent);
		session->DrawImage(Source, Rect(Point(), frameSize), src);
	}
}

void FrameAnimationImage::OnLoaded(Object ^sender, RoutedEventArgs ^e)
{
	_isLoaded = true;
	ResetSource(Source);
}

void FrameAnimationImage::OnAnimationEnded()
{
	AnimationEnded(this, ref new RoutedEventArgs());
}

void FrameAnimationImage::SetFrameRate(float value)
{
	_animationTimer->Interval = TimeSpan{ static_cast<long long>(1e7 / value) };
}
