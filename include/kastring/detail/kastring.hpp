#pragma once

#include "base.hpp"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <ostream>
#include <vector>

#include "./kastr.hpp"
#include "./sso.hpp"

namespace kastring {
class KAString {
  public:
    KAString() : data_() {}

    KAString(const char* cstr) : data_(cstr) {}

    KAString(const std::string& str) : data_(str) {}

    KAString(const char* ptr, std::size_t len) : data_(ptr, len) {}

    KAString(const Byte* ptr, std::size_t len) : data_(ptr, len) {}

    KAString(std::initializer_list<Byte> vec) : data_(vec) {}

    KAString(const KAStr& kastr) : data_(kastr.data(), kastr.byte_size()) {}

    // 拷贝构造/赋值, 移动构造/赋值, 析构
    KAString(const KAString&) = default;
    KAString& operator=(const KAString&) = default;
    KAString(KAString&&) noexcept = default;
    KAString& operator=(KAString&&) noexcept = default;
    ~KAString() = default;

    operator std::string() const {
        return std::string(reinterpret_cast<const char*>(data_.data()), data_.size());
    }

    // 隐式转换 KAStr, 也可以通过 as_kastr() 显式转换
    operator KAStr() const {
        return KAStr(data_.data(), data_.size());
    }

    friend std::ostream& operator<<(std::ostream& os, const KAString& s) {
        return os.write(reinterpret_cast<const char*>(s.data_.data()), s.data_.size());
    }

    char operator[](std::size_t idx) const {
        return static_cast<char>(byte_at(idx));
    }

    char& operator[](std::size_t idx) {
        if (idx >= data_.size()) {
            throw std::out_of_range("KAString::operator[] index out of bounds");
        }
        return reinterpret_cast<char&>(data_[idx]);
    }

    friend bool operator==(const KAString& lhs, const KAString& rhs) {
        return lhs.as_kastr() == rhs.as_kastr();
    }

    friend bool operator!=(const KAString& lhs, const KAString& rhs) {
        return ! (lhs == rhs);
    }

    // KAString == const char*
    friend bool operator==(const KAString& lhs, const char* rhs) {
        if (rhs == nullptr) return lhs.empty();
        return lhs.as_kastr() == KAStr(rhs);
    }

    friend bool operator==(const char* lhs, const KAString& rhs) {
        return rhs == lhs;
    }

    friend bool operator==(const KAString& lhs, const std::string& rhs) {
        return lhs.as_kastr() == KAStr(rhs);
    }

    friend bool operator==(const std::string& lhs, const KAString& rhs) {
        return rhs == lhs;
    }

    friend bool operator!=(const KAString& lhs, const char* rhs) {
        return ! (lhs == rhs);
    }

    friend bool operator!=(const char* lhs, const KAString& rhs) {
        return ! (lhs == rhs);
    }

    friend bool operator!=(const KAString& lhs, const std::string& rhs) {
        return ! (lhs == rhs);
    }

    friend bool operator!=(const std::string& lhs, const KAString& rhs) {
        return ! (lhs == rhs);
    }

    // KAString + KAString
    friend KAString operator+(const KAString& lhs, const KAString& rhs) {
        KAString result;
        result.data_.reserve(lhs.byte_size() + rhs.byte_size());
        result.append(lhs);
        result.append(rhs);
        return result;
    }

    // KAString + const char*
    friend KAString operator+(const KAString& lhs, const char* rhs) {
        KAString result = lhs;
        result.append(rhs);
        return result;
    }

    // const char* + KAString
    friend KAString operator+(const char* lhs, const KAString& rhs) {
        KAString result(lhs);
        result.append(rhs);
        return result;
    }

    // KAString + std::string
    friend KAString operator+(const KAString& lhs, const std::string& rhs) {
        KAString result = lhs;
        result.append(rhs.data(), rhs.size());
        return result;
    }

    // std::string + KAString
    friend KAString operator+(const std::string& lhs, const KAString& rhs) {
        KAString result(lhs);
        result.append(rhs);
        return result;
    }

