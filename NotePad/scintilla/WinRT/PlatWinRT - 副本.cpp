//
//          Copyright (c) 1990-2011, Scientific Toolworks, Inc.
//
// The License.txt file describes the conditions under which this software may be distributed.
//
// Author: Jason Haslam
//
// Additions Copyright (c) 2011 Archaeopteryx Software, Inc. d/b/a Wingware
// Scintilla platform layer for Qt

#include "pch.h"
#include <cstdio>
#include<algorithm>
#include  <mutex>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include "PlatWinRT.h"
#include "Scintilla.h"
#include "UniConversion.h"
#include "DBCS.h"
#include "FontQuality.h"
#include "winrt/Microsoft.Graphics.Canvas.Text.h"
#include "winrt/Microsoft.Graphics.Canvas.UI.Xaml.h"
#include <winrt/windows.graphics.display.h>
#include "winrt/Microsoft.Graphics.Canvas.Brushes.h"
#include "winrt/Microsoft.Graphics.Canvas.Geometry.h"
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Controls.h>

using namespace winrt;
using namespace Microsoft::Graphics::Canvas::Brushes;
using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Windows::UI::Text;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::Graphics::Display;
using namespace Microsoft::Graphics::Canvas::UI::Xaml;
using namespace Microsoft::Graphics::Canvas::Geometry;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Foundation;
using namespace Windows::UI::Popups;

namespace Scintilla {

	UINT CodePageFromCharSet(DWORD characterSet, UINT documentCodePage) {
		if (documentCodePage == SC_CP_UTF8) {
			return SC_CP_UTF8;
		}
		switch (characterSet) {
		case SC_CHARSET_ANSI: return 1252;
		case SC_CHARSET_DEFAULT: return documentCodePage ? documentCodePage : 1252;
		case SC_CHARSET_BALTIC: return 1257;
		case SC_CHARSET_CHINESEBIG5: return 950;
		case SC_CHARSET_EASTEUROPE: return 1250;
		case SC_CHARSET_GB2312: return 936;
		case SC_CHARSET_GREEK: return 1253;
		case SC_CHARSET_HANGUL: return 949;
		case SC_CHARSET_MAC: return 10000;
		case SC_CHARSET_OEM: return 437;
		case SC_CHARSET_RUSSIAN: return 1251;
		case SC_CHARSET_SHIFTJIS: return 932;
		case SC_CHARSET_TURKISH: return 1254;
		case SC_CHARSET_JOHAB: return 1361;
		case SC_CHARSET_HEBREW: return 1255;
		case SC_CHARSET_ARABIC: return 1256;
		case SC_CHARSET_VIETNAMESE: return 1258;
		case SC_CHARSET_THAI: return 874;
		case SC_CHARSET_8859_15: return 28605;
			// Not supported
		case SC_CHARSET_CYRILLIC: return documentCodePage;
		case SC_CHARSET_SYMBOL: return documentCodePage;
		}
		return documentCodePage;
	}


	struct FormatAndMetrics {
		int technology;
		CanvasTextFormat pTextFormat{nullptr};
		int extraFontFlag;
		int characterSet;
		FLOAT yAscent;
		FLOAT yDescent;
		FLOAT yInternalLeading;
		FormatAndMetrics(CanvasTextFormat hfont_, int extraFontFlag_, int characterSet_, float ya, float yd) :
			technology(SCWIN_TECH_GDI), pTextFormat(hfont_),
			extraFontFlag(extraFontFlag_), characterSet(characterSet_), yAscent(ya), yDescent(yd), yInternalLeading(0) {
		}
		FormatAndMetrics(const FormatAndMetrics &) = delete;
		FormatAndMetrics(FormatAndMetrics &&) = delete;
		FormatAndMetrics &operator=(const FormatAndMetrics &) = delete;
		FormatAndMetrics &operator=(FormatAndMetrics &&) = delete;

		~FormatAndMetrics() {
			pTextFormat = nullptr;
			extraFontFlag = 0;
			characterSet = 0;
			yAscent = 2;
			yDescent = 1;
			yInternalLeading = 0;
		}

	};



#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY 5
#endif

	namespace {
		std::recursive_mutex mutex;
		//CRITICAL_SECTION crPlatformLock;
		//HINSTANCE hinstPlatformRes = 0;

		//HCURSOR reverseArrowCursor = NULL;
		inline int mul_div(int number, int numerator, int denominator) {
			long long ret = number;
			ret *= numerator;
			ret /= denominator;
			return (int)ret;
		}
		FormatAndMetrics *FamFromFontID(void *fid) {
			return static_cast<FormatAndMetrics *>(fid);
		}

		BYTE Win32MapFontQuality(int extraFontFlag) {
			switch (extraFontFlag & SC_EFF_QUALITY_MASK) {

			case SC_EFF_QUALITY_NON_ANTIALIASED:
				return NONANTIALIASED_QUALITY;

			case SC_EFF_QUALITY_ANTIALIASED:
				return ANTIALIASED_QUALITY;

			case SC_EFF_QUALITY_LCD_OPTIMIZED:
				return CLEARTYPE_QUALITY;

			default:
				return SC_EFF_QUALITY_DEFAULT;
			}
		}

		//void SetLogFont(LOGFONTW &lf, const char *faceName, int characterSet, float size, int weight, bool italic, int extraFontFlag) {
		//	lf = LOGFONTW();
		//	// The negative is to allow for leading
		//	lf.lfHeight = -(abs(lround(size)));
		//	lf.lfWeight = weight;
		//	lf.lfItalic = italic ? 1 : 0;
		//	lf.lfCharSet = static_cast<BYTE>(characterSet);
		//	lf.lfQuality = Win32MapFontQuality(extraFontFlag);
		//	UTF16FromUTF8(faceName, lf.lfFaceName, LF_FACESIZE);
		//}

		/**
		 * Create a hash from the parameters for a font to allow easy checking for identity.
		 * If one font is the same as another, its hash will be the same, but if the hash is the
		 * same then they may still be different.
		 */
		int HashFont(const FontParameters &fp) noexcept {
			return
				static_cast<int>(fp.size) ^
				(fp.characterSet << 10) ^
				((fp.extraFontFlag & SC_EFF_QUALITY_MASK) << 9) ^
				((fp.weight / 100) << 12) ^
				(fp.italic ? 0x20000000 : 0) ^
				(fp.technology << 15) ^
				fp.faceName[0];
		}

	}

	class FontCached : Font {
		FontCached *next;
		int usage;
		float size;
		//LOGFONTW lf;
		CanvasTextFormat pTextFormat{nullptr};
		int technology;
		int hash;
		explicit FontCached(const FontParameters &fp);
		bool SameAs(const FontParameters &fp);
		void Release() override;

		static FontCached *first;
	public:
		~FontCached() override {}
		static FontID FindOrCreate(const FontParameters &fp);
		static void ReleaseId(FontID fid_);
	};

	FontCached *FontCached::first = nullptr;

	FontCached::FontCached(const FontParameters &fp) :
		next(0), usage(0), size(1.0), hash(0) {
		technology = fp.technology;
		hash = HashFont(fp);
		fid = 0;

		pTextFormat = CanvasTextFormat();
		const int faceSize = 200;
		wchar_t wszFace[faceSize] = L"";
		UTF16FromUTF8(fp.faceName, wszFace, faceSize);
		pTextFormat.FontFamily(hstring(wszFace));
		pTextFormat.FontSize(fp.size);
		pTextFormat.FontStyle(fp.italic ? FontStyle::Italic : FontStyle::Normal);
		pTextFormat.FontStretch(FontStretch::Normal);
		FontWeight fw;
		fw.Weight = fp.weight;
		pTextFormat.FontWeight(fw);
		float ya = 2, yd = 1;
		fid = new FormatAndMetrics(pTextFormat, fp.extraFontFlag, fp.characterSet, ya, yd);
		usage = 1;
	}

