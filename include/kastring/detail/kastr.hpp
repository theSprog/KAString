#pragma once

#include "base.hpp"
#include <cstring>
#include <stdexcept>
#include <string>
#include <ostream>
#include <limits>
#include <vector>

namespace kastring {
// ascii-only string, read-only and hasn't ownership
class KAStr {
  public:
    KAStr() : data_() {}

    KAStr(const char* cstr) : data_(reinterpret_cast<const Byte*>(cstr), std::strlen(cstr)) {}

    KAStr(const std::string s) : data_(reinterpret_cast<const Byte*>(s.c_str()), s.size()) {}

    KAStr(const char* ptr, std::size_t len) : data_(reinterpret_cast<const Byte*>(ptr), len) {}

    KAStr(const Byte* ptr, std::size_t len) : data_(ptr, len) {}

    operator std::string() const {
        return std::string(reinterpret_cast<const char*>(data_.data()), data_.size());
    }

    bool empty() const {
        return data_.empty();
    }

    bool is_all_lower() const {
        for (Byte c : data_) {
            if (c < 'a' || c > 'z') return false;
        }
        return true;
    }

    bool is_all_upper() const {
        for (Byte c : data_) {
            if (c < 'A' || c > 'Z') return false;
        }
        return true;
    }

    std::size_t byte_size() const {
        return data_.size();
    }

    const ByteSpan& as_bytes() const {
        return data_;
    }

    const Byte* data() const {
        return data_.data();
    }

    const Byte* begin() const {
        return data_.begin();
    }

    const Byte* end() const {
        return data_.end();
    }

    Byte front() const {
        if(empty()) throw std::out_of_range("KAStr::front() on empty");
        return data_[0];
    }

    Byte back() const {
        if(empty()) throw std::out_of_range("KAStr::back() on empty");
        return data_[byte_size() - 1];
    }

    using const_reverse_iterator = std::reverse_iterator<const Byte*>;

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    uint8_t byte_at(std::size_t idx) const {
        if (idx >= data_.size()) {
            throw std::out_of_range("KAStr::byte_at index out of bounds");
        }
        return data_[idx];
    }

    char operator[](std::size_t idx) const {
        return static_cast<char>(byte_at(idx));
    }

    friend bool operator==(const KAStr& lhs, const KAStr& rhs) {
        if (lhs.byte_size() != rhs.byte_size()) return false;
        if (lhs.byte_size() == 0) return true; // 都是空串
        return lhs.byte_size() == rhs.byte_size() && std::memcmp(lhs.begin(), rhs.begin(), lhs.byte_size()) == 0;
    }

    friend bool operator!=(const KAStr& lhs, const KAStr& rhs) {
        return ! (lhs == rhs);
    }