    // KAString + char
    friend KAString operator+(const KAString& lhs, char ch) {
        KAString result = lhs;
        result.append(ch);
        return result;
    }

    // char + KAString
    friend KAString operator+(char ch, const KAString& rhs) {
        KAString result;
        result.append(ch);
        result.append(rhs);
        return result;
    }

    KAString& operator+=(const KAString& rhs) {
        this->append(rhs);
        return *this;
    }

    KAString& operator+=(const KAStr& rhs) {
        this->append(rhs);
        return *this;
    }

    KAString& operator+=(const char* rhs) {
        this->append(rhs);
        return *this;
    }

    KAString& operator+=(const std::string& rhs) {
        this->append(rhs.data(), rhs.size());
        return *this;
    }

    KAString& operator+=(char ch) {
        this->append(ch);
        return *this;
    }

    int compare(const KAString& other) const {
        if (this->byte_size() < other.byte_size()) return -1;
        if (this->byte_size() > other.byte_size()) return 1;
        if (this->byte_size() == 0) return 0; // 都是空串

        const std::size_t n = std::min(this->byte_size(), other.byte_size());
        return std::memcmp(this->data(), other.data(), n);
    }

    bool operator<(const KAString& other) const {
        return this->compare(other) < 0;
    }

    bool empty() const {
        return data_.empty();
    }

    std::size_t byte_size() const {
        return data_.size();
    }

    std::size_t char_size() const {
        return data_.size(); // ASCII only
    }

    std::size_t capacity() const {
        return data_.capacity();
    }

    void clear() {
        return data_.clear();
    }

    const Byte* data() const {
        return data_.data();
    }

    Byte* data() {
        return data_.data();
    }

    const Byte* begin() const {
        return data_.data();
    }

    Byte* begin() {
        return data_.data();
    }

    const Byte* end() const {
        return data_.data() + data_.size();
    }

    Byte* end() {
        return data_.data() + data_.size();
    }

    using const_reverse_iterator = std::reverse_iterator<const Byte*>;

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    using reverse_iterator = std::reverse_iterator<Byte*>;

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    uint8_t byte_at(std::size_t idx) const {
        if (idx >= data_.size()) {
            throw std::out_of_range("KAString::byte_at index out of bounds");
        }
        return data_[idx];
    }

    uint8_t char_at(std::size_t idx) const {
        return byte_at(idx); // ascii-only byte == char
    }

    void reserve(size_t cap) {
        data_.reserve(cap);
    }

    void reverse() {
        std::reverse(begin(), end());
    }

    void resize(size_t new_size) {
        data_.resize(new_size, 0);
    }

    void resize(size_t new_size, const Byte& b) {
        data_.resize(new_size, b);
    }

    KAStr as_kastr() const {
        return KAStr(data_.data(), data_.size());
    }

    std::size_t find(const KAStr& substr, bool case_sensitive = true) const {
        return as_kastr().find(substr, case_sensitive);
    }

    std::size_t rfind(const KAStr& substr, bool case_sensitive = true) const {
        return as_kastr().rfind(substr, case_sensitive);
    }

    bool contains(const KAStr& substr, bool case_sensitive = true) const {
        return as_kastr().contains(substr, case_sensitive);
    }

    bool starts_with(const KAStr& prefix, bool case_sensitive = true) const {
        return as_kastr().starts_with(prefix, case_sensitive);
    }

    bool ends_with(const KAStr& suffix, bool case_sensitive = true) const {
        return as_kastr().ends_with(suffix, case_sensitive);
    }

    std::size_t count(const KAStr& str, bool case_sensitive = true) {
        return as_kastr().count(str, case_sensitive);
    }

    std::size_t count_overlapping(const KAStr& str, bool case_sensitive = true) {
        return as_kastr().count_overlapping(str, case_sensitive);
    }

    KAStr substr(std::size_t start, std::size_t count) const {
        return as_kastr().substr(start, count);
    }