	bool FontCached::SameAs(const FontParameters &fp) {
		if (
			(size == fp.size) &&
			(pTextFormat.FontWeight().Weight == fp.weight) &&
			(pTextFormat.FontStyle() == (fp.italic ? FontStyle::Italic : FontStyle::Normal)) &&
			/*(lf.lfCharSet == fp.characterSet) &&*/
			/*(lf.lfQuality == Win32MapFontQuality(fp.extraFontFlag)) &&*/
			(technology == fp.technology)) {
			const int faceSize = 200;
			wchar_t wszFace[faceSize] = L"";
			UTF16FromUTF8(fp.faceName, wszFace, faceSize);
			return 0 == wcscmp(pTextFormat.FontFamily().c_str(), wszFace);
		}
		return false;
	}

	void FontCached::Release() {
		delete FamFromFontID(fid);
		fid = nullptr;
	}

	FontID FontCached::FindOrCreate(const FontParameters &fp) {
		FontID ret = 0;
		mutex.lock();
		const int hashFind = HashFont(fp);
		for (FontCached *cur = first; cur; cur = cur->next) {
			if ((cur->hash == hashFind) &&
				cur->SameAs(fp)) {
				cur->usage++;
				ret = cur->fid;
			}
		}
		if (!ret) {
			FontCached *fc = new FontCached(fp);
			fc->next = first;
			first = fc;
			ret = fc->fid;
		}
		mutex.unlock();
		return ret;
	}

	void FontCached::ReleaseId(FontID fid_) {
		mutex.lock();
		FontCached **pcur = &first;
		for (FontCached *cur = first; cur; cur = cur->next) {
			if (cur->fid == fid_) {
				cur->usage--;
				if (cur->usage == 0) {
					*pcur = cur->next;
					cur->Release();
					cur->next = nullptr;
					delete cur;
				}
				break;
			}
			pcur = &cur->next;
		}
		mutex.unlock();
	}

	Font::Font() noexcept : fid(0) {
	}

	Font::~Font() {
	}

#define FONTS_CACHED

	void Font::Create(const FontParameters &fp) {
		Release();
		if (fp.faceName)
			fid = FontCached::FindOrCreate(fp);
	}

	void Font::Release() {
		if (fid)
			FontCached::ReleaseId(fid);
		fid = 0;
	}

	// Buffer to hold strings and string position arrays without always allocating on heap.
	// May sometimes have string too long to allocate on stack. So use a fixed stack-allocated buffer
	// when less than safe size otherwise allocate on heap and free automatically.
	template<typename T, int lengthStandard>
	class VarBuffer {
		T bufferStandard[lengthStandard];
	public:
		T *buffer;
		explicit VarBuffer(size_t length) : buffer(nullptr) {
			if (length > lengthStandard) {
				buffer = new T[length];
			}
			else {
				buffer = bufferStandard;
			}
		}
		// Deleted so VarBuffer objects can not be copied.
		VarBuffer(const VarBuffer &) = delete;
		VarBuffer(VarBuffer &&) = delete;
		VarBuffer &operator=(const VarBuffer &) = delete;
		VarBuffer &operator=(VarBuffer &&) = delete;

		~VarBuffer() {
			if (buffer != bufferStandard) {
				delete[]buffer;
				buffer = nullptr;
			}
		}
	};

	const int stackBufferLength = 1000;
	class TextWide : public VarBuffer<wchar_t, stackBufferLength> {
	public:
		int tlen;	// Using int instead of size_t as most Win32 APIs take int.
		TextWide(std::string_view text, bool unicodeMode, int codePage = 0) :
			VarBuffer<wchar_t, stackBufferLength>(text.length()) {
			if (unicodeMode) {
				tlen = static_cast<int>(UTF16FromUTF8(text, buffer, text.length()));
			}
			else {
				// Support Asian string display in 9x English
				//std::wstring_convert< std::codecvt_utf8<wchar_t> > strCnv;
				//auto str= strCnv.from_bytes(text.data());
				//wcscpy(buffer, str.c_str());
				tlen = ::MultiByteToWideChar(codePage, 0, text.data(), static_cast<int>(text.length()),
					buffer, static_cast<int>(text.length()));
			}
		}
	};
	typedef VarBuffer<XYPOSITION, stackBufferLength> TextPositions;


	class BlobInline;

	class SurfaceD2D : public Surface {
		bool unicodeMode;
		int x, y;

		int codePage;
		int codePageText;

		CanvasDrawingSession pRenderTarget{ nullptr };
		CanvasRenderTarget pCompatibleRenderTarget{ nullptr };
		bool ownRenderTarget;
		//int clipsActive;
		std::vector<CanvasActiveLayer> clipsActive;

		CanvasTextFormat pTextFormat{ nullptr };
		FLOAT yAscent;
		FLOAT yDescent;
		FLOAT yInternalLeading;

		CanvasSolidColorBrush pBrush{ nullptr };

		int logPixelsY;
		float dpiScaleX;
		float dpiScaleY;

		void Clear();
		void SetFont(Font &font_);

	public:
		SurfaceD2D();
		// Deleted so SurfaceD2D objects can not be copied.
		SurfaceD2D(const SurfaceD2D &) = delete;
		SurfaceD2D(SurfaceD2D &&) = delete;
		SurfaceD2D &operator=(const SurfaceD2D &) = delete;
		SurfaceD2D &operator=(SurfaceD2D &&) = delete;
		virtual ~SurfaceD2D() override;

		void SetScale();
		void Init(WindowID wid) override;
		void Init(SurfaceID sid, WindowID wid) override;
		void InitPixMap(int width, int height, Surface *surface_, WindowID wid) override;

		void Release() override;
		bool Initialised() override;

		HRESULT FlushDrawing();

