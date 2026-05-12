#include "RenderWindow.hpp"

#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")


呈现窗口类::呈现窗口类(
    HWND* 窗口句柄
) {
    this->窗口句柄 = 窗口句柄;

    this->任务栏_句柄 = FindWindow(L"Shell_TrayWnd", NULL);
    this->通知区域_句柄 = FindWindowEx(this->任务栏_句柄, NULL, L"TrayNotifyWnd", NULL);
    this->开始按钮_句柄 = FindWindowEx(this->任务栏_句柄, NULL, L"Start", NULL);
    HWND 最小化区域_句柄 = FindWindowEx(this->任务栏_句柄, NULL, L"ReBarWindow32", NULL);
    this->活动区域_句柄 = FindWindowEx(最小化区域_句柄, NULL, L"MSTaskSwWClass", NULL);

    D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &this->D2D工厂
    );

    D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties = D2D1::RenderTargetProperties();
    renderTargetProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    renderTargetProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

    this->D2D工厂->CreateDCRenderTarget(
        &renderTargetProperties,
        &this->D2D呈现目标
    );

    this->D2D呈现目标->CreateSolidColorBrush(
        D2D1::ColorF(0x000000, 1),
        &this->D2D纯色笔刷
    );

    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&this->DWrite工厂)
    );
}


呈现窗口类::~呈现窗口类()
{
    if (this->DWrite主歌词文本格式) { this->DWrite主歌词文本格式->Release(); this->DWrite主歌词文本格式 = nullptr; }
    if (this->DWrite副歌词文本格式) { this->DWrite副歌词文本格式->Release(); this->DWrite副歌词文本格式 = nullptr; }
    if (this->D2D工厂) { this->D2D工厂->Release(); this->D2D工厂 = nullptr; }
    if (this->D2D呈现目标) { this->D2D呈现目标->Release(); this->D2D呈现目标 = nullptr; }
    if (this->D2D纯色笔刷) { this->D2D纯色笔刷->Release(); this->D2D纯色笔刷 = nullptr; }
    if (this->DWrite工厂) { this->DWrite工厂->Release(); this->DWrite工厂 = nullptr; }
    this->窗口句柄 = nullptr;
}


void 呈现窗口类::标记重绘()
{
    this->主歌词格式脏 = true;
    this->副歌词格式脏 = true;
    this->主歌词布局脏 = true;
    this->副歌词布局脏 = true;
    this->窗口大小脏 = true;
}


bool 呈现窗口类::需要重绘()
{
    if (this->主歌词布局脏 || this->副歌词布局脏 || this->窗口大小脏) return true;
    if (this->主歌词 != this->上次主歌词) return true;
    if (this->副歌词 != this->上次副歌词) return true;
    if (this->上次副歌词空 != this->副歌词.empty()) return true;

    auto 当前主色 = this->深浅模式 ? this->字体颜色_浅色_主歌词 : this->字体颜色_深色_主歌词;
    auto 当前副色 = this->深浅模式 ? this->字体颜色_浅色_副歌词 : this->字体颜色_深色_副歌词;
    if (当前主色.r != 上次主歌词颜色.r || 当前主色.g != 上次主歌词颜色.g || 当前主色.b != 上次主歌词颜色.b || 当前主色.a != 上次主歌词颜色.a) return true;
    if (当前副色.r != 上次副歌词颜色.r || 当前副色.g != 上次副歌词颜色.g || 当前副色.b != 上次副歌词颜色.b || 当前副色.a != 上次副歌词颜色.a) return true;
    if (this->对齐方式_主歌词 != this->上次主歌词对齐) return true;
    if (this->对齐方式_副歌词 != this->上次副歌词对齐) return true;

    return false;
}