    KAStr substr(std::size_t start) const {
        return as_kastr().substr(start);
    }

    KAStr substr_until(const KAStr& delim) const {
        return as_kastr().substr_until(delim);
    }

    KAStr substr_from(const KAStr& delim) const {
        return as_kastr().substr_from(delim);
    }

    KAStr substr_between(const KAStr& l, KAStr r) const {
        return as_kastr().substr_between(l, r);
    }

    KAStr subrange(std::size_t start, std::size_t end) const {
        return as_kastr().subrange(start, end);
    }

    KAStr subrange(std::size_t start) const {
        return as_kastr().subrange(start);
    }

    std::pair<KAStr, KAStr> split_at(std::size_t mid) const {
        return as_kastr().split_at(mid);
    }

    std::pair<KAStr, KAStr> split_exclusive_at(std::size_t mid) const {
        return as_kastr().split_exclusive_at(mid);
    }

    std::vector<KAStr> split_count(const KAStr& delim, std::size_t max_splits) const {
        return as_kastr().split_count(delim, max_splits);
    }

    std::vector<KAStr> rsplit_count(const KAStr& delim, std::size_t max_splits) const {
        return as_kastr().rsplit_count(delim, max_splits);
    }

    std::vector<KAStr> split(const KAStr& delim) const {
        return as_kastr().split(delim);
    }

    std::vector<KAStr> rsplit(const KAStr& delim) const {
        return as_kastr().rsplit(delim);
    }

    std::pair<KAStr, KAStr> split_once(const KAStr& delim) const {
        return as_kastr().split_once(delim);
    }

    std::pair<KAStr, KAStr> rsplit_once(const KAStr& delim) const {
        return as_kastr().rsplit_once(delim);
    }

    std::vector<KAStr> split_whitespace() const {
        return as_kastr().split_whitespace();
    }

    std::vector<KAStr> lines() const {
        return as_kastr().lines();
    }

    KAStr strip_prefix(const KAStr& prefix) const {
        return as_kastr().strip_prefix(prefix);
    }

    KAStr strip_suffix(const KAStr& suffix) const {
        return as_kastr().strip_suffix(suffix);
    }

    KAStr trim_start() const {
        return as_kastr().trim_start();
    }

    KAStr trim_end() const {
        return as_kastr().trim_end();
    }

    KAStr trim() const {
        return as_kastr().trim();
    }

    template <typename Predicate>
    std::vector<KAStr> match(Predicate pred) const {
        return as_kastr().match(pred);
    }

    template <typename Predicate>
    std::vector<std::pair<std::size_t, KAStr>> match_indices(Predicate pred) const {
        return as_kastr().match_indices(pred);
    }

    template <typename Predicate>
    KAStr trim_start_matches(Predicate pred) const {
        return as_kastr().trim_start_matches(pred);
    }

    template <typename Predicate>
    KAStr trim_end_matches(Predicate pred) const {
        return as_kastr().trim_end_matches(pred);
    }

    template <typename Predicate>
    KAStr trim_matches(Predicate pred) const {
        return as_kastr().trim_matches(pred);
    }

    KAString join(const std::vector<KAStr>& vec) const {
        return as_kastr().join(vec);
    }

    void append(char ch) {
        data_.push_back(static_cast<Byte>(ch));
    }

    void append(const char* cstr) {
        if (cstr == nullptr) return;
        const std::size_t len = std::strlen(cstr);
        append(KAStr(cstr, len));
    }

    void append(const char* ptr, std::size_t len) {
        if (ptr == nullptr || len == 0) return;
        data_.append(KAStr(ptr, len));
    }

    void append(const KAString& other) {
        data_.append(other.as_kastr());
    }

    void append(const KAStr& strview) {
        data_.append(strview.begin(), strview.byte_size());
    }

    void chop(std::size_t n) {
        if (n >= data_.size()) {
            data_.clear(); // chop 全部或更多时清空
        } else {
            data_.resize(data_.size() - n);
        }
    }