		void PenColour(ColourDesired fore) override;
		void D2DPenColour(ColourDesired fore, int alpha = 255);
		int LogPixelsY() override;
		int DeviceHeightFont(int points) override;
		void MoveTo(int x_, int y_) override;
		void LineTo(int x_, int y_) override;
		void Polygon(Point *pts, size_t npts, ColourDesired fore, ColourDesired back) override;
		void RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) override;
		void FillRectangle(PRectangle rc, ColourDesired back) override;
		void FillRectangle(PRectangle rc, Surface &surfacePattern) override;
		void RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) override;
		void AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
			ColourDesired outline, int alphaOutline, int flags) override;
		void GradientRectangle(PRectangle rc, const std::vector<ColourStop> &stops, GradientOptions options) override;
		void DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage) override;
		void Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) override;
		void Copy(PRectangle rc, Point from, Surface &surfaceSource) override;

		std::unique_ptr<IScreenLineLayout> Layout(const IScreenLine *screenLine) override;

		void DrawTextCommon(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text, UINT fuOptions);
		void DrawTextNoClip(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore, ColourDesired back) override;
		void DrawTextClipped(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore, ColourDesired back) override;
		void DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore) override;
		void MeasureWidths(Font &font_, std::string_view text, XYPOSITION *positions) override;
		XYPOSITION WidthText(Font &font_, std::string_view text) override;
		XYPOSITION Ascent(Font &font_) override;
		XYPOSITION Descent(Font &font_) override;
		XYPOSITION InternalLeading(Font &font_) override;
		XYPOSITION Height(Font &font_) override;
		XYPOSITION AverageCharWidth(Font &font_) override;

		void SetClip(PRectangle rc) override;
		void FlushCachedState() override;

		void SetUnicodeMode(bool unicodeMode_) override;
		void SetDBCSMode(int codePage_) override;
		void SetBidiR2L(bool bidiR2L_) override;
	};

	SurfaceD2D::SurfaceD2D() :
		unicodeMode(false),
		x(0), y(0) {

		codePage = 0;
		codePageText = 0;

		ownRenderTarget = false;


		// From selected font
		yAscent = 2;
		yDescent = 1;
		yInternalLeading = 0;

		logPixelsY = 72;
		dpiScaleX = 1.0;
		dpiScaleY = 1.0;
	}

	SurfaceD2D::~SurfaceD2D() {
		Clear();
	}

	void SurfaceD2D::Clear() {

	}

	void SurfaceD2D::Release() {
		Clear();
	}

	void SurfaceD2D::SetScale() {
		auto var = DisplayInformation::GetForCurrentView();
		logPixelsY = var.RawDpiY();
		dpiScaleX = var.RawDpiX() / 96.0;
		dpiScaleY = logPixelsY / 96.0;
	}

	bool SurfaceD2D::Initialised() {
		return pRenderTarget != 0;
	}

	HRESULT SurfaceD2D::FlushDrawing() {
		pRenderTarget.Flush();
		return 1;
	}

	void SurfaceD2D::Init(WindowID  wid) {
		Release();
		SetScale();
		CanvasControl  c{ nullptr };
		winrt::attach_abi(c, wid);
		pCompatibleRenderTarget = CanvasRenderTarget(c, static_cast<float>(c.ActualWidth()), static_cast<float>(c.ActualHeight()));
		winrt::detach_abi(c);
		pRenderTarget = pCompatibleRenderTarget.CreateDrawingSession();
	}

	void SurfaceD2D::Init(SurfaceID sid, WindowID) {
		Release();
		SetScale();
		winrt::copy_from_abi(pRenderTarget, sid);
	}

	void SurfaceD2D::InitPixMap(int width, int height, Surface *surface_, WindowID) {
		Release();
		SetScale();
		SurfaceD2D *psurfOther = static_cast<SurfaceD2D *>(surface_);

		pCompatibleRenderTarget = CanvasRenderTarget(psurfOther->pRenderTarget, static_cast<float>(width), static_cast<float>(height));

		pRenderTarget = pCompatibleRenderTarget.CreateDrawingSession();
		ownRenderTarget = true;
		SetUnicodeMode(psurfOther->unicodeMode);
		SetDBCSMode(psurfOther->codePage);
	}

	void SurfaceD2D::PenColour(ColourDesired fore) {
		D2DPenColour(fore);
	}

	void SurfaceD2D::D2DPenColour(ColourDesired fore, int alpha) {
		if (pRenderTarget) {
			winrt::Windows::UI::Color col;
			col.R = (fore.AsInteger() & 0xff); /// 255.0f;
			col.G = ((fore.AsInteger() & 0xff00) >> 8);// / 255.0f;
			col.B = (fore.AsInteger() >> 16); /// 255.0f;
			col.A = alpha; // 255.0f;
			if (pBrush) {
				pBrush.Color(col);
			}
			else {
				pBrush = CanvasSolidColorBrush(pRenderTarget, col);
			}
		}
	}

	static CanvasTextAntialiasing DWriteMapFontQuality(int extraFontFlag) {
		switch (extraFontFlag & SC_EFF_QUALITY_MASK) {

		case SC_EFF_QUALITY_NON_ANTIALIASED:
			return CanvasTextAntialiasing::Aliased;

		case SC_EFF_QUALITY_ANTIALIASED:
			return  CanvasTextAntialiasing::Grayscale;

		case SC_EFF_QUALITY_LCD_OPTIMIZED:
			return CanvasTextAntialiasing::ClearType;
		default:
			return  CanvasTextAntialiasing::Auto;
		}
	}

	void SurfaceD2D::SetFont(Font &font_) {
		FormatAndMetrics *pfm = FamFromFontID(font_.GetID());
		PLATFORM_ASSERT(pfm->technology == SCWIN_TECH_DIRECTWRITE);
		pTextFormat = pfm->pTextFormat;
		yAscent = pfm->yAscent;
		yDescent = pfm->yDescent;
		yInternalLeading = pfm->yInternalLeading;
		codePageText = codePage;
		if (pfm->characterSet) {
			codePageText = Scintilla::CodePageFromCharSet(pfm->characterSet, codePage);
		}
		if (pRenderTarget) {
			CanvasTextLayout textLayout = CanvasTextLayout(pRenderTarget, L"X", pTextFormat, 100.0f, 100.0f);
			auto LineMetric = textLayout.LineMetrics().at(0);
			pfm->yAscent = yAscent = LineMetric.Baseline;
			pfm->yDescent = yDescent = LineMetric.Height - yAscent;
			CanvasTextAntialiasing  aaMode = DWriteMapFontQuality(pfm->extraFontFlag);
			pRenderTarget.TextAntialiasing(aaMode);
		}
	}

	int SurfaceD2D::LogPixelsY() {
		return logPixelsY;
	}

	int SurfaceD2D::DeviceHeightFont(int points) {
		return mul_div(points, LogPixelsY(), 72);
	}

	void SurfaceD2D::MoveTo(int x_, int y_) {
		x = x_;
		y = y_;
	}

	static int Delta(int difference) {
		if (difference < 0)
			return -1;
		else if (difference > 0)
			return 1;
		else
			return 0;
	}

	static float RoundFloat(float f) {
		return float(int(f + 0.5f));
	}

	void SurfaceD2D::LineTo(int x_, int y_) {
		if (pRenderTarget) {
			const int xDiff = x_ - x;
			const int xDelta = Delta(xDiff);
			const int yDiff = y_ - y;
			const int yDelta = Delta(yDiff);
			if ((xDiff == 0) || (yDiff == 0)) {
				// Horizontal or vertical lines can be more precisely drawn as a filled rectangle
				const int xEnd = x_ - xDelta;
				const int left = std::min(x, xEnd);
				const int width = abs(x - xEnd) + 1;
				const int yEnd = y_ - yDelta;
				const int top = std::min(y, yEnd);
				const int height = abs(y - yEnd) + 1;
				Rect rectangle1 = Rect(static_cast<float>(left), static_cast<float>(top),
					static_cast<float>(left + width), static_cast<float>(top + height));
				pRenderTarget.FillRectangle(rectangle1, pBrush);
			}
			else if ((abs(xDiff) == abs(yDiff))) {
				// 45 degree slope
				pRenderTarget.DrawLine(x + 0.5f, y + 0.5f, x_ + 0.5f - xDelta, y_ + 0.5f - yDelta, pBrush);
			}
			else {
				// Line has a different slope so difficult to avoid last pixel
				pRenderTarget.DrawLine(x + 0.5f, y + 0.5f, x_ + 0.5f, y_ + 0.5f, pBrush);
			}
			x = x_;
			y = y_;
		}
	}

	void SurfaceD2D::Polygon(Point *pts, size_t npts, ColourDesired fore, ColourDesired back) {
		if (pRenderTarget) {
			float2 *pts = new float2[npts];
			for (size_t i = 0; i < static_cast<size_t>(npts); i++) {
				pts[i] = float2(pts[i].x + 0.5f, pts[i].y + 0.5f);
			}
			array_view<const float2> points = array_view<const float2>(pts, pts + npts);
			CanvasGeometry geometry = CanvasGeometry::CreatePolygon(pRenderTarget, points);

			D2DPenColour(back);
			pRenderTarget.FillGeometry(geometry, pBrush);
			D2DPenColour(fore);
			pRenderTarget.DrawGeometry(geometry, pBrush);
		}
	}

	void SurfaceD2D::RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) {
		if (pRenderTarget) {
			Rect rectangle1 = Rect(RoundFloat(rc.left) + 0.5f, rc.top + 0.5f, RoundFloat(rc.Width()) - 0.5f, rc.Height() - 0.5f);
			D2DPenColour(back);
			pRenderTarget.FillRectangle(rectangle1, pBrush);
			D2DPenColour(fore);
			pRenderTarget.DrawRectangle(rectangle1, pBrush);
		}
	}

	void SurfaceD2D::FillRectangle(PRectangle rc, ColourDesired back) {
		if (pRenderTarget) {
			D2DPenColour(back);
			Rect rectangle1 = Rect(RoundFloat(rc.left), rc.top, RoundFloat(rc.Width()), rc.Height());
			pRenderTarget.FillRectangle(rectangle1, pBrush);
		}
	}

	void SurfaceD2D::FillRectangle(PRectangle rc, Surface &surfacePattern) {
		SurfaceD2D &surfOther = static_cast<SurfaceD2D &>(surfacePattern);
		surfOther.FlushDrawing();
		CanvasImageBrush pBitmapBrush = CanvasImageBrush(surfOther.pCompatibleRenderTarget);

		pRenderTarget.FillRectangle(
			Rect(rc.left, rc.top, rc.Width(), rc.Height()),
			pBitmapBrush);
	}

	void SurfaceD2D::RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) {
		if (pRenderTarget) {
			Rect roundedRectFill =
				Rect(rc.left + 1.0f, rc.top + 1.0f, rc.Width() - 1.0f, rc.Height() - 1.0f);
			D2DPenColour(back);
			pRenderTarget.FillRoundedRectangle(roundedRectFill, 4, 4, pBrush);

			Rect roundedRect =
				Rect(rc.left + 0.5f, rc.top + 0.5f, rc.Width() - 0.5f, rc.Height() - 0.5f);
			D2DPenColour(fore);
			pRenderTarget.DrawRoundedRectangle(roundedRect, 4, 4, pBrush);
		}
	}

	void SurfaceD2D::AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
		ColourDesired outline, int alphaOutline, int /* flags*/) {
		if (pRenderTarget) {
			if (cornerSize == 0) {
				// When corner size is zero, draw square rectangle to prevent blurry pixels at corners
				Rect rectFill = Rect(RoundFloat(rc.left) + 1.0f, rc.top + 1.0f, RoundFloat(rc.Width()) - 1.0f, rc.Height() - 1.0f);
				D2DPenColour(fill, alphaFill);
				pRenderTarget.FillRectangle(rectFill, pBrush);

				Rect rectOutline = Rect(RoundFloat(rc.left) + 0.5f, rc.top + 0.5f, RoundFloat(rc.Width()) - 0.5f, rc.Height() - 0.5f);
				D2DPenColour(outline, alphaOutline);
				pRenderTarget.DrawRectangle(rectOutline, pBrush);
			}
			else {
				const float cornerSizeF = static_cast<float>(cornerSize);
				Rect roundedRectFill =
					Rect(RoundFloat(rc.left) + 1.0f, rc.top + 1.0f, RoundFloat(rc.Width()) - 1.0f, rc.Height() - 1.0f);
				D2DPenColour(fill, alphaFill);
				pRenderTarget.FillRoundedRectangle(roundedRectFill, cornerSizeF, cornerSizeF, pBrush);

				Rect roundedRect =
					Rect(RoundFloat(rc.left) + 0.5f, rc.top + 0.5f, RoundFloat(rc.Width()) - 0.5f, rc.Height() - 0.5f);
				D2DPenColour(outline, alphaOutline);
				pRenderTarget.DrawRoundedRectangle(roundedRect, cornerSizeF, cornerSizeF, pBrush);
			}
		}
	}

	namespace {

		Windows::UI::Color ColorFromColourAlpha(ColourAlpha colour) noexcept {
			Windows::UI::Color col;
			col.R = colour.GetRedComponent();
			col.G = colour.GetGreenComponent();
			col.B = colour.GetBlueComponent();
			col.A = colour.GetAlphaComponent();
			return col;
		}

	}

	void SurfaceD2D::GradientRectangle(PRectangle rc, const std::vector<ColourStop> &stops, GradientOptions options) {
		if (pRenderTarget) {
			float2 startPoint = float2(rc.left, rc.top);
			float2 endPoint;
			switch (options) {
			case GradientOptions::leftToRight:
				endPoint = float2(rc.right, rc.top);
				break;
			case GradientOptions::topToBottom:
			default:
				endPoint = float2(rc.left, rc.bottom);
				break;
			}

			std::vector<CanvasGradientStop> gradientStops;
			for (const ColourStop &stop : stops) {
				gradientStops.push_back({ stop.position, ColorFromColourAlpha(stop.colour) });
			}
			CanvasLinearGradientBrush pBrushLinear = CanvasLinearGradientBrush(pRenderTarget, array_view<const CanvasGradientStop>(gradientStops));
			Rect rectangle = Rect(round(rc.left), rc.top, round(rc.right), rc.bottom);
			pRenderTarget.FillRectangle(rectangle, pBrushLinear);


		}
	}

	void SurfaceD2D::DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage) {
		if (pRenderTarget) {
			if (rc.Width() > width)
				rc.left += floor((rc.Width() - width) / 2);
			rc.right = rc.left + width;
			if (rc.Height() > height)
				rc.top += floor((rc.Height() - height) / 2);
			rc.bottom = rc.top + height;

			std::vector<unsigned char> image(height * width * 4);
			for (int yPixel = 0; yPixel < height; yPixel++) {
				for (int xPixel = 0; xPixel < width; xPixel++) {
					unsigned char *pixel = &image[0] + (yPixel*width + xPixel) * 4;
					const unsigned char alpha = pixelsImage[3];
					// Input is RGBA, output is BGRA with premultiplied alpha
					pixel[2] = (*pixelsImage++) * alpha / 255;
					pixel[1] = (*pixelsImage++) * alpha / 255;
					pixel[0] = (*pixelsImage++) * alpha / 255;
					pixel[3] = *pixelsImage++;
				}
			}

			Size size = Size(width, height);

			Windows::Graphics::DirectX::DirectXPixelFormat dp = Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized;
			array_view<uint8_t const> im = array_view<uint8_t const>(image);
			CanvasBitmap bitmap = CanvasBitmap::CreateFromBytes(pRenderTarget, im, width, height, dp, 72.0);

			Rect rcDestination = { rc.left, rc.top, rc.Width(), rc.Height() };
			pRenderTarget.DrawImage(bitmap, rcDestination);
		}
	}

	void SurfaceD2D::Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) {
		if (pRenderTarget) {
			FLOAT radius = rc.Width() / 2.0f;

			PenColour(back);
			pRenderTarget.FillEllipse((rc.left + rc.right) / 2.0f, (rc.top + rc.bottom) / 2.0f,
				radius, radius, pBrush);
			PenColour(fore);
			pRenderTarget.DrawEllipse((rc.left + rc.right) / 2.0f, (rc.top + rc.bottom) / 2.0f,
				radius, radius, pBrush);
		}
	}

	void SurfaceD2D::Copy(PRectangle rc, Point from, Surface &surfaceSource) {
		SurfaceD2D &surfOther = static_cast<SurfaceD2D &>(surfaceSource);
		surfOther.FlushDrawing();
		Rect rcDestination = { rc.left, rc.top, rc.Width(), rc.Height() };
		Rect rcSource = { from.x, from.y,  rc.Width(), rc.Height() };
		pRenderTarget.DrawImage(surfOther.pCompatibleRenderTarget, rcDestination, rcSource);
		pRenderTarget.Flush();

		//surfOther.pRenderTarget = surfOther.pCompatibleRenderTarget.CreateDrawingSession();
		surfOther.pRenderTarget.Clear(Windows::UI::Colors::White());
	}
	/*
	class BlobInline : implements< BlobInline,ICanvasTextInlineObject >{
		XYPOSITION width;

		void Draw(ICanvasTextRenderer const& textRenderer, float2 const& point, bool isSideways, bool isRightToLeft, IInspectable const& brush) const {}

	public:
		BlobInline(XYPOSITION width_ = 0.0f) noexcept : width(width_) {
		}
		// Defaulted so BlobInline objects can be copied.
		BlobInline(const BlobInline &) = default;
		BlobInline(BlobInline &&) = default;
		BlobInline &operator=(const BlobInline &) = default;
		BlobInline &operator=(BlobInline &&) = default;
		virtual ~BlobInline() {
		}
	};


	class ScreenLineLayout : public IScreenLineLayout {
		CanvasTextLayout textLayout{ nullptr };
		std::string text;
		std::wstring buffer;
		std::vector<BlobInline> blobs;
		static void FillTextLayoutFormats(const IScreenLine *screenLine, CanvasTextLayout &textLayout, std::vector<BlobInline> &blobs);
		static std::wstring ReplaceRepresentation(std::string_view text);
		static size_t GetPositionInLayout(std::string_view text, size_t position);
	public:
		ScreenLineLayout(const IScreenLine *screenLine);
		// Deleted so ScreenLineLayout objects can not be copied
		ScreenLineLayout(const ScreenLineLayout &) = delete;
		ScreenLineLayout(ScreenLineLayout &&) = delete;
		ScreenLineLayout &operator=(const ScreenLineLayout &) = delete;
		ScreenLineLayout &operator=(ScreenLineLayout &&) = delete;
		~ScreenLineLayout() override;
		size_t PositionFromX(XYPOSITION xDistance, bool charPosition) override;
		XYPOSITION XFromPosition(size_t caretPosition) override;
		std::vector<Interval> FindRangeIntervals(size_t start, size_t end) override;
	};

	// Each char can have its own style, so we fill the textLayout with the textFormat of each char

	void ScreenLineLayout::FillTextLayoutFormats(const IScreenLine *screenLine, CanvasTextLayout &textLayout, std::vector<BlobInline> &blobs) {
		// Reserve enough entries up front so they are not moved and the pointers handed
		// to textLayout remain valid.
		const ptrdiff_t numRepresentations = screenLine->RepresentationCount();
		std::string_view text = screenLine->Text();
		const ptrdiff_t numTabs = std::count(std::begin(text), std::end(text), '\t');
		blobs.reserve(numRepresentations + numTabs);

		UINT32 layoutPosition = 0;

		for (size_t bytePosition = 0; bytePosition < screenLine->Length();) {
			const unsigned char uch = screenLine->Text()[bytePosition];
			const unsigned int byteCount = UTF8BytesOfLead[uch];
			const UINT32 codeUnits = static_cast<UINT32>(UTF16LengthFromUTF8ByteCount(byteCount));
			//const DWRITE_TEXT_RANGE textRange = { layoutPosition, codeUnits };

			XYPOSITION representationWidth = screenLine->RepresentationWidth(bytePosition);
			if ((representationWidth == 0.0f) && (screenLine->Text()[bytePosition] == '\t')) {
				CanvasTextLayoutRegion realCaretMetrics;
				textLayout.GetCaretPosition(
					layoutPosition,
					false, // trailing if false, else leading edge
					realCaretMetrics
				);

				const XYPOSITION nextTab = screenLine->TabPositionAfter(realCaretMetrics.LayoutBounds.X);//realPt.x);
				representationWidth = nextTab - realCaretMetrics.LayoutBounds.X;
			}
			if (representationWidth > 0.0f) {
				blobs.push_back(BlobInline(representationWidth));
				//textLayout.SetInlineObject(layoutPosition, codeUnits ,blobs.back());
			};

			FormatAndMetrics *pfm =
				static_cast<FormatAndMetrics *>(screenLine->FontOfPosition(bytePosition)->GetID());


			textLayout.SetFontFamily(layoutPosition, codeUnits, pfm->pTextFormat.FontFamily.LocaleName());

			textLayout.SetFontSize(layoutPosition, codeUnits ,pfm->pTextFormat.FontSize());
			textLayout.SetFontWeight(layoutPosition, codeUnits ,pfm->pTextFormat.FontWeight());
			textLayout.SetFontStyle(layoutPosition, codeUnits ,pfm->pTextFormat.FontStyle());


			textLayout.SetLocaleName(layoutPosition, codeUnits,pfm->pTextFormat.LocaleName());

			textLayout.SetFontStretch(layoutPosition, codeUnits,pfm->pTextFormat.FontStretch());

			bytePosition += byteCount;
			layoutPosition += codeUnits;
		}

	}

	// Convert to a wide character string and replace tabs with X to stop DirectWrite tab expansion

	std::wstring ScreenLineLayout::ReplaceRepresentation(std::string_view text) {
		const TextWide wideText(text, true);
		std::wstring ws(wideText.buffer, wideText.tlen);
		std::replace(ws.begin(), ws.end(), L'\t', L'X');
		return ws;
	}

	// Finds the position in the wide character version of the text.

	size_t ScreenLineLayout::GetPositionInLayout(std::string_view text, size_t position) {
		const std::string_view textUptoPosition = text.substr(0, position);
		return UTF16Length(textUptoPosition);
	}

	ScreenLineLayout::ScreenLineLayout(const IScreenLine *screenLine) {
		// If the text is empty, then no need to go through this function
		if (!screenLine->Length())
			return;

		text = screenLine->Text();

		// Get textFormat
		FormatAndMetrics *pfm = static_cast<FormatAndMetrics *>(screenLine->FontOfPosition(0)->GetID());

		if ( !pfm->pTextFormat) {
			return;
		}

		// Convert the string to wstring and replace the original control characters with their representative chars.
		buffer = ReplaceRepresentation(screenLine->Text());

		// Create a text layout

		CanvasTextLayout pTextLayout = CanvasTextLayout(pRenderTarget, hstring(buffer), pfm->pTextFormat, screenLine->Width(), screenLine->Height());

		// Fill the textLayout chars with their own formats
		FillTextLayoutFormats(screenLine, textLayout, blobs);
	}

	ScreenLineLayout::~ScreenLineLayout() {
	}

	// Get the position from the provided x

	size_t ScreenLineLayout::PositionFromX(XYPOSITION xDistance, bool charPosition) {
		if (!textLayout) {
			return 0;
		}

		// Returns the text position corresponding to the mouse x,y.
		// If hitting the trailing side of a cluster, return the
		// leading edge of the following text position.

		BOOL isTrailingHit;
		BOOL isInside;
		CanvasTextLayoutRegion caretMetrics;

		textLayout.HitTest(
			xDistance,
			0.0f,
			caretMetrics
		);

		DWRITE_HIT_TEST_METRICS hitTestMetrics = {};
		if (isTrailingHit) {
			FLOAT caretX = 0.0f;
			FLOAT caretY = 0.0f;

			// Uses hit-testing to align the current caret position to a whole cluster,
			// rather than residing in the middle of a base character + diacritic,
			// surrogate pair, or character + UVS.

			// Align the caret to the nearest whole cluster.
			textLayout->HitTestTextPosition(
				caretMetrics.textPosition,
				false,
				&caretX,
				&caretY,
				&hitTestMetrics
			);
		}

		size_t pos;
		if (charPosition) {
			pos = isTrailingHit ? hitTestMetrics.textPosition : caretMetrics.textPosition;
		}
		else {
			pos = isTrailingHit ? hitTestMetrics.textPosition + hitTestMetrics.length : caretMetrics.textPosition;
		}

		// Get the character position in original string
		return UTF8PositionFromUTF16Position(text, pos);
	}

	// Finds the point of the caret position

	XYPOSITION ScreenLineLayout::XFromPosition(size_t caretPosition) {
		if (!textLayout) {
			return 0.0;
		}
		// Convert byte positions to wchar_t positions
		const size_t position = GetPositionInLayout(text, caretPosition);

		// Translate text character offset to point x,y.
		CanvasTextLayoutRegion caretMetrics;

		textLayout.GetCaretPosition(
			static_cast<UINT32>(position),
			false, // trailing if false, else leading edge
			caretMetrics
		);


		return caretMetrics.LayoutBounds.X;
	}

	// Find the selection range rectangles

	std::vector<Interval> ScreenLineLayout::FindRangeIntervals(size_t start, size_t end) {
		std::vector<Interval> ret;

		if (!textLayout || (start == end)) {
			return ret;
		}

		// Convert byte positions to wchar_t positions
		const size_t startPos = GetPositionInLayout(text, start);
		const size_t endPos = GetPositionInLayout(text, end);

		// Find selection range length
		const size_t rangeLength = (endPos > startPos) ? (endPos - startPos) : (startPos - endPos);

		// Determine actual number of hit-test ranges
		UINT32 actualHitTestCount = 0;

		// First try with 2 elements and if more needed, allocate.
		std::vector<DWRITE_HIT_TEST_METRICS> hitTestMetrics(2);
		textLayout->HitTestTextRange(
			static_cast<UINT32>(startPos),
			static_cast<UINT32>(rangeLength),
			0, // x
			0, // y
			hitTestMetrics.data(),
			static_cast<UINT32>(hitTestMetrics.size()),
			&actualHitTestCount
		);

		if (actualHitTestCount == 0) {
			return ret;
		}

		if (hitTestMetrics.size() < actualHitTestCount) {
			// Allocate enough room to return all hit-test metrics.
			hitTestMetrics.resize(actualHitTestCount);
			textLayout->HitTestTextRange(
				static_cast<UINT32>(startPos),
				static_cast<UINT32>(rangeLength),
				0, // x
				0, // y
				hitTestMetrics.data(),
				static_cast<UINT32>(hitTestMetrics.size()),
				&actualHitTestCount
			);
		}

		// Get the selection ranges behind the text.

		for (size_t i = 0; i < actualHitTestCount; ++i) {
			// Store selection rectangle
			const DWRITE_HIT_TEST_METRICS &htm = hitTestMetrics[i];
			Interval selectionInterval;

			selectionInterval.left = htm.left;
			selectionInterval.right = htm.left + htm.width;

			ret.push_back(selectionInterval);
		}

		return ret;
	}
	*/
	std::unique_ptr<IScreenLineLayout> SurfaceD2D::Layout(const IScreenLine *screenLine) {
		return {}; //std::make_unique<ScreenLineLayout>(screenLine);
	}

	void SurfaceD2D::DrawTextCommon(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text, UINT fuOptions) {
		SetFont(font_);

		// Use Unicode calls
		const TextWide tbuf(text, unicodeMode, codePageText);
		if (pRenderTarget && pTextFormat && pBrush) {

			CanvasTextLayout pTextLayout = CanvasTextLayout(pRenderTarget, hstring(tbuf.buffer, tbuf.tlen), pTextFormat, rc.Width() + 50, rc.Height());
			//pBrush.Color(Windows::UI:: Colors::Red());
			//pRenderTarget.DrawCircle(50, 50, 50, pBrush);
			pRenderTarget.DrawTextLayout(pTextLayout, rc.left, ybase - yAscent, pBrush);
		}
	}

	void SurfaceD2D::DrawTextNoClip(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text,
		ColourDesired fore, ColourDesired back) {
		if (pRenderTarget) {
			FillRectangle(rc, back);
			D2DPenColour(fore);
			DrawTextCommon(rc, font_, ybase, text, ETO_OPAQUE);
		}
	}

	void SurfaceD2D::DrawTextClipped(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text,
		ColourDesired fore, ColourDesired back) {
		if (pRenderTarget) {
			FillRectangle(rc, back);
			D2DPenColour(fore);
			DrawTextCommon(rc, font_, ybase, text, ETO_OPAQUE | ETO_CLIPPED);
		}
	}

	void SurfaceD2D::DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text,
		ColourDesired fore) {
		// Avoid drawing spaces in transparent mode
		for (size_t i = 0; i < text.length(); i++) {
			if (text[i] != ' ') {
				if (pRenderTarget) {
					D2DPenColour(fore);
					DrawTextCommon(rc, font_, ybase, text, 0);
				}
				return;
			}
		}
	}

	XYPOSITION SurfaceD2D::WidthText(Font &font_, std::string_view text) {
		FLOAT width = 1.0;
		SetFont(font_);
		const TextWide tbuf(text, unicodeMode, codePageText);
		if (pTextFormat) {
			CanvasTextLayout textLayout = CanvasTextLayout(pRenderTarget, hstring(tbuf.buffer, tbuf.tlen), pTextFormat, 1000.0f, 1000.0f);
			width = textLayout.LayoutBounds().Width;
		}
		return width;
	}

	void SurfaceD2D::MeasureWidths(Font &font_, std::string_view text, XYPOSITION *positions) {
		SetFont(font_);
		if (!pTextFormat) {
			return;
		}
		const TextWide tbuf(text, unicodeMode, codePageText);
		TextPositions poses(tbuf.tlen);
		// Initialize poses for safety.
		std::fill(poses.buffer, poses.buffer + tbuf.tlen, 0.0f);
		// Create a layout
		CanvasTextLayout textLayout = CanvasTextLayout(pRenderTarget, hstring(tbuf.buffer, tbuf.tlen), pTextFormat, 10000.0f, 1000.0f);
		auto clusterMetrics = textLayout.ClusterMetrics();

		// A cluster may be more than one WCHAR, such as for "ffi" which is a ligature in the Candara font
		FLOAT position = 0.0f;
		size_t ti = 0;
		for (size_t ci = 0; ci < clusterMetrics.size(); ci++) {
			for (size_t inCluster = 0; inCluster < clusterMetrics[ci].CharacterCount; inCluster++) {
				poses.buffer[ti++] = position + clusterMetrics[ci].Width * (inCluster + 1) / clusterMetrics[ci].CharacterCount;
			}
			position += clusterMetrics[ci].Width;
		}
		PLATFORM_ASSERT(ti == static_cast<size_t>(tbuf.tlen));
		if (unicodeMode) {
			// Map the widths given for UTF-16 characters back onto the UTF-8 input string
			int ui = 0;
			size_t i = 0;
			while (ui < tbuf.tlen) {
				const unsigned char uch = text[i];
				const unsigned int byteCount = UTF8BytesOfLead[uch];
				if (byteCount == 4) {	// Non-BMP
					ui++;
				}
				for (unsigned int bytePos = 0; (bytePos < byteCount) && (i < text.length()); bytePos++) {
					positions[i++] = poses.buffer[ui];
				}
				ui++;
			}
			XYPOSITION lastPos = 0.0f;
			if (i > 0)
				lastPos = positions[i - 1];
			while (i < text.length()) {
				positions[i++] = lastPos;
			}
		}
		else if (codePageText == 0) {

			// One char per position
			PLATFORM_ASSERT(text.length() == static_cast<size_t>(tbuf.tlen));
			for (int kk = 0; kk < tbuf.tlen; kk++) {
				positions[kk] = poses.buffer[kk];
			}

		}
		else {

			// May be one or two bytes per position
			int ui = 0;
			for (size_t i = 0; i < text.length() && ui < tbuf.tlen;) {
				positions[i] = poses.buffer[ui];
				if (DBCSIsLeadByte(codePageText, text[i])) {
					positions[i + 1] = poses.buffer[ui];
					i += 2;
				}
				else {
					i++;
				}

				ui++;
			}
		}
	}

	XYPOSITION SurfaceD2D::Ascent(Font &font_) {
		SetFont(font_);
		return ceil(yAscent);
	}

	XYPOSITION SurfaceD2D::Descent(Font &font_) {
		SetFont(font_);
		return ceil(yDescent);
	}

	XYPOSITION SurfaceD2D::InternalLeading(Font &font_) {
		SetFont(font_);
		return floor(yInternalLeading);
	}

	XYPOSITION SurfaceD2D::Height(Font &font_) {
		return Ascent(font_) + Descent(font_);
	}

	XYPOSITION SurfaceD2D::AverageCharWidth(Font &font_) {
		FLOAT width = 1.0;
		SetFont(font_);
		if (pTextFormat&&pRenderTarget) {
			const WCHAR wszAllAlpha[] = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
			const size_t lenAllAlpha = wcslen(wszAllAlpha);
			CanvasTextLayout textLayout = CanvasTextLayout(pRenderTarget, hstring(wszAllAlpha, lenAllAlpha), pTextFormat, 1000.0f, 1000.0f);
			width = textLayout.LayoutBounds().Width / lenAllAlpha;
		}
		return width;
	}

	void SurfaceD2D::SetClip(PRectangle rc) {
		if (pRenderTarget) {
			Rect rcClip = { rc.left, rc.top, rc.Width(), rc.Height() };
			//clipsActive.push_back(pRenderTarget.CreateLayer(0, rcClip));
		}
	}

	void SurfaceD2D::FlushCachedState() {
	}

	void SurfaceD2D::SetUnicodeMode(bool unicodeMode_) {
		unicodeMode = unicodeMode_;
	}

	void SurfaceD2D::SetDBCSMode(int codePage_) {
		// No action on window as automatically handled by system.
		codePage = codePage_;
	}

	void SurfaceD2D::SetBidiR2L(bool) {
	}


	Surface *Surface::Allocate(int technology) {
		return new SurfaceD2D;
	}


	Window::~Window() {
	}

	void Window::Destroy() {
		//if (wid)
		//	::DestroyWindow(static_cast<HWND>(wid));
		wid = 0;
	}


	PRectangle Window::GetPosition()const {
		Control  c{ nullptr };
		winrt::attach_abi(c, wid);
		PRectangle rect = PRectangle::FromInts(0, 0, c.ActualWidth(), c.ActualHeight());
		winrt::detach_abi(c);
		return rect;
	}

	void Window::SetPosition(PRectangle rc) {
	}

	void Window::SetPositionRelative(PRectangle rc, const Window *relativeTo) {

	}

	PRectangle Window::GetClientPosition()const {
		Control  c{ nullptr };
		winrt::attach_abi(c, wid);
		PRectangle ret = PRectangle::FromInts(0, 0, c.ActualWidth(), c.ActualHeight());
		winrt::detach_abi(c);
		return ret;
	}

	void Window::Show(bool show) {
		Control  c{ nullptr };
		winrt::attach_abi(c, wid);
		if (show)
			c.Visibility(Windows::UI::Xaml::Visibility::Visible);
		else
			c.Visibility(Windows::UI::Xaml::Visibility::Collapsed);
		winrt::detach_abi(c);
	}

	void Window::InvalidateAll() {
		CanvasControl  c{ nullptr };
		winrt::attach_abi(c, wid);
		if (c)
			c.Invalidate();
	}
	void Window::InvalidateRectangle(PRectangle rc) {
		try
		{
			CanvasControl  c{ nullptr };
			winrt::attach_abi(c, wid);

			Windows::Foundation::Rect r(rc.left, rc.top, rc.Width(), rc.Height());
			if (c)
			{
				c.Invalidate();
			}
			wid = winrt::detach_abi(c);

		}
		catch (winrt::hresult_error const& ex)
		{
			HRESULT hr = ex.to_abi(); // HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND).
			winrt::hstring message = ex.message(); // The system cannot find the file specified.
		}
	}

	//static LRESULT Window_SendMessage(Window *w, UINT msg, WPARAM wParam=0, LPARAM lParam=0) {
	//	return ::SendMessage(static_cast<HWND>(w->GetID()), msg, wParam, lParam);
	//}

	void Window::SetFont(Font &font) {
		//Window_SendMessage(this, WM_SETFONT,
		//	reinterpret_cast<WPARAM>(font.GetID()), 0);
	}



	void Window::SetCursor(Cursor curs) {
		using namespace Windows::UI::Core;
		CoreWindow cw = CoreWindow::GetForCurrentThread();
		CoreCursor  cursor{ nullptr };
		switch (curs) {
		case cursorText:
			cursor = CoreCursor(CoreCursorType::IBeam, 0);
			break;
		case cursorUp:
			cursor = CoreCursor(CoreCursorType::UpArrow, 0);
			break;
		case cursorWait:
			cursor = CoreCursor(CoreCursorType::Wait, 0);
			break;
		case cursorHoriz:
			cursor = CoreCursor(CoreCursorType::SizeWestEast, 0);
			break;
		case cursorVert:
			cursor = CoreCursor(CoreCursorType::SizeNorthSouth, 0);
			break;
		case cursorHand:
			cursor = CoreCursor(CoreCursorType::Hand, 0);
			break;
		case cursorReverseArrow:
			//::SetCursor(GetReverseArrowCursor());
			break;
		case cursorArrow:
		case cursorInvalid:	// Should not occur, but just in case.
			cursor = CoreCursor(CoreCursorType::Arrow, 0);
			break;
		}
		if (cursor)
			cw.PointerCursor(cursor);
	}


	/* Returns rectangle of monitor pt is on, both rect and pt are in Window's
	   coordinates */
	PRectangle Window::GetMonitorRect(Point pt) {

		return PRectangle();
	}



	//----------------------------------------------------------------------
	//class ListWidget : public QListWidget {
	//public:
	//	explicit ListWidget(QWidget *parent);
	//	virtual ~ListWidget();

	//	void setDelegate(IListBoxDelegate *lbDelegate);
	//	void selectionChanged();

	//protected:
	//	void mouseReleaseEvent(QMouseEvent * event) override;
	//	void mouseDoubleClickEvent(QMouseEvent *event) override;
	//	QStyleOptionViewItem viewOptions() const override;

	//private:
	//	IListBoxDelegate *delegate;
	//};

	class ListBoxImpl : public ListBox {
	public:
		ListBoxImpl();
		~ListBoxImpl();

		void SetFont(Font &font) override;
		void Create(Window &parent, int ctrlID, Point location,
			int lineHeight, bool unicodeMode_, int technology) override;
		void SetAverageCharWidth(int width) override;
		void SetVisibleRows(int rows) override;
		int GetVisibleRows() const override;
		PRectangle GetDesiredRect() override;
		int CaretFromEdge() override;
		void Clear() override;
		void Append(char *s, int type = -1) override;
		int Length() override;
		void Select(int n) override;
		int GetSelection() override;
		int Find(const char *prefix) override;
		void GetValue(int n, char *value, int len) override;
		void RegisterImage(int type, const char *xpmData) override;
		void RegisterRGBAImage(int type, int width, int height,
			const unsigned char *pixelsImage) override;
		void ClearRegisteredImages() override;
		void SetDelegate(IListBoxDelegate *lbDelegate) override;
		void SetList(const char *list, char separator, char typesep) override;

	private:
		ListView listview{ nullptr };
		bool unicodeMode;
		int visibleRows;
	};
	ListBoxImpl::ListBoxImpl()
		: unicodeMode(false), visibleRows(5)
	{}

	ListBoxImpl::~ListBoxImpl() {}

	void ListBoxImpl::Create(Window &parent,
		int /*ctrlID*/,
		Point location,
		int /*lineHeight*/,
		bool unicodeMode_,
		int)
	{
		unicodeMode = unicodeMode_;

		listview = ListView();

		wid = winrt::get_abi(listview);
	}
	void ListBoxImpl::SetFont(Font &font)
	{

	}
	void ListBoxImpl::SetAverageCharWidth(int /*width*/) {}

	void ListBoxImpl::SetVisibleRows(int rows)
	{
		visibleRows = rows;
	}

	int ListBoxImpl::GetVisibleRows() const
	{
		return visibleRows;
	}
	PRectangle ListBoxImpl::GetDesiredRect()
	{

		return PRectangle(0, 0, listview.ActualWidth(), listview.ActualHeight());
	}
	int ListBoxImpl::CaretFromEdge()
	{
		int extra = 12;
		return  2 * listview.ActualWidth() + extra;
	}
	void ListBoxImpl::Clear()
	{

	}
	void ListBoxImpl::Append(char *s, int type)
	{
		TextWide str(s, unicodeMode);

	}
	int ListBoxImpl::Length()
	{
		return 0;
	}
	void ListBoxImpl::Select(int n)
	{

	}
	int ListBoxImpl::GetSelection()
	{
		return 0;
	}
	int ListBoxImpl::Find(const char *prefix)
	{
		return 0;
	}
	void ListBoxImpl::GetValue(int n, char *value, int len)
	{
	}

	void ListBoxImpl::RegisterImage(int type, const char *xpmData)
	{

	}

	void ListBoxImpl::RegisterRGBAImage(int type, int width, int height, const unsigned char *pixelsImage)
	{
	}

	void ListBoxImpl::ClearRegisteredImages()
	{

	}
	void ListBoxImpl::SetDelegate(IListBoxDelegate *lbDelegate)
	{

	}
	void ListBoxImpl::SetList(const char *list, char separator, char typesep)
	{
		// This method is *not* platform dependent.
		// It is borrowed from the GTK implementation.
		Clear();
		size_t count = strlen(list) + 1;
		std::vector<char> words(list, list + count);
		char *startword = &words[0];
		char *numword = nullptr;
		int i = 0;
		for (; words[i]; i++) {
			if (words[i] == separator) {
				words[i] = '\0';
				if (numword)
					*numword = '\0';
				Append(startword, numword ? atoi(numword + 1) : -1);
				startword = &words[0] + i + 1;
				numword = nullptr;
			}
			else if (words[i] == typesep) {
				numword = &words[0] + i;
			}
		}
		if (startword) {
			if (numword)
				*numword = '\0';
			Append(startword, numword ? atoi(numword + 1) : -1);
		}
	}


	ListBox::ListBox() noexcept {}
	ListBox::~ListBox() {}

	ListBox *ListBox::Allocate()
	{
		return new ListBoxImpl();
	}



	//----------------------------------------------------------------------
	Menu::Menu() noexcept : mid(nullptr) {}
	void Menu::CreatePopUp()
	{
		Destroy();
		MenuFlyout menu;
		mid = winrt::get_abi(menu);
	}

	void Menu::Destroy()
	{
		if (mid) {
			::IUnknown *menu = static_cast<::IUnknown *>(mid);
			//menu->Release();
		}
		mid = nullptr;
	}
	void Menu::Show(Point pt, Window & w)
	{
		MenuFlyout menu{ nullptr };
		winrt::attach_abi(menu, mid);
		CanvasControl  c{ nullptr };
		winrt::attach_abi(c, w.GetID());
		menu.ShowAt(c, Windows::Foundation::Point(pt.x, pt.y));
		winrt::detach_abi(menu);
		winrt::detach_abi(c);
		Destroy();
	}

	DynamicLibrary *DynamicLibrary::Load(const char *modulePath) {
		return nullptr;//static_cast<DynamicLibrary *>(new DynamicLibraryImpl(modulePath));
	}

	ColourDesired Platform::Chrome() {
		Windows::UI::Color gray = Windows::UI::Colors::Gray();
		return ColourDesired(gray.R, gray.G, gray.B);
	}

	ColourDesired Platform::ChromeHighlight() {
		Windows::UI::Color gray = Windows::UI::Colors::LightGray();
		return ColourDesired(gray.R, gray.G, gray.B);
	}

	const char *Platform::DefaultFont() {
		return "Verdana";
	}

	int Platform::DefaultFontSize() {
		return 8;
	}

	unsigned int Platform::DoubleClickTime() {
		return 500;
	}

	void Platform::DebugDisplay(const char *s) {
		//::OutputDebugStringA(s);
	}

	//#define TRACE