void 呈现窗口类::确保文本格式()
{
    if (this->主歌词格式脏)
    {
        if (this->DWrite主歌词文本格式)
        {
            this->DWrite主歌词文本格式->Release();
            this->DWrite主歌词文本格式 = nullptr;
        }
        this->DWrite工厂->CreateTextFormat(
            this->字体名称.c_str(),
            nullptr,
            this->字体样式_主歌词_字重,
            this->字体样式_主歌词_斜体,
            DWRITE_FONT_STRETCH_NORMAL,
            this->DPI(this->副歌词.empty() ? 20 : 15),
            L"zh-CN",
            &this->DWrite主歌词文本格式
        );
        this->主歌词格式脏 = false;
    }

    if (this->副歌词格式脏)
    {
        if (this->DWrite副歌词文本格式)
        {
            this->DWrite副歌词文本格式->Release();
            this->DWrite副歌词文本格式 = nullptr;
        }
        this->DWrite工厂->CreateTextFormat(
            this->字体名称.c_str(),
            nullptr,
            this->字体样式_副歌词_字重,
            this->字体样式_副歌词_斜体,
            DWRITE_FONT_STRETCH_NORMAL,
            this->DPI(15),
            L"zh-CN",
            &this->DWrite副歌词文本格式
        );
        this->副歌词格式脏 = false;
    }
}


bool 呈现窗口类::需要隐藏窗口()
{
    if (!IsWindowVisible(this->任务栏_句柄))
    {
        return true;
    }

    HWND 前台窗口 = GetForegroundWindow();
    if (!前台窗口 || 前台窗口 == *this->窗口句柄 || 前台窗口 == this->任务栏_句柄)
    {
        return false;
    }

    if (IsIconic(前台窗口) || !IsWindowVisible(前台窗口))
    {
        return false;
    }

    RECT 前台矩形 = {};
    if (!GetWindowRect(前台窗口, &前台矩形))
    {
        return false;
    }

    HMONITOR 监视器 = MonitorFromWindow(前台窗口, MONITOR_DEFAULTTONEAREST);
    if (!监视器)
    {
        return false;
    }

    MONITORINFO 监视器信息 = {};
    监视器信息.cbSize = sizeof(监视器信息);
    if (!GetMonitorInfo(监视器, &监视器信息))
    {
        return false;
    }

    const RECT& 屏幕矩形 = 监视器信息.rcMonitor;
    constexpr int 允许误差 = 2;
    return
        abs(前台矩形.left - 屏幕矩形.left) <= 允许误差 &&
        abs(前台矩形.top - 屏幕矩形.top) <= 允许误差 &&
        abs(前台矩形.right - 屏幕矩形.right) <= 允许误差 &&
        abs(前台矩形.bottom - 屏幕矩形.bottom) <= 允许误差;
}