    KAString chopped(std::size_t n) {
        KAString result;
        if (n >= data_.size()) {
            result.data_.clear(); // 全部裁掉，返回空串
        } else {
            result.data_.assign(data_.begin(), data_.end() - n);
        }
        return result;
    }

    KAString& fill(char ch, std::size_t size = static_cast<std::size_t>(-1)) {
        if (size == static_cast<std::size_t>(-1)) {
            // 保持当前大小
            std::fill(data_.begin(), data_.end(), ch);
        } else {
            // 先调整大小，再填充
            data_.resize(size);
            std::fill(data_.begin(), data_.end(), ch);
        }
        return *this;
    }

    KAString& prepend(const KAStr& str) {
        data_.insert(0, str.begin(), str.end());
        return *this;
    }

    KAString& remove(const KAStr& str, bool case_sensitive = true) {
        if (str.empty() || str.byte_size() > data_.size()) return *this;

        std::size_t pos = 0;
        while (pos <= byte_size() - str.byte_size()) {
            std::size_t found = KAStr(begin() + pos, byte_size() - pos).find(str, case_sensitive);
            if (found == knpos) break;

            data_.erase(pos + found, pos + found + str.byte_size());
            pos += found;
        }

        return *this;
    }

    KAString& remove_at(std::size_t pos) {
        if (pos >= byte_size()) throw std::out_of_range("KAString::remove_at()");
        data_.erase(pos);
        return *this;
    }

    KAString& remove_first() {
        if (empty()) throw std::out_of_range("KAString::remove_first()");
        data_.erase(0);
        return *this;
    }

    KAString& remove_last() {
        if (empty()) throw std::out_of_range("KAString::remove_last()");
        data_.pop_back();
        return *this;
    }

    KAString repeated(int times) const {
        KAString result;
        if (times <= 0 || empty()) return result;

        result.data_.reserve(byte_size() * times);
        for (int i = 0; i < times; ++i) {
            result.data_.insert(result.byte_size(), data_.begin(), data_.end());
        }
        return result;
    }

    KAString& replace_count(const KAStr& before,
                            const KAStr& after,
                            std::size_t max_replace = static_cast<std::size_t>(-1),
                            bool case_sensitive = true) {
        if (before.empty()) return *this;
        if (before == after || max_replace == 0) return *this;

        std::size_t replaced = 0;
        std::size_t pos = 0;

        while (pos + before.byte_size() <= byte_size()) {
            std::size_t found = this->as_kastr().subrange(pos).find(before, case_sensitive);
            if (found == knpos) break;

            std::size_t abs_pos = pos + found;
            this->replace(abs_pos, before.byte_size(), after);

            ++replaced;
            if (replaced >= max_replace) break;

            pos = abs_pos + after.byte_size();
        }

        return *this;
    }

    KAString& rreplace_count(const KAStr& before,
                             const KAStr& after,
                             std::size_t max_replace = static_cast<std::size_t>(-1),
                             bool case_sensitive = true) {
        if (before.empty()) return *this;
        if (before == after || max_replace == 0) return *this;

        std::size_t replaced = 0;
        std::size_t start = byte_size() - before.byte_size();

        while (start + 1 > 0 && replaced < max_replace) {
            std::size_t found = this->as_kastr().subrange(0, start + before.byte_size()).rfind(before, case_sensitive);
            if (found == knpos) break;

            this->replace(found, before.byte_size(), after);
            ++replaced;

            if (found == 0) break;
            start = found - 1; // 向前推进
        }

        return *this;
    }

    KAString& replace_nth(const KAStr& before, const KAStr& after, std::size_t nth, bool case_sensitive = true) {
        if (before.empty()) return *this;
        if (before == after) return *this;

        std::size_t count = 0;
        std::size_t pos = 0;

        while (pos + before.byte_size() <= byte_size()) {
            std::size_t found = this->as_kastr().subrange(pos).find(before, case_sensitive);
            if (found == knpos) break;

            std::size_t abs_pos = pos + found;
            if (count == nth) {
                return this->replace(abs_pos, before.byte_size(), after);
            }

            ++count;
            pos = abs_pos + before.byte_size(); // move past match
        }

        return *this; // nth match not found
    }

