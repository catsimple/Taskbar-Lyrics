#include "NetworkServer.hpp"
#include "CreateWindow.hpp"
#include "nlohmann/json.hpp"
#include <d2d1.h>

namespace
{
    bool 颜色相同(const D2D1::ColorF& 左, const D2D1::ColorF& 右)
    {
        return 左.r == 右.r && 左.g == 右.g && 左.b == 右.b && 左.a == 右.a;
    }
}


网络服务器类::网络服务器类(
    任务栏窗口类* 任务栏窗口,
    unsigned short 端口
) {
    this->任务栏窗口 = 任务栏窗口;

    auto handler = [this] (auto func) {
        auto bind = std::bind(func,this,std::placeholders::_1,std::placeholders::_2);
        return httplib::Server::Handler(bind);
    };

    auto 线程函数 = [this, handler, 端口] () {
        this->网络服务器.Post("/taskbar/font/font", handler(&网络服务器类::字体));
        this->网络服务器.Post("/taskbar/font/color", handler(&网络服务器类::颜色));
        this->网络服务器.Post("/taskbar/font/style", handler(&网络服务器类::样式));
        this->网络服务器.Post("/taskbar/lyrics/lyrics", handler(&网络服务器类::歌词));
        this->网络服务器.Post("/taskbar/lyrics/align", handler(&网络服务器类::对齐));
        this->网络服务器.Post("/taskbar/window/position", handler(&网络服务器类::位置));
        this->网络服务器.Post("/taskbar/window/margin", handler(&网络服务器类::边距));
        this->网络服务器.Post("/taskbar/window/screen", handler(&网络服务器类::屏幕));
        this->网络服务器.Post("/taskbar/close", handler(&网络服务器类::关闭));
        this->网络服务器.listen("127.0.0.1", 端口);
    };

    this->网络服务器_线程 = new std::thread(线程函数);
}


网络服务器类::~网络服务器类()
{
    this->网络服务器.stop();

    this->网络服务器_线程->detach();
    delete this->网络服务器_线程;
    this->网络服务器_线程 = nullptr;

    this->任务栏窗口 = nullptr;
}


void 网络服务器类::字体(
    const httplib::Request& req,
    httplib::Response& res
) {
    auto json = nlohmann::json::parse(req.body);
    auto 新字体 = this->字符转换.from_bytes(
        json["font_family"].get<std::string>()
    );

    if (this->任务栏窗口->呈现窗口->字体名称 != 新字体)
    {
        this->任务栏窗口->呈现窗口->字体名称 = 新字体;
        this->任务栏窗口->呈现窗口->标记重绘();
        PostMessage(this->任务栏窗口->窗口句柄, WM_PAINT, NULL, NULL);
    }
    res.status = 200;
}


void 网络服务器类::颜色(
    const httplib::Request& req,
    httplib::Response& res
) {
    auto json = nlohmann::json::parse(req.body);
    bool 变化 = false;

    auto 浅色主歌词 = D2D1::ColorF(
        json["basic"]["light"]["hex_color"].get<unsigned int>(),
        json["basic"]["light"]["opacity"].get<float>()
    );
    auto 深色主歌词 = D2D1::ColorF(
        json["basic"]["dark"]["hex_color"].get<unsigned int>(),
        json["basic"]["dark"]["opacity"].get<float>()
    );
    auto 浅色副歌词 = D2D1::ColorF(
        json["extra"]["light"]["hex_color"].get<unsigned int>(),
        json["extra"]["light"]["opacity"].get<float>()
    );
    auto 深色副歌词 = D2D1::ColorF(
        json["extra"]["dark"]["hex_color"].get<unsigned int>(),
        json["extra"]["dark"]["opacity"].get<float>()
    );

    if (!颜色相同(this->任务栏窗口->呈现窗口->字体颜色_浅色_主歌词, 浅色主歌词))
    {
        this->任务栏窗口->呈现窗口->字体颜色_浅色_主歌词 = 浅色主歌词;
        变化 = true;
    }
    if (!颜色相同(this->任务栏窗口->呈现窗口->字体颜色_深色_主歌词, 深色主歌词))
    {
        this->任务栏窗口->呈现窗口->字体颜色_深色_主歌词 = 深色主歌词;
        变化 = true;
    }
    if (!颜色相同(this->任务栏窗口->呈现窗口->字体颜色_浅色_副歌词, 浅色副歌词))
    {
        this->任务栏窗口->呈现窗口->字体颜色_浅色_副歌词 = 浅色副歌词;
        变化 = true;
    }
    if (!颜色相同(this->任务栏窗口->呈现窗口->字体颜色_深色_副歌词, 深色副歌词))
    {
        this->任务栏窗口->呈现窗口->字体颜色_深色_副歌词 = 深色副歌词;
        变化 = true;
    }

    if (变化)
    {
        PostMessage(this->任务栏窗口->窗口句柄, WM_PAINT, NULL, NULL);
    }
    res.status = 200;
}