void 呈现窗口类::更新窗口(bool 强制重绘)
{
    bool 当前任务栏可见 = IsWindowVisible(this->任务栏_句柄);
    bool 当前应当隐藏 = this->需要隐藏窗口();
    bool 任务栏可见性变化 = 当前任务栏可见 != this->上次任务栏可见;
    bool 隐藏状态变化 = 当前应当隐藏 != this->上次应当隐藏;
    if (任务栏可见性变化 || 隐藏状态变化)
    {
        this->上次任务栏可见 = 当前任务栏可见;
        this->上次应当隐藏 = 当前应当隐藏;
        强制重绘 = true;
    }

    if (当前应当隐藏)
    {
        if ((任务栏可见性变化 || 隐藏状态变化) && IsWindowVisible(*this->窗口句柄))
        {
            ShowWindow(*this->窗口句柄, SW_HIDE);
        }
        return;
    }

    if ((任务栏可见性变化 || 隐藏状态变化) && !IsWindowVisible(*this->窗口句柄))
    {
        ShowWindow(*this->窗口句柄, SW_SHOWNOACTIVATE);
    }

    RECT 旧任务栏_矩形 = this->任务栏_矩形;
    RECT 旧开始按钮_矩形 = this->开始按钮_矩形;
    RECT 旧活动区域_矩形 = this->活动区域_矩形;
    RECT 旧通知区域_矩形 = this->通知区域_矩形;

    GetWindowRect(this->任务栏_句柄, &this->任务栏_矩形);
    GetWindowRect(this->通知区域_句柄, &this->通知区域_矩形);
    GetWindowRect(this->开始按钮_句柄, &this->开始按钮_矩形);
    GetWindowRect(this->活动区域_句柄, &this->活动区域_矩形);

    bool 布局变化 = false;
    if (std::memcmp(&旧任务栏_矩形, &this->任务栏_矩形, sizeof(RECT))) 布局变化 = true;
    if (this->居中对齐)
    {
        if (std::memcmp(&旧开始按钮_矩形, &this->开始按钮_矩形, sizeof(RECT))) 布局变化 = true;
    }
    else
    {
        if (std::memcmp(&旧活动区域_矩形, &this->活动区域_矩形, sizeof(RECT))) 布局变化 = true;
        if (std::memcmp(&旧通知区域_矩形, &this->通知区域_矩形, sizeof(RECT))) 布局变化 = true;
    }

    long 任务栏左 = this->任务栏_矩形.left;
    long 任务栏上 = this->任务栏_矩形.top;

    RECT 通知区 = this->通知区域_矩形;
    RECT 开始键 = this->开始按钮_矩形;
    RECT 活动区 = this->活动区域_矩形;
    RECT 任务栏 = this->任务栏_矩形;

    偏移矩形(通知区, 任务栏左, 任务栏上);
    偏移矩形(开始键, 任务栏左, 任务栏上);
    偏移矩形(活动区, 任务栏左, 任务栏上);
    偏移矩形(任务栏, 任务栏左, 任务栏上);

    long 左 = 0;
    long 上 = 0;
    long 宽 = 0;
    long 高 = 任务栏.bottom - 任务栏.top;

    switch (this->窗口位置)
    {
        case WindowAlignment::WindowAlignmentAdaptive:
        {
            if (this->居中对齐)
            {
                左 = static_cast<long>(this->组件按钮 ? this->DPI(160) : 0) + this->左边距;
                宽 = 开始键.left - static_cast<long>(this->组件按钮 ? this->DPI(160) : 0) - this->左边距 - this->右边距;
            }
            else
            {
                左 = 活动区.right + this->左边距;
                宽 = 通知区.left - 活动区.right - this->左边距 - this->右边距;
            }
        }
        break;

        case WindowAlignment::WindowAlignmentLeft:
        {
            if (this->居中对齐)
            {
                左 = static_cast<long>(this->组件按钮 ? this->DPI(160) : 0) + this->左边距;
                宽 = 开始键.left - static_cast<long>(this->组件按钮 ? this->DPI(160) : 0) - this->左边距 - this->右边距;
            }
            else
            {
                左 = 0 + this->左边距;
                宽 = 通知区.left - 0 - this->左边距 - this->右边距;
            }
        }
        break;

        case WindowAlignment::WindowAlignmentCenter:
        {
            int center = (任务栏.right - 任务栏.left) / 2;
            int lw = 活动区.right - 开始键.left;
            int rw = 通知区.right - 通知区.left;

            if (lw > rw)
            {
                左 = lw + this->左边距;
                宽 = (center - lw) * 2 - this->左边距 - this->右边距;
            }
            else
            {
                左 = center - (center - rw) + this->左边距;
                宽 = (center - rw) * 2 - this->左边距 - this->右边距;
            }
        }
        break;

        case WindowAlignment::WindowAlignmentRight:
        {
            左 = 活动区.right + this->左边距;
            宽 = 通知区.left - 活动区.right - this->左边距 - this->右边距;
        }
        break;
    }

    bool 位置变化 = (左 != this->上次左 || 上 != this->上次上 || 宽 != this->上次宽 || 高 != this->上次高);

    if (!强制重绘 && !布局变化 && !位置变化 && !this->需要重绘())
    {
        return;
    }

    BringWindowToTop(*this->窗口句柄);
    this->上次左 = 左;
    this->上次上 = 上;
    this->上次宽 = 宽;
    this->上次高 = 高;

    MoveWindow(*this->窗口句柄, 左, 上, 宽, 高, false);
    this->绘制窗口(左, 上, 宽, 高, 任务栏左, 任务栏上);
    RedrawWindow(*this->窗口句柄, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
}


void 呈现窗口类::绘制窗口(
    long 左,
    long 上,
    long 宽,
    long 高,
    long 屏幕偏移左,
    long 屏幕上偏移上
) {
    RECT rect = {};
    GetClientRect(*this->窗口句柄, &rect);

    HDC hdc = GetDC(*this->窗口句柄);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, 宽, 高);
    HBITMAP oldBitmap = HBITMAP(SelectObject(memDC, memBitmap));

    this->绘制歌词(memDC, rect);

    BLENDFUNCTION blend = {
        AC_SRC_OVER,
        0,
        255,
        AC_SRC_ALPHA
    };

    POINT 目标位置 = { 左 + 屏幕偏移左, 上 + 屏幕上偏移上 };
    SIZE 大小 = { 宽, 高 };
    POINT 来源位置 = { 0, 0 };

    UpdateLayeredWindow(*this->窗口句柄, hdc, &目标位置, &大小, memDC, &来源位置, 0, &blend, ULW_ALPHA);

    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
    ReleaseDC(*this->窗口句柄, hdc);
}