    KAString& rreplace_nth(const KAStr& before, const KAStr& after, std::size_t nth, bool case_sensitive = true) {
        if (before.empty()) return *this;
        if (before == after) return *this;

        std::vector<std::size_t> matches;
        std::size_t search_end = byte_size();

        while (search_end >= before.byte_size()) {
            std::size_t found = this->as_kastr().subrange(0, search_end).rfind(before, case_sensitive);
            if (found == knpos) break;

            matches.push_back(found);
            if (found == 0) break;
            search_end = found;
        }

        if (nth < matches.size()) {
            return this->replace(matches[nth], before.byte_size(), after);
        }

        return *this;
    }

    // 将 pos 处 len 个字符替换为指定内容
    KAString& replace(std::size_t pos, std::size_t len, const KAStr& after) {
        if (pos >= byte_size() || pos + len > byte_size()) {
            throw std::out_of_range("KAString::replace(pos, len, after)");
        }

        if (len == after.byte_size()) {
            // 长度一致，可以直接覆盖（就地替换）
            for (std::size_t i = 0; i < len; ++i) {
                data_[pos + i] = after[i];
            }
        } else {
            data_.erase(pos, pos + len);
            data_.insert(pos, after.begin(), after.end());
        }
        return *this;
    }

    // 将所有指定内容替换为另一个内容
    KAString& replace_all(const KAStr& before, const KAStr& after, bool case_sensitive = true) {
        return replace_count(before, after, static_cast<std::size_t>(-1), case_sensitive);
    }

    KAString& replace_first(const KAStr& before, const KAStr& after, bool case_sensitive = true) {
        return replace_count(before, after, 1, case_sensitive);
    }

    KAString& replace_last(const KAStr& before, const KAStr& after, bool case_sensitive = true) {
        return rreplace_count(before, after, 1, case_sensitive);
    }

    KAString ljust(std::size_t width, char fill = ' ', bool truncate = true) const {
        std::size_t cur = byte_size();
        if (cur >= width) {
            return truncate ? KAString(this->substr(0, width)) : *this;
        }

        KAString result = *this;
        std::vector<Byte> padding(width - cur, static_cast<Byte>(fill));
        result.data_.insert(result.byte_size(), padding.begin(), padding.end());
        return result;
    }

    KAString rjust(std::size_t width, char fill = ' ', bool truncate = true) const {
        std::size_t cur = byte_size();
        if (cur >= width) {
            return truncate ? KAString(this->substr(cur - width)) : *this;
        }

        KAString result;
        std::vector<Byte> padding(width - cur, static_cast<Byte>(fill));
        result.data_.insert(0, padding.begin(), padding.end());
        result.data_.insert(result.byte_size(), this->begin(), this->end());
        return result;
    }

    KAString center(std::size_t width, char fill = ' ') const {
        std::size_t len = byte_size();
        if (len >= width) return *this; // 长度超出就返回

        std::size_t total_pad = width - len;
        std::size_t left_pad = total_pad / 2; // left 总会相对右边可能少一点(由于整数除法截断)
        std::size_t right_pad = total_pad - left_pad;

        std::vector<Byte> left(left_pad, static_cast<Byte>(fill));
        std::vector<Byte> right(right_pad, static_cast<Byte>(fill));

        KAString result;
        result.data_.insert(result.byte_size(), left.begin(), left.end());
        result.data_.insert(result.byte_size(), this->begin(), this->end());
        result.data_.insert(result.byte_size(), right.begin(), right.end());

        return result;
    }

