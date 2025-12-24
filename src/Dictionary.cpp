// Dictionary.cpp - Use Unicode escape sequences for portability
#include "Dictionary.h"

CDictionary::CDictionary() {
}

CDictionary::~CDictionary() {
}

std::vector<std::wstring> CDictionary::Lookup(const std::wstring& code) const {
    auto it = _dictionary.find(code);
    if (it != _dictionary.end()) {
        return it->second;
    }
    return std::vector<std::wstring>();
}

void CDictionary::Initialize() {
    // Common Traditional Chinese characters with simple codes
    // Using Unicode escape sequences for portability

    // Single letter codes - Cangjie radicals
    AddEntry(L"a", L"\x65E5");  // 日
    AddEntry(L"a", L"\x66F0");  // 曰
    AddEntry(L"a", L"\x5B89");  // 安

    AddEntry(L"b", L"\x6708");  // 月
    AddEntry(L"b", L"\x80A9");  // 肩

    AddEntry(L"c", L"\x91D1");  // 金
    AddEntry(L"c", L"\x9322");  // 錢

    AddEntry(L"d", L"\x6728");  // 木
    AddEntry(L"d", L"\x6797");  // 林
    AddEntry(L"d", L"\x68EE");  // 森

    AddEntry(L"e", L"\x6C34");  // 水
    AddEntry(L"e", L"\x6C38");  // 永

    AddEntry(L"f", L"\x706B");  // 火
    AddEntry(L"f", L"\x708E");  // 炎

    AddEntry(L"g", L"\x571F");  // 土
    AddEntry(L"g", L"\x5730");  // 地

    AddEntry(L"h", L"\x7AF9");  // 竹
    AddEntry(L"h", L"\x7B46");  // 筆

    AddEntry(L"i", L"\x6208");  // 戈
    AddEntry(L"i", L"\x6211");  // 我

    AddEntry(L"j", L"\x5341");  // 十
    AddEntry(L"j", L"\x5343");  // 千

    AddEntry(L"k", L"\x5927");  // 大
    AddEntry(L"k", L"\x592A");  // 太

    AddEntry(L"l", L"\x4E2D");  // 中
    AddEntry(L"l", L"\x7530");  // 田

    AddEntry(L"m", L"\x4E00");  // 一
    AddEntry(L"m", L"\x4E03");  // 七

    AddEntry(L"n", L"\x5F13");  // 弓
    AddEntry(L"n", L"\x5F35");  // 張

    AddEntry(L"o", L"\x4EBA");  // 人
    AddEntry(L"o", L"\x5165");  // 入

    AddEntry(L"p", L"\x5FC3");  // 心
    AddEntry(L"p", L"\x60F3");  // 想

    AddEntry(L"q", L"\x624B");  // 手
    AddEntry(L"q", L"\x6253");  // 打

    AddEntry(L"r", L"\x53E3");  // 口
    AddEntry(L"r", L"\x5403");  // 吃

    AddEntry(L"s", L"\x5C38");  // 尸
    AddEntry(L"s", L"\x5C4B");  // 屋

    AddEntry(L"t", L"\x5EFF");  // 廿
    AddEntry(L"t", L"\x8349");  // 草

    AddEntry(L"u", L"\x5C71");  // 山
    AddEntry(L"u", L"\x5CF0");  // 峰

    AddEntry(L"v", L"\x5973");  // 女
    AddEntry(L"v", L"\x597D");  // 好

    AddEntry(L"w", L"\x7530");  // 田
    AddEntry(L"w", L"\x7537");  // 男

    AddEntry(L"x", L"\x96E3");  // 難
    AddEntry(L"x", L"\x91CD");  // 重

    AddEntry(L"y", L"\x535C");  // 卜
    AddEntry(L"y", L"\x4EA4");  // 交

    AddEntry(L"z", L"\x7D1A");  // 級

    // Two letter codes - common characters
    AddEntry(L"aa", L"\x660C");  // 昌
    AddEntry(L"ab", L"\x660E");  // 明
    AddEntry(L"am", L"\x65E9");  // 早

    AddEntry(L"bd", L"\x6735");  // 朵
    AddEntry(L"bm", L"\x6709");  // 有
    AddEntry(L"bu", L"\x5C71");  // 山

    AddEntry(L"dd", L"\x6797");  // 林
    AddEntry(L"df", L"\x6770");  // 杰

    AddEntry(L"hk", L"\x7B54");   // 答
    AddEntry(L"hml", L"\x4F60");  // 你

    AddEntry(L"jj", L"\x5EFF");  // 廿
    AddEntry(L"jr", L"\x4EC0");  // 什

    AddEntry(L"ka", L"\x5520");  // 唠
    AddEntry(L"kd", L"\x5433");  // 吳
    AddEntry(L"km", L"\x544A");  // 告
    AddEntry(L"kr", L"\x548C");  // 和

    AddEntry(L"ll", L"\x7551");  // 畑
    AddEntry(L"lm", L"\x7531");  // 由

    AddEntry(L"mb", L"\x4E14");   // 且
    AddEntry(L"mm", L"\x4E8C");   // 二
    AddEntry(L"mmm", L"\x4E09");  // 三
    AddEntry(L"mr", L"\x662F");   // 是

    AddEntry(L"nk", L"\x597D");  // 好
    AddEntry(L"nm", L"\x5B89");  // 安
    AddEntry(L"no", L"\x5979");  // 她

    AddEntry(L"ohni", L"\x4F60");  // 你
    AddEntry(L"ohr", L"\x4F55");   // 何
    AddEntry(L"oir", L"\x6642");   // 時
    AddEntry(L"oll", L"\x7684");   // 的
    AddEntry(L"omg", L"\x5728");   // 在
    AddEntry(L"omr", L"\x4ED6");   // 他
    AddEntry(L"onf", L"\x4EE5");   // 以

    AddEntry(L"pa", L"\x6027");  // 性
    AddEntry(L"pd", L"\x60F3");  // 想

    AddEntry(L"rc", L"\x5509");   // 唉
    AddEntry(L"rk", L"\x4ECA");   // 今
    AddEntry(L"ro", L"\x500B");   // 個
    AddEntry(L"rr", L"\x54C1");   // 品
    AddEntry(L"rrr", L"\x5668");  // 器

    AddEntry(L"vfd", L"\x597D");  // 好
    AddEntry(L"vnd", L"\x5982");  // 如
    AddEntry(L"vnm", L"\x5B89");  // 安

    AddEntry(L"wg", L"\x7532");   // 甲
    AddEntry(L"wl", L"\x754C");   // 界
    AddEntry(L"wlb", L"\x7576");  // 當

    AddEntry(L"yr", L"\x8A00");    // 言
    AddEntry(L"yrcr", L"\x8A9E");  // 語
    AddEntry(L"yrhv", L"\x8B1D");  // 謝

    // Common phrases and words
    AddEntry(L"hmlvfd", L"\x4F60\x597D");    // 你好
    AddEntry(L"yrhvyrhv", L"\x8B1D\x8B1D");  // 謝謝

    // Numbers using codes
    AddEntry(L"mmmm", L"\x56DB");  // 四
    AddEntry(L"mdm", L"\x4E94");   // 五
    AddEntry(L"ju", L"\x4E03");    // 七
}

void CDictionary::AddEntry(const std::wstring& code, const std::wstring& character) {
    _dictionary[code].push_back(character);
}
