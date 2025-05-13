#pragma once

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <type_traits>
#include <tuple>
#include <algorithm>
#include <iostream>

#include "base.hpp"
#include "style.hpp"
#include "kastr.hpp"
#include "kastring.hpp"

namespace kastring {
inline KAString KAStr::own() {
    return KAString(*this);
}

template <typename T>
inline KAString KAStr::join(const std::vector<T>& parts) const {
    return this->join(parts.begin(), parts.end());
}

// 主迭代器版本（保持不变）
template <typename It>
inline KAString KAStr::join(It first, It last) const {
    if (first == last) return KAString();

    KAString result;

    typedef typename std::iterator_traits<It>::iterator_category Category;
    // typedef typename std::iterator_traits<It>::value_type Value;

    if (std::is_same<Category, std::random_access_iterator_tag>::value) {
        std::size_t count = std::distance(first, last);
        std::size_t total_size = 0;
        for (It it = first; it != last; ++it) {
            total_size += it->byte_size(); // 必须要求 T 有 byte_size()
        }
        total_size += (count - 1) * this->byte_size();
        result.reserve(total_size);
    }

    result.append(*first);
    ++first;
    for (; first != last; ++first) {
        result.append(*this);  // 加入分隔符
        result.append(*first); // 加入元素
    }

    return result;
}

namespace {
// 上报错误并返回错误字符串
inline void format_error(const std::string& msg) {
    throw std::invalid_argument(msg);
}

// 整数进制枚举
enum IntBase {
    Dec,
    HexLower,
    HexUpper,
    Bin
};

// 格式说明，仅是否有进制及具体类型
struct FormatSpec {
    bool has_base;
    IntBase base;

    FormatSpec() : has_base(false), base(Dec) {}
};

// 解析 "{:x}" 里的 ":x" 部分
inline FormatSpec parse_spec(const std::string& spec) {
    FormatSpec fs;
    if (spec.empty()) return fs;

    if (spec == ":x") {
        fs.has_base = true;
        fs.base = HexLower;
    } else if (spec == ":X") {
        fs.has_base = true;
        fs.base = HexUpper;
    } else if (spec == ":b") {
        fs.has_base = true;
        fs.base = Bin;
    } else if (spec == ":d") {
        fs.has_base = true;
        fs.base = Dec;
    } else {
        format_error("unsupport spec: " + spec);
    }
    return fs;
}

// 拆分 format 字符串为若干文本或 "{...}" 片段
inline std::vector<KAStr> parse_format(const KAStr& fmt) {
    std::vector<KAStr> parts;
    std::size_t pos = 0;
    std::size_t last = 0;
    const std::size_t len = fmt.byte_size();

    while (pos < len) {
        char c = static_cast<char>(fmt[pos]);

        // Handle {{
        if (pos + 1 < len && c == '{' && fmt[pos + 1] == '{') {
            if (pos > last) parts.push_back(fmt.subrange(last, pos));
            parts.push_back(fmt.subrange(pos, pos + 1)); // just one '{'
            pos += 2;
            last = pos;
            continue;
        }

        // Handle }}
        if (pos + 1 < len && c == '}' && fmt[pos + 1] == '}') {
            if (pos > last) parts.push_back(fmt.subrange(last, pos));
            parts.push_back(fmt.subrange(pos, pos + 1)); // just one '}'
            pos += 2;
            last = pos;
            continue;
        }

        // Handle normal field {foo}
        if (c == '{') {
            if (pos > last) parts.push_back(fmt.subrange(last, pos));
            ++pos;
            std::size_t start = pos;
            while (pos < len && fmt[pos] != '}') ++pos;
            if (pos == len) format_error("unmatched '{'");
            parts.push_back(fmt.subrange(start - 1, pos + 1)); // include {}
            ++pos;
            last = pos;
            continue;
        }

        // Handle lone unmatched }
        if (c == '}') {
            format_error("unmatched '}'");
        }

        ++pos;
    }

    // Final remaining text
    if (last < len) {
        parts.push_back(fmt.subrange(last, len));
    }

    return parts;
}

// to_string 重载：vector
template <typename T>
std::string to_string(const std::vector<T>& vec);

// to_string 重载：chrono durations
inline std::string to_string(const std::chrono::nanoseconds& d) {
    return std::to_string(d.count()) + "ns";
}

inline std::string to_string(const std::chrono::milliseconds& d) {
    return std::to_string(d.count()) + "ms";
}

inline std::string to_string(const std::chrono::microseconds& d) {
    return std::to_string(d.count()) + "us";
}

inline std::string to_string(const std::chrono::seconds& d) {
    return std::to_string(d.count()) + "s";
}

inline std::string to_string(const std::chrono::minutes& d) {
    return std::to_string(d.count()) + "min";
}

inline std::string to_string(const std::chrono::hours& d) {
    return std::to_string(d.count()) + "h";
}

inline std::string to_string(const std::chrono::system_clock::time_point& tp) {
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return std::string(buf);
}

// fallback to_string
template <typename T>
std::string to_string(const T& val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

inline std::string to_string(bool b) {
    return b ? "true" : "false";
}

template <typename T>
std::string to_string(const std::vector<T>& vec) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i) oss << ", ";
        oss << to_string(vec[i]);
    }
    oss << "]";
    return oss.str();
}