    static KAString from_num(int n, int base = 10) {
        char buf[64];
        const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";

        if (base < 2 || base > 36)
            throw std::invalid_argument("KAString::fromNum(int, int), base must meet: 2 <= base <= 36, got " +
                                        std::to_string(base));

        char* p = buf + sizeof(buf);
        bool negative = (n < 0);
        unsigned int u = negative ? -static_cast<unsigned int>(n) : static_cast<unsigned int>(n);

        do {
            *--p = digits[u % base];
            u /= base;
        } while (u > 0);

        if (negative) *--p = '-';

        return KAString(p, static_cast<std::size_t>((buf + sizeof(buf)) - p));
    }

    static KAString from_num(double d, char fmt = 'g', int precision = 6) {
        if (fmt != 'f' && fmt != 'e' && fmt != 'g') {
            throw std::invalid_argument(
                "KAString::fromNum(double, char, int), fmt only support `f`, `e` and `g`, got " + std::to_string(fmt));
        }

        char fmt_buf[16];
        char buf[128];

        std::snprintf(fmt_buf, sizeof(fmt_buf), "%%.%d%c", precision, fmt);
        std::snprintf(buf, sizeof(buf), fmt_buf, d);
        return KAString(buf);
    }

    long long to_longlong(int base = 10) const {
        return as_kastr().to_longlong(base);
    }

    unsigned long long to_ulonglong(int base = 10) const {
        return as_kastr().to_ulonglong(base);
    }

    long to_long(int base = 10) const {
        return as_kastr().to_long(base);
    }

    unsigned long to_ulong(int base = 10) const {
        return as_kastr().to_ulong(base);
    }

    int to_int(int base = 10) const {
        return as_kastr().to_int(base);
    }

    unsigned int to_uint(int base = 10) const {
        return as_kastr().to_uint(base);
    }

    short to_short(int base = 10) const {
        return as_kastr().to_short(base);
    }

    unsigned short to_ushort(int base = 10) const {
        return as_kastr().to_ushort(base);
    }

    float to_float() const {
        return as_kastr().to_float();
    }

    double to_double() const {
        return as_kastr().to_double();
    }

    KAString to_upper() const {
        KAString res = *this;
        res.upper_self();
        return res;
    }

    KAString to_lower() const {
        KAString res = *this;
        res.lower_self();
        return res;
    }

    void upper_self() {
        for (auto& ch : data_) {
            if (ch >= 'a' && ch <= 'z') ch -= 32;
        }
    }

    void lower_self() {
        for (auto& ch : data_) {
            if (ch >= 'A' && ch <= 'Z') ch += 32;
        }
    }

    KAString simplified() const {
        return KAStr(" ").join(as_kastr().trim().split_whitespace());
    }

    template <typename... Args>
    KAString fmt(const Args&... args) const {
        return as_kastr().fmt(args...);
    }

    template <typename Predicate>
    KAString& remove_if(Predicate pred) {
        data_.remove_if(pred);
        return *this;
    }

    template <typename Predicate>
    KAString&
    replace_char_if(Predicate pred, const KAStr& replacement, std::size_t max_replace = static_cast<std::size_t>(-1)) {
        static_assert(std::is_convertible<decltype(std::declval<Predicate>()(char{})), bool>::value,
                      "replace_char_if expects predicate of type bool(char)");
        if (max_replace == 0) return *this;

        std::size_t replaced = 0;
        std::size_t i = 0;
        while (i < data_.size()) {
            char c = static_cast<char>(data_[i]);

            if (pred(c)) {
                if (replacement.byte_size() == 1) {
                    data_[i] = replacement[0]; // overwrite 1 byte
                    ++i;                       // move to next char
                } else {
                    data_.erase(i, i + 1); // remove 1 byte
                    data_.insert(i, replacement.begin(), replacement.end());
                    i += replacement.byte_size();
                }

                ++replaced;
                if (replaced >= max_replace) break;
            } else {
                ++i;
            }
        }

        return *this;
    }