void 呈现窗口类::绘制歌词(
    HDC& hdc,
    RECT& rect
) {
    this->D2D呈现目标->BindDC(hdc, &rect);
    this->D2D呈现目标->BeginDraw();

    this->确保文本格式();

    DWRITE_TRIMMING 歌词裁剪 = {
        DWRITE_TRIMMING_GRANULARITY_CHARACTER,
        0,
        0
    };

    auto 当前主色 = this->深浅模式 ? this->字体颜色_浅色_主歌词 : this->字体颜色_深色_主歌词;
    auto 当前副色 = this->深浅模式 ? this->字体颜色_浅色_副歌词 : this->字体颜色_深色_副歌词;
    bool 副空 = this->副歌词.empty();

    IDWriteTextFormat* 主格式 = this->DWrite主歌词文本格式;
    IDWriteTextFormat* 副格式 = 副空 ? nullptr : this->DWrite副歌词文本格式;

    if (副空)
    {
        D2D1_RECT_F 主歌词_矩形 = D2D1::RectF(
            max(0, rect.left + this->DPI(10)),
            max(0, rect.top + this->DPI(10)),
            max(0, rect.right - this->DPI(10)),
            max(0, rect.bottom - this->DPI(10))
        );

        主歌词_矩形.right = max(主歌词_矩形.right, 主歌词_矩形.left);
        主歌词_矩形.bottom = max(主歌词_矩形.bottom, 主歌词_矩形.top);

        IDWriteTextLayout* 布局 = nullptr;
        this->DWrite工厂->CreateTextLayout(
            this->主歌词.c_str(),
            this->主歌词.size(),
            主格式,
            (float)(主歌词_矩形.right - 主歌词_矩形.left),
            (float)(主歌词_矩形.bottom - 主歌词_矩形.top),
            &布局
        );
        布局->SetTrimming(&歌词裁剪, nullptr);
        布局->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        布局->SetTextAlignment(this->对齐方式_主歌词);
        布局->SetUnderline(this->字体样式_主歌词_下划线, DWRITE_TEXT_RANGE{0, this->主歌词.size()});
        布局->SetStrikethrough(this->字体样式_主歌词_删除线, DWRITE_TEXT_RANGE{0, this->主歌词.size()});

        this->D2D纯色笔刷->SetColor(当前主色);
        this->D2D呈现目标->DrawTextLayout(
            D2D1::Point2F(主歌词_矩形.left, 主歌词_矩形.top),
            布局,
            this->D2D纯色笔刷,
            D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
        );
        布局->Release();
    }
    else
    {
        D2D1_RECT_F 主歌词_矩形 = D2D1::RectF(
            max(0, rect.left + this->DPI(5)),
            max(0, rect.top + this->DPI(5)),
            max(0, rect.right - this->DPI(5)),
            max(0, rect.bottom / 2.0f)
        );

        主歌词_矩形.right = max(主歌词_矩形.right, 主歌词_矩形.left);
        主歌词_矩形.bottom = max(主歌词_矩形.bottom, 主歌词_矩形.top);

        IDWriteTextLayout* 主布局 = nullptr;
        this->DWrite工厂->CreateTextLayout(
            this->主歌词.c_str(),
            this->主歌词.size(),
            主格式,
            (float)(主歌词_矩形.right - 主歌词_矩形.left),
            (float)(主歌词_矩形.bottom - 主歌词_矩形.top),
            &主布局
        );
        主布局->SetTrimming(&歌词裁剪, nullptr);
        主布局->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        主布局->SetTextAlignment(this->对齐方式_主歌词);
        主布局->SetUnderline(this->字体样式_主歌词_下划线, DWRITE_TEXT_RANGE{0, this->主歌词.size()});
        主布局->SetStrikethrough(this->字体样式_主歌词_删除线, DWRITE_TEXT_RANGE{0, this->主歌词.size()});

        this->D2D纯色笔刷->SetColor(当前主色);
        this->D2D呈现目标->DrawTextLayout(
            D2D1::Point2F(主歌词_矩形.left, 主歌词_矩形.top),
            主布局,
            this->D2D纯色笔刷,
            D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
        );
        主布局->Release();

        D2D1_RECT_F 副歌词_矩形 = D2D1::RectF(
            max(0, rect.left + this->DPI(5)),
            max(0, rect.bottom / 2.0f),
            max(0, rect.right - this->DPI(5)),
            max(0, rect.bottom - this->DPI(5))
        );

        副歌词_矩形.right = max(副歌词_矩形.right, 副歌词_矩形.left);
        副歌词_矩形.bottom = max(副歌词_矩形.bottom, 副歌词_矩形.top);

        IDWriteTextLayout* 副布局 = nullptr;
        this->DWrite工厂->CreateTextLayout(
            this->副歌词.c_str(),
            this->副歌词.size(),
            副格式,
            (float)(副歌词_矩形.right - 副歌词_矩形.left),
            (float)(副歌词_矩形.bottom - 副歌词_矩形.top),
            &副布局
        );
        副布局->SetTrimming(&歌词裁剪, nullptr);
        副布局->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        副布局->SetTextAlignment(this->对齐方式_副歌词);
        副布局->SetUnderline(this->字体样式_副歌词_下划线, DWRITE_TEXT_RANGE{0, this->副歌词.size()});
        副布局->SetStrikethrough(this->字体样式_副歌词_删除线, DWRITE_TEXT_RANGE{0, this->副歌词.size()});

        this->D2D纯色笔刷->SetColor(当前副色);
        this->D2D呈现目标->DrawTextLayout(
            D2D1::Point2F(副歌词_矩形.left, 副歌词_矩形.top),
            副布局,
            this->D2D纯色笔刷,
            D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
        );
        副布局->Release();
    }

    this->D2D呈现目标->EndDraw();

    this->上次主歌词 = this->主歌词;
    this->上次副歌词 = this->副歌词;
    this->上次副歌词空 = 副空;
    this->上次主歌词颜色 = 当前主色;
    this->上次副歌词颜色 = 当前副色;
    this->上次主歌词对齐 = this->对齐方式_主歌词;
    this->上次副歌词对齐 = this->对齐方式_副歌词;
    this->上次窗口宽 = rect.right - rect.left;
    this->上次窗口高 = rect.bottom - rect.top;
    this->主歌词布局脏 = false;
    this->副歌词布局脏 = false;
    this->窗口大小脏 = false;
}


void 呈现窗口类::偏移矩形(
    RECT& 矩形,
    long 水平偏移,
    long 垂直偏移
) {
    矩形.left   -= 水平偏移;
    矩形.right  -= 水平偏移;
    矩形.top    -= 垂直偏移;
    矩形.bottom -= 垂直偏移;
}


float 呈现窗口类::DPI(
    UINT 像素大小
) {
    auto 屏幕DPI = GetDpiForWindow(*this->窗口句柄);
    auto 新像素大小 = static_cast<float>(像素大小 * 屏幕DPI / 96);
    return 新像素大小;
}
