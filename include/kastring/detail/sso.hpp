#pragma once
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "base.hpp"

namespace kastring {
class SSOBytes {
  public:
    enum : std::size_t {
        HEAP_VIEW_SIZE = sizeof(std::vector<Byte>) + sizeof(uint8_t),
        SSO_CAPACITY = HEAP_VIEW_SIZE - 1
    };

  private:
    enum : uint8_t {
        kHeapFlag = 0x80
    };

    union {
        struct {
            Byte data[SSO_CAPACITY];
            uint8_t len;
        } sso;

        struct {
            std::vector<Byte> vec;
            uint8_t tag;
        } heap;
    };

    void promote_to_heap() {
        if (! is_sso()) return;
        /*
        不能原地构造, 必须通过构造 tmp 再 move ！！！
        标准库里 vector 的实现，往往会先在这块内存里写入它自己的 pointer / size / capacity
        再去把 [first, last) 那段内存拷贝到 heap 上
        由于拷贝源就是刚刚被自己当作“对象存放区”写过元数据的 sso_data
        所以前面那几个字节已经不再是 'a','b','c'…，因此复制到堆上的数据就发生了破坏，看上去就像乱码。
        */
        std::vector<Byte> tmp(sso.data, sso.data + sso.len);
        new (&heap.vec) std::vector<Byte>(std::move(tmp));
        heap.tag = kHeapFlag;
    }

    void init_uncheck(const Byte* p, size_t len) {
        if (len <= SSO_CAPACITY) {
            std::memcpy(sso.data, p, len);
            sso.len = static_cast<uint8_t>(len); // 断言不会超
        } else {
            new (&heap.vec) std::vector<Byte>(p, p + len);
            heap.tag = kHeapFlag;
        }
    }

  public:
    bool is_sso() const {
        const uint8_t tag_value = heap.tag; // 强制访问为 heap 联合成员
        return (tag_value & kHeapFlag) == 0;
    }

    ~SSOBytes() {
        if (! is_sso()) {
            heap.vec.~vector();
        }
    }

    // 默认构造
    SSOBytes() {
        sso.len = 0;
        heap.tag = 0;
    }

    /**
     * @brief 基本构造函数
     * 
     * @warning 绝不不要将 len 设置的大于 p 实际所拥有的内存域长度
     */
    explicit SSOBytes(const Byte* p, size_t len) : SSOBytes() {
        if (p == nullptr) return;

#ifndef NDEBUG
        // LCOV_EXCL_START
        assert(p != nullptr && "ERROR: null pointer passed to SSOBytes");
        std::size_t p_len = std::strlen(reinterpret_cast<const char*>(p));
        if (len > p_len) {
            std::ostringstream oss;
            oss << "ERROR: length( " << std::to_string(len) << " )" << " exceeds input string size( "
                << std::to_string(p_len) << " )";
            std::cerr << oss.str() << std::endl;
            assert(false);
        }
        // LCOV_EXCL_STOP
#endif

        init_uncheck(p, len);
    }

    explicit SSOBytes(Byte ch) {
        sso.data[0] = ch;
        sso.len = 1;
    }

    explicit SSOBytes(const char* cstr) : SSOBytes(cstr, strlen(cstr)) {}

    explicit SSOBytes(const std::string& str) : SSOBytes(str.c_str(), str.size()) {}

    explicit SSOBytes(const char* str_p, std::size_t len) : SSOBytes() {
        if (str_p == nullptr) return;
        init_uncheck(reinterpret_cast<const Byte*>(str_p), len);
    }

    explicit SSOBytes(std::initializer_list<Byte> bs) {
        init_uncheck(bs.begin(), bs.size());
    }

    SSOBytes(const SSOBytes& other) {
        if (other.is_sso()) {
            std::memcpy(sso.data, other.sso.data, other.sso.len);
            sso.len = other.sso.len;
        } else {
            new (&heap.vec) std::vector<Byte>(other.heap.vec);
            heap.tag = kHeapFlag;
        }
    }

    SSOBytes& operator=(const SSOBytes& other) {
        if (this == &other) return *this;
        this->~SSOBytes();
        new (this) SSOBytes(other);
        return *this;
    }

    SSOBytes(SSOBytes&& other) noexcept {
        if (other.is_sso()) {
            std::memcpy(sso.data, other.sso.data, other.sso.len);
            sso.len = other.sso.len;
        } else {
            new (&heap.vec) std::vector<Byte>(std::move(other.heap.vec));
            heap.tag = kHeapFlag;
        }
    }

    SSOBytes& operator=(SSOBytes&& other) noexcept {
        if (this == &other) return *this;
        this->~SSOBytes();
        new (this) SSOBytes(std::move(other));
        return *this;
    }