    template <typename Predicate>
    KAString& replace_groups_if(Predicate pred,
                                const KAStr& replacement,
                                std::size_t max_replace = static_cast<std::size_t>(-1)) {
        static_assert(std::is_convertible<decltype(std::declval<Predicate>()(char{})), bool>::value,
                      "replace_char_if expects predicate of type bool(char)");
        if (max_replace == 0) return *this;

        std::size_t replaced = 0;
        std::size_t i = 0;
        while (i < data_.size()) {
            // 找到第一个满足 pred 的位置
            if (! pred(static_cast<char>(data_[i]))) {
                ++i;
                continue;
            }

            // 开始一段 group
            std::size_t start = i;
            while (i < data_.size() && pred(static_cast<char>(data_[i]))) {
                ++i;
            }

            // 区间 [start, i) 是一个 group
            std::size_t group_len = i - start;
            if (group_len == replacement.byte_size()) {
                std::copy(replacement.begin(), replacement.end(), data_.begin() + start);
            } else {
                data_.erase(start, i);
                data_.insert(start, replacement.begin(), replacement.end());
            }

            ++replaced;
            if (replaced >= max_replace) break;

            i = start + replacement.byte_size(); // 跳过刚插入的
        }

        return *this;
    }

    template <typename Predicate>
    KAString&
    rreplace_char_if(Predicate pred, const KAStr& replacement, std::size_t max_replace = static_cast<std::size_t>(-1)) {
        static_assert(std::is_convertible<decltype(std::declval<Predicate>()(char{})), bool>::value,
                      "replace_char_if_rev expects predicate of type bool(char)");

        if (max_replace == 0) return *this;

        std::size_t replaced = 0;
        std::size_t i = data_.size();

        while (i-- > 0) {
            char c = static_cast<char>(data_[i]);

            if (pred(c)) {
                if (replacement.byte_size() == 1) {
                    data_[i] = replacement[0];
                } else {
                    data_.erase(i, i + 1);
                    data_.insert(i, replacement.begin(), replacement.end());
                }

                ++replaced;
                if (replaced >= max_replace) break;

                // 如果 replacement 比 1 多，i 已指向替换后的末尾前，继续向前处理
            }
        }

        return *this;
    }

    template <typename Predicate>
    KAString& rreplace_groups_if(Predicate pred,
                                 const KAStr& replacement,
                                 std::size_t max_replace = static_cast<std::size_t>(-1)) {
        static_assert(std::is_convertible<decltype(std::declval<Predicate>()(char{})), bool>::value,
                      "rreplace_groups_if expects predicate of type bool(char)");

        if (max_replace == 0) return *this;

        std::size_t replaced = 0;
        std::size_t i = data_.size();

        while (i-- > 0) {
            if (! pred(static_cast<char>(data_[i]))) {
                continue;
            }

            std::size_t end = i + 1;
            while (pred(static_cast<char>(data_[i]))) {
                if (i == 0) break;
                --i;
            }

            std::size_t start = pred(static_cast<char>(data_[i])) ? i : i + 1;

            std::size_t group_len = end - start;
            if (group_len == replacement.byte_size()) {
                std::copy(replacement.begin(), replacement.end(), data_.begin() + start);
            } else {
                data_.erase(start, end);
                data_.insert(start, replacement.begin(), replacement.end());
            }

            ++replaced;
            if (replaced >= max_replace) break;

            if (start == 0) break;
            i = start - 1;
        }

        return *this;
    }


  private:
    SSOBytes data_;

    template <typename Target, typename Source>
    Target checked_numeric_cast(Source value, const char* context) const {
        if (value < static_cast<Source>(std::numeric_limits<Target>::min()) ||
            value > static_cast<Source>(std::numeric_limits<Target>::max()))
            throw std::out_of_range(std::string(context) + ": out of range");
        return static_cast<Target>(value);
    }

    void check_base(int base) const {
        if (2 <= base && base <= 36) return;

        throw std::invalid_argument("base must be in [2, 36], but got " + std::to_string(base));
    }
};
} // namespace kastring

namespace std {
template <>
struct hash<kastring::KAString> {
    std::size_t operator()(const kastring::KAString& s) const {
        return std::hash<kastring::KAStr>()(s);
    }
};
} // namespace std