#ifdef TRACE
	void Platform::DebugPrintf(const char *format, ...) {
		char buffer[2000];
		va_list pArguments;
		va_start(pArguments, format);
		vsprintf(buffer, format, pArguments);
		va_end(pArguments);
		Platform::DebugDisplay(buffer);
	}
#else
	void Platform::DebugPrintf(const char *, ...) {
	}
#endif

	static bool assertionPopUps = true;

	bool Platform::ShowAssertionPopUps(bool assertionPopUps_) {
		const bool ret = assertionPopUps;
		assertionPopUps = assertionPopUps_;
		return ret;
	}

	void Platform::Assert(const char *c, const char *file, int line) {
		std::wostringstream strStream;
		strStream << L"Assertion " << c << L"failed at " << line << line << assertionPopUps << L"\r\n";
		std::wstring content = strStream.str();
		std::wstring title = L"Assertion failure";
		if (assertionPopUps) {
			MessageDialog dialog(hstring(content), hstring(title));

		}
		else {
			std::ostringstream strStream;
			strStream << "Assertion " << c << "failed at " << line << line << assertionPopUps << "\r\n";
			Platform::DebugDisplay(strStream.str().c_str());
			//::DebugBreak();
			abort();
		}
	}

	void Platform_Initialise(void *hInstance) {

	}

	void Platform_Finalise(bool fromDllMain) {

	}

}