void 网络服务器类::样式(
    const httplib::Request& req,
    httplib::Response& res
) {
    auto json = nlohmann::json::parse(req.body);
    bool 变化 = false;

    auto 主歌词_字重 = json["basic"]["weight"]["value"].get<DWRITE_FONT_WEIGHT>();
    auto 主歌词_斜体 = json["basic"]["slope"].get<DWRITE_FONT_STYLE>();
    auto 主歌词_下划线 = json["basic"]["underline"].get<bool>();
    auto 主歌词_删除线 = json["basic"]["strikethrough"].get<bool>();
    auto 副歌词_字重 = json["extra"]["weight"]["value"].get<DWRITE_FONT_WEIGHT>();
    auto 副歌词_斜体 = json["extra"]["slope"].get<DWRITE_FONT_STYLE>();
    auto 副歌词_下划线 = json["extra"]["underline"].get<bool>();
    auto 副歌词_删除线 = json["extra"]["strikethrough"].get<bool>();

    if (this->任务栏窗口->呈现窗口->字体样式_主歌词_字重 != 主歌词_字重)
    {
        this->任务栏窗口->呈现窗口->字体样式_主歌词_字重 = 主歌词_字重;
        变化 = true;
    }
    if (this->任务栏窗口->呈现窗口->字体样式_主歌词_斜体 != 主歌词_斜体)
    {
        this->任务栏窗口->呈现窗口->字体样式_主歌词_斜体 = 主歌词_斜体;
        变化 = true;
    }
    if (this->任务栏窗口->呈现窗口->字体样式_主歌词_下划线 != 主歌词_下划线)
    {
        this->任务栏窗口->呈现窗口->字体样式_主歌词_下划线 = 主歌词_下划线;
        变化 = true;
    }
    if (this->任务栏窗口->呈现窗口->字体样式_主歌词_删除线 != 主歌词_删除线)
    {
        this->任务栏窗口->呈现窗口->字体样式_主歌词_删除线 = 主歌词_删除线;
        变化 = true;
    }
    if (this->任务栏窗口->呈现窗口->字体样式_副歌词_字重 != 副歌词_字重)
    {
        this->任务栏窗口->呈现窗口->字体样式_副歌词_字重 = 副歌词_字重;
        变化 = true;
    }
    if (this->任务栏窗口->呈现窗口->字体样式_副歌词_斜体 != 副歌词_斜体)
    {
        this->任务栏窗口->呈现窗口->字体样式_副歌词_斜体 = 副歌词_斜体;
        变化 = true;
    }
    if (this->任务栏窗口->呈现窗口->字体样式_副歌词_下划线 != 副歌词_下划线)
    {
        this->任务栏窗口->呈现窗口->字体样式_副歌词_下划线 = 副歌词_下划线;
        变化 = true;
    }
    if (this->任务栏窗口->呈现窗口->字体样式_副歌词_删除线 != 副歌词_删除线)
    {
        this->任务栏窗口->呈现窗口->字体样式_副歌词_删除线 = 副歌词_删除线;
        变化 = true;
    }

    if (变化)
    {
        this->任务栏窗口->呈现窗口->标记重绘();
        PostMessage(this->任务栏窗口->窗口句柄, WM_PAINT, NULL, NULL);
    }
    res.status = 200;
}


void 网络服务器类::歌词(
    const httplib::Request& req,
    httplib::Response& res
) {
    auto json = nlohmann::json::parse(req.body);
    auto 新主歌词 = this->字符转换.from_bytes(
        json["basic"].get<std::string>()
    );
    auto 新副歌词 = this->字符转换.from_bytes(
        json["extra"].get<std::string>()
    );

    if (this->任务栏窗口->呈现窗口->主歌词 != 新主歌词 || this->任务栏窗口->呈现窗口->副歌词 != 新副歌词)
    {
        this->任务栏窗口->呈现窗口->主歌词 = 新主歌词;
        this->任务栏窗口->呈现窗口->副歌词 = 新副歌词;
        PostMessage(this->任务栏窗口->窗口句柄, WM_PAINT, NULL, NULL);
    }
    res.status = 200;
}