// 整数格式化，仅对 integral<T> 生效
template <typename T>
typename std::enable_if<std::is_integral<T>::value && ! std::is_same<T, bool>::value, std::string>::type
format_integer(T val, IntBase base) {
    std::ostringstream oss;
    if (base == Dec) {
        oss << val;
    } else if (base == Bin) {
        std::string s;
        typename std::make_unsigned<T>::type u = static_cast<typename std::make_unsigned<T>::type>(val);
        do {
            s += (u & 1) ? '1' : '0';
            u >>= 1;
        } while (u);
        std::reverse(s.begin(), s.end());
        return s;
    } else {
        oss << std::hex;
        if (base == HexUpper) oss << std::uppercase;
        oss << val;
    }
    return oss.str();
}

// 根据是否整型有选择地调用 format_integer 或 to_string
template <typename T>
std::string maybe_format_integer(const T& val, IntBase base, std::true_type) {
    return format_integer(val, base);
}

template <typename T>
std::string maybe_format_integer(const T& val, IntBase /*unused*/, std::false_type) {
    return to_string(val);
}

inline std::string maybe_format_integer(bool val, IntBase, std::true_type) {
    return to_string(val);
}

// 递归展开 tuple，根据 arg_index 调用对应格式化
template <size_t I, typename Tuple>
typename std::enable_if<I == std::tuple_size<Tuple>::value, void>::type
apply_arg(KAString& /* oss */, const std::string& /*part*/, size_t /*arg_index*/, const Tuple& /*tup*/) {
    // 参数越界
    format_error("not enough arguments");
}

template <size_t I, typename Tuple>
    typename std::enable_if < I<std::tuple_size<Tuple>::value, void>::type
                              apply_arg(KAString& out, const std::string& part, size_t arg_index, const Tuple& tup) {
    if (arg_index == I) {
        // 取出 {spec}
        std::string spec = part.substr(1, part.size() - 2);
        FormatSpec fs = parse_spec(spec);
        typedef typename std::remove_reference<typename std::tuple_element<I, Tuple>::type>::type ArgType;
        const ArgType& val = std::get<I>(tup);
        if (fs.has_base) {
            // 整数才做进制，否则当作普通 to_string
            out.append(KAStr(
                maybe_format_integer(val, fs.base, std::integral_constant<bool, std::is_integral<ArgType>::value>())));
        } else {
            out.append(KAStr(to_string(val)));
        }
    } else {
        // 继续下一个
        apply_arg<I + 1>(out, part, arg_index, tup);
    }
}

} // namespace

// 公共接口
template <typename... Args>
KAString KAStr::fmt(const Args&... args) const {
    std::vector<KAStr> parts = parse_format(*this);

    KAString out;
    std::tuple<const Args&...> tup(args...);
    size_t arg_index = 0;

    for (size_t i = 0; i < parts.size(); ++i) {
        KAStr part = parts[i];
        if (part.byte_size() >= 2 && part.front() == '{' && part.back() == '}') {
            apply_arg<0>(out, part, arg_index, tup);
            ++arg_index;
        } else {
            out.append(part);
        }
    }
    return out;
}

inline StyledKAStr KAStr::style() const {
    return StyledKAStr(*this);
}
} // namespace kastring

namespace kastring {
inline KAString StyledKAStr::to_ansi() const {
    KAString out;
    out.reserve(text_.byte_size() + 64);

    out.append("\033[");

    bool first = true;
    auto emit = [&](const KAString& code) {
        if (! first) out.append(";");
        out.append(code);
        first = false;
    };

    if (! fg_code_.empty()) emit(fg_code_);
    if (! bg_code_.empty()) emit(bg_code_);
    if (bold_) emit("1");
    if (italic_) emit("3");
    if (underline_) emit("4");

    out.append("m");
    out.append(text_);
    out.append("\033[0m");
    return out;
}

inline KAString StyledKAStr::own() const {
    return to_ansi();
}

inline std::ostream& operator<<(std::ostream& os, const StyledKAStr& s) {
    return os << s.to_ansi();
}
} // namespace kastring