    bool operator==(const SSOBytes& other) const {
        if (size() != other.size()) return false;
        const Byte* lhs = data();
        const Byte* rhs = other.data();
        return std::equal(lhs, lhs + size(), rhs);
    }

    bool operator!=(const SSOBytes& other) const {
        return ! (*this == other);
    }

    Byte& operator[](std::size_t idx) {
        return is_sso() ? reinterpret_cast<Byte&>(sso.data[idx]) : heap.vec[idx];
    }

    const Byte& operator[](std::size_t idx) const {
        return is_sso() ? reinterpret_cast<const Byte&>(sso.data[idx]) : heap.vec[idx];
    }

    Byte& at(std::size_t idx) {
        if (idx >= size()) throw std::out_of_range("SSOBytes::at()");
        return (*this)[idx];
    }

    std::size_t size() const {
        return is_sso() ? sso.len : heap.vec.size();
    }

    std::size_t capacity() const {
        return is_sso() ? SSO_CAPACITY : heap.vec.capacity();
    }

    bool empty() const {
        return size() == 0;
    }

    Byte* data() {
        return is_sso() ? reinterpret_cast<Byte*>(sso.data) : heap.vec.data();
    }

    const Byte* data() const {
        return is_sso() ? reinterpret_cast<const Byte*>(sso.data) : heap.vec.data();
    }

    Byte& front() {
        return is_sso() ? reinterpret_cast<Byte&>(sso.data[0]) : heap.vec.front();
    }

    const Byte& front() const {
        return is_sso() ? reinterpret_cast<const Byte&>(sso.data[0]) : heap.vec.front();
    }

    Byte& back() {
        return is_sso() ? reinterpret_cast<Byte&>(sso.data[sso.len - 1]) : heap.vec.back();
    }

    const Byte& back() const {
        return is_sso() ? reinterpret_cast<const Byte&>(sso.data[sso.len - 1]) : heap.vec.back();
    }

    void clear() {
        if (is_sso()) {
            sso.len = 0;
        } else {
            heap.vec.clear();
        }
    }

    void push_back(Byte byte) {
        if (is_sso()) {
            if (sso.len < SSO_CAPACITY) {
                sso.data[sso.len++] = byte;
            } else {
                promote_to_heap();
                heap.vec.push_back(byte);
            }
        } else {
            heap.vec.push_back(byte);
        }
    }

    void pop_back() {
        if (empty()) {
            throw std::runtime_error("SSOBytes::pop_back(): pop on empty SSO");
        }
        if (is_sso()) {
            --sso.len;
        } else {
            heap.vec.pop_back();
        }
    }

    void append(const Byte* src, std::size_t len) {
        if (len == 0) return;

        if (is_sso() && sso.len + len <= SSO_CAPACITY) {
            std::memcpy(sso.data + sso.len, src, len);
            sso.len += static_cast<uint8_t>(len);
        } else {
            promote_to_heap();
            heap.vec.insert(heap.vec.end(), src, src + len);
        }
    }

    void append(const char* cstr) {
        append(reinterpret_cast<const Byte*>(cstr), std::strlen(cstr));
    }

    void append(std::initializer_list<Byte> list) {
        append(list.begin(), list.size());
    }

    void append(const std::string& str) {
        append(reinterpret_cast<const Byte*>(str.data()), str.size());
    }

    void insert(std::size_t pos, Byte byte) {
        if (! (pos <= size())) {
            throw std::out_of_range("SSOBytes::insert()");
        }
        if (is_sso() && sso.len < SSO_CAPACITY) {
            for (std::size_t i = sso.len; i > pos; --i) sso.data[i] = sso.data[i - 1];
            sso.data[pos] = byte;
            ++sso.len;
        } else {
            promote_to_heap();
            heap.vec.insert(std::next(heap.vec.begin(), static_cast<std::ptrdiff_t>(pos)), byte);
        }
    }

    void resize(std::size_t n, Byte val = 0) {
        if (! is_sso()) {
            heap.vec.resize(n, val);
            return;
        }

        // is sso
        if (n <= SSO_CAPACITY) { // 如果 resize 后仍然在栈上
            if (n > sso.len) {   // 扩展
                std::fill(sso.data + sso.len, sso.data + n, val);
            }
            sso.len = static_cast<uint8_t>(n);
        } else { // 扩展到堆上, 进入 heap 模式
            promote_to_heap();
            heap.vec.resize(n, val); // 使用 vector 的 resize
        }
    }