void 网络服务器类::对齐(
    const httplib::Request& req,
    httplib::Response& res
) {
    auto json = nlohmann::json::parse(req.body);
    auto 新主对齐 = json["basic"].get<DWRITE_TEXT_ALIGNMENT>();
    auto 新副对齐 = json["extra"].get<DWRITE_TEXT_ALIGNMENT>();

    if (this->任务栏窗口->呈现窗口->对齐方式_主歌词 != 新主对齐 || this->任务栏窗口->呈现窗口->对齐方式_副歌词 != 新副对齐)
    {
        this->任务栏窗口->呈现窗口->对齐方式_主歌词 = 新主对齐;
        this->任务栏窗口->呈现窗口->对齐方式_副歌词 = 新副对齐;
        PostMessage(this->任务栏窗口->窗口句柄, WM_PAINT, NULL, NULL);
    }
    res.status = 200;
}


void 网络服务器类::位置(
    const httplib::Request& req,
    httplib::Response& res
) {
    auto json = nlohmann::json::parse(req.body);
    auto 新位置 = json["position"]["value"].get<WindowAlignment>();

    if (this->任务栏窗口->呈现窗口->窗口位置 != 新位置)
    {
        this->任务栏窗口->呈现窗口->窗口位置 = 新位置;
        PostMessage(this->任务栏窗口->窗口句柄, WM_PAINT, NULL, NULL);
    }
    res.status = 200;
}


void 网络服务器类::边距(
    const httplib::Request& req,
    httplib::Response& res
) {
    auto json = nlohmann::json::parse(req.body);
    auto 新左边距 = json["left"].get<int>();
    auto 新右边距 = json["right"].get<int>();

    if (this->任务栏窗口->呈现窗口->左边距 != 新左边距 || this->任务栏窗口->呈现窗口->右边距 != 新右边距)
    {
        this->任务栏窗口->呈现窗口->左边距 = 新左边距;
        this->任务栏窗口->呈现窗口->右边距 = 新右边距;
        PostMessage(this->任务栏窗口->窗口句柄, WM_PAINT, NULL, NULL);
    }
    res.status = 200;
}


void 网络服务器类::屏幕(
    const httplib::Request& req,
    httplib::Response& res
) {
    auto json = nlohmann::json::parse(req.body);
    auto parent_taskbar = json["parent_taskbar"]["value"].get<std::string>();
    auto 新任务栏句柄 = FindWindow(this->字符转换.from_bytes(parent_taskbar).c_str(), NULL);
    auto 新开始按钮句柄 = FindWindowEx(新任务栏句柄, NULL, L"Start", NULL);

    if (this->任务栏窗口->呈现窗口->任务栏_句柄 != 新任务栏句柄 || this->任务栏窗口->呈现窗口->开始按钮_句柄 != 新开始按钮句柄)
    {
        this->任务栏窗口->呈现窗口->任务栏_句柄 = 新任务栏句柄;
        this->任务栏窗口->呈现窗口->开始按钮_句柄 = 新开始按钮句柄;

        GetWindowRect(this->任务栏窗口->呈现窗口->任务栏_句柄, &this->任务栏窗口->呈现窗口->任务栏_矩形);
        GetWindowRect(this->任务栏窗口->呈现窗口->开始按钮_句柄, &this->任务栏窗口->呈现窗口->开始按钮_矩形);

        SetParent(this->任务栏窗口->窗口句柄, this->任务栏窗口->呈现窗口->任务栏_句柄);

        PostMessage(this->任务栏窗口->窗口句柄, WM_PAINT, NULL, NULL);
    }
    res.status = 200;
}


void 网络服务器类::关闭(
    const httplib::Request& req,
    httplib::Response& res
) {
    this->任务栏窗口->呈现窗口->主歌词 = L"检测到网易云音乐重载页面";
    this->任务栏窗口->呈现窗口->副歌词 = L"正在尝试关闭任务栏歌词...";

    PostMessage(this->任务栏窗口->窗口句柄, WM_PAINT, NULL, NULL);
    PostMessage(this->任务栏窗口->窗口句柄, WM_CLOSE, NULL, NULL);
    res.status = 200;
}