    friend bool operator==(const KAStr& lhs, const char* rhs) {
        return lhs == KAStr(rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const KAStr& s) {
        return os.write(reinterpret_cast<const char*>(s.begin()), s.byte_size());
    }

    friend bool operator<(const KAStr& a, const KAStr& b) {
        return std::lexicographical_compare(a.as_bytes().begin(),
                                            a.as_bytes().end(),
                                            b.as_bytes().begin(),
                                            b.as_bytes().end());
    }

    friend bool operator>(const KAStr& a, const KAStr& b) {
        return b < a;
    }

    friend bool operator<=(const KAStr& a, const KAStr& b) {
        return ! (b < a);
    }

    friend bool operator>=(const KAStr& a, const KAStr& b) {
        return ! (a < b);
    }

    std::size_t find(const KAStr& substr, bool case_sensitive = true) const {
        if (substr.byte_size() == 0) return 0;
        if (substr.byte_size() > byte_size()) return knpos;
        if (case_sensitive) {


#if defined(__GLIBC__) || defined(__linux__)
            const void* where = ::memmem(begin(), byte_size(), substr.begin(), substr.byte_size());
            if (where) return static_cast<const Byte*>(where) - data_.begin();
#else
            auto* it = std::search(begin(), end(), substr.begin(), substr.end(), [](Byte a, Byte b) { return a == b; });
            if (it != data_.end()) return it - data_.begin();
#endif
        } else {
            const std::size_t haystack_len = byte_size();
            const std::size_t needle_len = substr.byte_size();
            const Byte* haystack = data_.begin();
            const Byte* needle = substr.begin();
            for (std::size_t i = 0; i + needle_len <= haystack_len; ++i) {
                if (ascii_equal(haystack + i, needle, needle_len, false)) return i;
            }
        }
        return knpos;
    }

    std::size_t rfind(const KAStr& substr, bool case_sensitive = true) const {
        const std::size_t haystack_len = byte_size();
        const std::size_t needle_len = substr.byte_size();
        if (needle_len > haystack_len) return knpos;
        for (std::size_t i = haystack_len - needle_len + 1; i-- > 0;) {
            if (ascii_equal(data_.begin() + i, substr.begin(), needle_len, case_sensitive)) {
                return i;
            }
        }
        return knpos;
    }

    bool contains(const KAStr& substr, bool case_sensitive = true) const {
        return find(substr, case_sensitive) != knpos;
    }

    bool starts_with(const KAStr& prefix, bool case_sensitive = true) const {
        if (prefix.byte_size() > byte_size()) return false;
        return ascii_equal(data_.begin(), prefix.begin(), prefix.byte_size(), case_sensitive);
    }

    bool ends_with(const KAStr& suffix, bool case_sensitive = true) const {
        if (suffix.byte_size() > byte_size()) return false;
        return ascii_equal(data_.end() - suffix.byte_size(), suffix.begin(), suffix.byte_size(), case_sensitive);
    }

    std::size_t count(const KAStr& str, bool case_sensitive = true) {
        return count_base(str, false, case_sensitive);
    }

    std::size_t count_overlapping(const KAStr& str, bool case_sensitive = true) {
        return count_base(str, true, case_sensitive);
    }

    KAStr substr(std::size_t start, std::size_t count) const {
        if (start > byte_size()) return KAStr();
        count = std::min(count, byte_size() - start);
        return KAStr(data_.begin() + start, count);
    }

    KAStr substr(std::size_t start) const {
        if (start >= byte_size()) return KAStr();
        return substr(start, byte_size() - start);
    }

    // delim 必定不能是空串, 否则抛出异常
    KAStr substr_until(const KAStr& delim) const {
        if (delim.empty()) {
            throw std::invalid_argument("KAStr::substr_until(): delimiter must not be empty");
        }

        auto vec = split_count(delim, 1);
        return vec[0];
    }

    KAStr substr_from(const KAStr& delim) const {
        if (delim.empty()) {
            throw std::invalid_argument("KAStr::substr_from(): delimiter must not be empty");
        }

        auto vec = split_count(delim, 1);
        if (vec.empty() || vec.size() == 1) return "";
        return vec[1];
    }

    KAStr substr_between(const KAStr& l, const KAStr& r) const {
        if (l.empty() || r.empty()) {
            throw std::invalid_argument("KAStr::substr_between(): delimiter must not be empty");
        }
        auto left = find(l);
        if (left == knpos) return "";
        left += +l.byte_size(); // skip pattern
        auto right = rfind(r);
        if (right == knpos) return "";
        if (left >= right) return "";
        return subrange(left, right);
    }

    // [start, end)
    KAStr subrange(std::size_t start, std::size_t end) const {
        if (start >= end) return KAStr();
        return substr(start, end - start);
    }

    KAStr subrange(std::size_t start) const {
        return subrange(start, byte_size());
    }

    std::pair<KAStr, KAStr> split_at(std::size_t mid) const {
        if (mid > byte_size()) {
            throw std::runtime_error("KAStr::split_at: mid offset " + std::to_string(mid) + " > byte_offset() " +
                                     std::to_string(byte_size()));
        }
        return {KAStr(data_.begin(), mid), KAStr(data_.begin() + mid, byte_size() - mid)};
    }

    std::pair<KAStr, KAStr> split_exclusive_at(std::size_t mid) const {
        if (mid >= byte_size()) {
            throw std::runtime_error("KAStr::split_exclusive_at: mid offset " + std::to_string(mid) +
                                     " > byte_offset() " + std::to_string(byte_size()));
        }
        return {KAStr(data_.begin(), mid), KAStr(data_.begin() + mid + 1, byte_size() - mid - 1)};
    }

    // 如果 delim 是空串则字符级别分割
    std::vector<KAStr> split_count(const KAStr& delim, std::size_t max_splits) const {
        std::vector<KAStr> result;
        std::size_t pos = 0, splits = 0;

        if (delim.empty()) { // 空串, 按字节分割
            std::size_t len = this->byte_size();
            splits = std::min(len, max_splits);

            for (std::size_t i = 0; i < splits; ++i) result.emplace_back(this->begin() + i, 1);

            if (splits < len) result.emplace_back(this->begin() + splits, len - splits);
            return result;
        }

        while (splits < max_splits) {
            std::size_t found = this->subrange(pos, byte_size()).find(delim);
            if (found == knpos) break;

            result.emplace_back(data_.begin() + pos, found);
            pos += found + delim.byte_size();
            ++splits;
        }

        if (pos <= byte_size()) result.emplace_back(data_.begin() + pos, byte_size() - pos);
        return result;
    }

    std::vector<KAStr> rsplit_count(const KAStr& delim, const std::size_t max_splits) const {
        std::vector<KAStr> result;
        std::size_t end = byte_size();
        std::size_t splits = 0;

        if (delim.empty()) {
            std::size_t len = this->byte_size();
            splits = std::min(len, max_splits);
            std::size_t remain = len - splits;

            // 每个字符作为独立分段
            for (std::size_t i = len; i-- > remain;) {
                result.emplace_back(this->begin() + i, 1);
            }
            // 保留前缀部分为剩余段
            if (remain > 0) result.emplace_back(this->begin(), remain);

            return result;
        }

        while (splits < max_splits) {
            std::size_t found = this->subrange(0, end).rfind(delim);
            if (found == knpos) break;

            std::size_t after = found + delim.byte_size();
            result.emplace_back(data_.begin() + after, end - after);
            end = found;
            ++splits;
        }

        result.emplace_back(data_.data(), end);
        return result;
    }

    std::vector<KAStr> split(const KAStr& delim) const {
        return split_count(delim, static_cast<std::size_t>(-1));
    }

    std::vector<KAStr> rsplit(const KAStr& delim) const {
        return rsplit_count(delim, static_cast<std::size_t>(-1));
    }

    std::pair<KAStr, KAStr> split_once(const KAStr& delim) const {
        auto vec = split_count(delim, 1);
        if (vec.size() == 1) return {vec[0], KAStr()};
        return {vec[0], vec[1]};
    }

    std::pair<KAStr, KAStr> rsplit_once(const KAStr& delim) const {
        auto vec = rsplit_count(delim, 1);
        if (vec.size() == 1) return {vec[0], KAStr()};
        return {vec[0], vec[1]};
    }

    std::vector<KAStr> split_whitespace() const {
        std::vector<KAStr> result;
        std::size_t start = 0;
        while (start < byte_size()) {
            while (start < byte_size() && isspace(data_[start])) ++start;
            std::size_t end = start;
            while (end < byte_size() && ! isspace(data_[end])) ++end;
            if (start < end) result.emplace_back(data_.begin() + start, end - start);
            start = end;
        }
        return result;
    }

    std::vector<KAStr> lines() const {
        std::vector<KAStr> result;
        std::size_t start = 0;

        for (std::size_t i = 0; i < byte_size(); ++i) {
            if (data_[i] == '\n') {
                result.emplace_back(data_.begin() + start, i - start);
                start = i + 1;
            } else if (data_[i] == '\r') {
                std::size_t len = i - start;
                if (i + 1 < byte_size() && data_[i + 1] == '\n') {
                    result.emplace_back(data_.begin() + start, len);
                    start = i + 2;
                    ++i; // skip \n
                } else {
                    result.emplace_back(data_.begin() + start, len);
                    start = i + 1;
                }
            }
        }

        if (start < byte_size()) {
            result.emplace_back(data_.begin() + start, byte_size() - start);
        }
        return result;
    }

    KAStr strip_prefix(const KAStr& prefix) const {
        if (starts_with(prefix)) return KAStr(data_.begin() + prefix.byte_size(), byte_size() - prefix.byte_size());
        return *this;
    }

    KAStr strip_suffix(const KAStr& suffix) const {
        if (ends_with(suffix)) return KAStr(data_.begin(), byte_size() - suffix.byte_size());
        return *this;
    }

    KAStr trim_start() const {
        std::size_t start = 0;
        while (start < byte_size() && isspace(data_[start])) ++start;
        return KAStr(data_.begin() + start, byte_size() - start);
    }

    KAStr trim_end() const {
        std::size_t end = byte_size();
        while (end > 0 && isspace(data_[end - 1])) --end;
        return KAStr(data_.begin(), end);
    }

    KAStr trim() const {
        return trim_start().trim_end();
    }

    template <typename Predicate>
    std::vector<KAStr> match(Predicate pred) const {
        std::vector<KAStr> out;
        std::size_t start = 0;
        while (start < byte_size()) {
            while (start < byte_size() && ! pred(data_[start])) ++start;
            std::size_t end = start;
            while (end < byte_size() && pred(data_[end])) ++end;
            if (start < end) out.emplace_back(data_.begin() + start, end - start);
            start = end;
        }
        return out;
    }

    template <typename Predicate>
    std::vector<std::pair<std::size_t, KAStr>> match_indices(Predicate pred) const {
        std::vector<std::pair<std::size_t, KAStr>> out;
        std::size_t start = 0;
        while (start < byte_size()) {
            while (start < byte_size() && ! pred(data_[start])) ++start;
            std::size_t end = start;
            while (end < byte_size() && pred(data_[end])) ++end;
            if (start < end) out.emplace_back(start, KAStr(data_.begin() + start, end - start));
            start = end;
        }
        return out;
    }

    template <typename Predicate>
    KAStr trim_start_matches(Predicate pred) const {
        std::size_t start = 0;
        while (start < byte_size() && pred(data_[start])) ++start;
        return KAStr(data_.begin() + start, byte_size() - start);
    }

    template <typename Predicate>
    KAStr trim_end_matches(Predicate pred) const {
        std::size_t end = byte_size();
        while (end > 0 && pred(data_[end - 1])) --end;
        return KAStr(data_.begin(), end);
    }

    template <typename Predicate>
    KAStr trim_matches(Predicate pred) const {
        return trim_start_matches(pred).trim_end_matches(pred);
    }

    long long to_longlong(int base = 10) const {
        check_base(base);
        return std::stoll(*this, nullptr, base);
    }

    unsigned long long to_ulonglong(int base = 10) const {
        check_base(base);
        return std::stoull(*this, nullptr, base);
    }

    long to_long(int base = 10) const {
        check_base(base);
        return std::stol(*this, nullptr, base);
    }

    unsigned long to_ulong(int base = 10) const {
        check_base(base);
        return std::stoul(*this, nullptr, base);
    }

    int to_int(int base = 10) const {
        check_base(base);
        return std::stoi(*this, nullptr, base);
    }

    unsigned int to_uint(int base = 10) const {
        check_base(base);
        return checked_numeric_cast<unsigned int>(to_ulong(base), "KAString::to_uint");
    }

    short to_short(int base = 10) const {
        check_base(base);
        return checked_numeric_cast<short>(to_int(base), "KAString::to_short");
    }

    unsigned short to_ushort(int base = 10) const {
        check_base(base);
        return checked_numeric_cast<unsigned short>(to_uint(base), "KAString::to_ushort");
    }

    float to_float() const {
        return std::stof(*this);
    }

    double to_double() const {
        return std::stod(*this);
    }

    // kAString-related
    KAString own();
    template <typename It>
    KAString join(It first, It last) const;
    template <typename T>
    KAString join(const std::vector<T>& parts) const;

    template <typename... Args>
    KAString fmt(const Args&... args) const;

    StyledKAStr style() const;

  private:
    char to_lower_ascii(char c) const {
        if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
        return c;
    }

    bool ascii_equal(const Byte* a, const Byte* b, std::size_t n, bool case_sensitive) const {
        if (case_sensitive) {
            return std::memcmp(a, b, n) == 0;
        } else {
            for (std::size_t i = 0; i < n; ++i) {
                if (to_lower_ascii(a[i]) != to_lower_ascii(b[i])) return false;
            }
            return true;
        }
    }

    std::size_t count_base(const KAStr& str, bool allow_overlapping, bool case_sensitive) const {
        if (this->byte_size() == 0 || str.byte_size() == 0 || str.byte_size() > this->byte_size()) return 0;

        std::size_t result = 0;
        std::size_t pos = 0;

        while (pos + str.byte_size() <= this->byte_size()) {
            std::size_t found = this->subrange(pos).find(str, case_sensitive);
            if (found == knpos) break;

            ++result;
            if (allow_overlapping) {
                pos += found + 1;
            } else {
                pos += found + str.byte_size();
            }
        }

        return result;
    }

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

  private:
    // 不拥有所有权
    ByteSpan data_;
};
} // namespace kastring

namespace std {
template <>
struct hash<kastring::KAStr> {
    std::size_t operator()(const kastring::KAStr& s) const {
        return fnv1a_hash(s);
    }
};
} // namespace std