    void reserve(std::size_t n) {
        if (! is_sso()) {
            heap.vec.reserve(n);
        } else if (n > SSO_CAPACITY) {
            promote_to_heap();
            heap.vec.reserve(n);
        }
    }

    void erase(std::size_t pos) {
        if (is_sso()) {
            if (! (pos < sso.len)) {
                throw std::out_of_range("SSOBytes::erase()");
            }
            for (std::size_t i = pos; i + 1 < sso.len; ++i) sso.data[i] = sso.data[i + 1];
            --sso.len;
        } else {
            heap.vec.erase(std::next(heap.vec.begin(), static_cast<std::ptrdiff_t>(pos)));
        }
    }

    void erase(std::size_t from, std::size_t to) {
        if (from > to || to > size()) {
            throw std::out_of_range("SSOBytes::erase(from, to)");
        }

        std::size_t count = to - from;
        if (count == 0) return; // nothing to erase

        if (is_sso()) {
            for (std::size_t i = from; i + count < sso.len; ++i) {
                sso.data[i] = sso.data[i + count];
            }
            sso.len -= static_cast<Byte>(count);
        } else {
            heap.vec.erase(heap.vec.begin() + static_cast<std::ptrdiff_t>(from),
                           heap.vec.begin() + static_cast<std::ptrdiff_t>(to));
        }
    }

    void shrink_to_fit() {
        if (! is_sso()) {
            heap.vec.shrink_to_fit();
        }
    }

    void swap(SSOBytes& other) noexcept {
        if (this == &other) return;

        if (is_sso() && other.is_sso()) { // 两者都是 SSO
            std::array<Byte, SSO_CAPACITY> tmp;
            std::uint8_t tmp_len = sso.len;

            std::memcpy(tmp.data(), sso.data, sso.len);
            std::memcpy(sso.data, other.sso.data, other.sso.len);
            std::memcpy(other.sso.data, tmp.data(), tmp_len);

            std::swap(sso.len, other.sso.len);
        } else if (! is_sso() && ! other.is_sso()) { // 两者都不是 SSO
            heap.vec.swap(other.heap.vec);
        } else {
            SSOBytes tmp = std::move(*this);
            *this = std::move(other);
            other = std::move(tmp);
        }
    }

    template <typename It>
    void insert(std::size_t pos, It first, It last) {
        std::size_t count = static_cast<std::size_t>(std::distance(first, last));
        if(count == 0) return;
        if (! (pos <= size())) {
            throw std::out_of_range("SSOBytes::insert<It>()");
        }

        if (is_sso() && sso.len + count <= SSO_CAPACITY) {
            for (std::size_t i = sso.len; i > pos; --i) sso.data[i + count - 1] = sso.data[i - 1];
            for (std::size_t i = 0; i < count; ++i) sso.data[pos + i] = static_cast<Byte>(*first++);
            sso.len += static_cast<Byte>(count);
        } else {
            promote_to_heap();
            heap.vec.insert(heap.vec.begin() + pos, first, last);
        }
    }

    template <typename Predicate>
    void remove_if(Predicate pred) {
        if (is_sso()) {
            std::size_t write = 0;
            for (std::size_t read = 0; read < sso.len; ++read) {
                char c = static_cast<char>(sso.data[read]);
                if (! pred(c)) {
                    sso.data[write++] = sso.data[read];
                }
            }
            sso.len = static_cast<Byte>(write);
        } else {
            auto& vec = heap.vec;
            vec.erase(std::remove_if(vec.begin(), vec.end(), pred), vec.end());
        }
    }

    template <typename It>
    void assign(It begin, It end) {
        std::size_t n = static_cast<std::size_t>(std::distance(begin, end));
        if (is_sso() && n <= SSO_CAPACITY) {
            for (std::size_t i = 0; i < n; ++i) sso.data[i] = static_cast<Byte>(*(begin++));
            sso.len = static_cast<Byte>(n);
        } else {            // 不是 SSO, 或者 n 过大, 总之都要进入堆模式
            if (is_sso()) { //
                promote_to_heap();
            }
            heap.vec.assign(begin, end);
        }
    }

    void assign(std::initializer_list<Byte> list) {
        assign(list.begin(), list.end());
    }

    using iterator = Byte*;
    using const_iterator = const Byte*;

    friend void swap(SSOBytes& lhs, SSOBytes& rhs) noexcept {
        lhs.swap(rhs);
    }

    iterator begin() {
        return data();
    }

    iterator end() {
        return data() + size();
    }

    const_iterator begin() const {
        return data();
    }

    const_iterator end() const {
        return data() + size();
    }

    const_iterator cbegin() const {
        return begin();
    }

    const_iterator cend() const {
        return end();
    }
};
} // namespace kstring
