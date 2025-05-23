#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "../../include/kastring/detail/sso.hpp"

using kastring::Byte;
using kastring::SSOBytes;

TEST_CASE("Default constructor should initialize empty SSO") {
    SSOBytes s;
    CHECK(s.size() == 0);
    CHECK(s.capacity() >= 0);
    CHECK(s.is_sso() == true);
}

TEST_CASE("SSO operator") {
    SSOBytes a("hello");
    SSOBytes b("world");
    SSOBytes c("hello");
    CHECK(a != b);
    CHECK(a == c);
}

TEST_CASE("Copy constructor from SSO") {
    SSOBytes a;
    a.push_back('x');
    a.push_back('y');
    SSOBytes b(a);
    CHECK(b.size() == 2);
    CHECK(b[0] == 'x');
    CHECK(b[1] == 'y');
    CHECK(b.is_sso() == true);

    const SSOBytes c(a);
    CHECK(c[0] == 'x');
}

TEST_CASE("Copy constructor from heap") {
    SSOBytes a;
    for (int i = 0; i < 100; ++i) a.push_back(static_cast<Byte>('a' + i % 26)); // force into heap
    CHECK(a.is_sso() == false);
    SSOBytes b(a);
    CHECK(b.size() == 100);
    CHECK(b[0] == 'a');
    CHECK(b[25] == 'z');
    CHECK(b.is_sso() == false);
}

TEST_CASE("Copy assignment from SSO and heap") {
    SSOBytes a;
    for (char c : {'h', 'e', 'y'}) a.push_back(Byte(c));

    SSOBytes b;
    b = a;
    CHECK(b.size() == 3);
    CHECK(b[0] == 'h');
    CHECK(b.is_sso());

    SSOBytes big;
    for (int i = 0; i < 100; ++i) big.push_back('A');

    b = big;
    CHECK(b.size() == 100);
    CHECK(b[0] == 'A');
    CHECK(! b.is_sso());
}

TEST_CASE("Move constructor from SSO and heap") {
    SSOBytes a;
    a.push_back('k');
    SSOBytes b(std::move(a));
    CHECK(b.size() == 1);
    CHECK(b[0] == 'k');
    CHECK(b.is_sso());

    SSOBytes big;
    for (int i = 0; i < 80; ++i) big.push_back('Z');
    SSOBytes moved(std::move(big));
    CHECK(moved.size() == 80);
    CHECK(moved[0] == 'Z');
    CHECK(! moved.is_sso());
}

TEST_CASE("Move assignment from SSO and heap") {
    SSOBytes a;
    a.push_back('q');
    SSOBytes b;
    b = std::move(a);
    CHECK(b.size() == 1);
    CHECK(b[0] == 'q');
    CHECK(b.is_sso());

    SSOBytes big;
    for (int i = 0; i < 50; ++i) big.push_back('w');
    SSOBytes c;
    c = std::move(big);
    CHECK(c.size() == 50);
    CHECK(c[0] == 'w');
    CHECK(! c.is_sso());
}

TEST_CASE("Self-assignment should not crash or corrupt") {
    SSOBytes a;
    for (char c : {'x', 'y', 'z'}) a.push_back(Byte(c));
    CHECK(a.size() == 3);
    CHECK(a[2] == 'z');

    SSOBytes b;
    for (int i = 0; i < 80; ++i) b.push_back('x');
    CHECK(b.size() == 80);
    CHECK(b[0] == 'x');
}

TEST_CASE("SSOBytes push_back and operator[] (SSO mode)") {
    SSOBytes s;
    CHECK(s.empty());
    CHECK(s.size() == 0);

    s.push_back('a');
    s.push_back('b');
    s.push_back('c');
    CHECK(s.size() == 3);
    CHECK(s[0] == 'a');
    CHECK(s[1] == 'b');
    CHECK(s[2] == 'c');
    CHECK(s.is_sso());
}

TEST_CASE("SSOBytes push_back and operator[] (heap mode)") {
    SSOBytes s;
    for (int i = 0; i < 100; ++i) s.push_back(static_cast<Byte>('A' + i % 26));
    CHECK(s.size() == 100);
    CHECK(s[0] == 'A');
    CHECK(s[25] == 'Z');
    CHECK(! s.is_sso());
}

TEST_CASE("at() valid access and throws on invalid access") {
    SSOBytes s;
    s.push_back('x');
    s.push_back('y');
    CHECK(s.at(0) == 'x');
    CHECK(s.at(1) == 'y');

    s.push_back('z');
    CHECK_THROWS_AS(s.at(3), std::out_of_range);
    CHECK_THROWS_AS(s.at(99), std::out_of_range);
}

TEST_CASE("front() and back() behavior") {
    SSOBytes s;
    s.push_back('m');
    s.push_back('n');
    s.push_back('o');
    CHECK(s.front() == 'm');
    CHECK(s.back() == 'o');
    s.back() = 'z';
    CHECK(s.back() == 'z');

    const SSOBytes s2("mno");
    CHECK(s2.front() == 'm');
    CHECK(s2.back() == 'o');
    CHECK(s2.back() != 'z');
}

TEST_CASE("clear() should reset state") {
    SSOBytes s;
    s.push_back('x');
    s.push_back('y');
    s.clear();
    CHECK(s.empty());
    CHECK(s.size() == 0);
    CHECK(s.is_sso());

    SSOBytes heap("heap");
    heap.resize(100);
    CHECK(! heap.empty());
    CHECK(heap.size() == 100);
    CHECK(! heap.is_sso());
    heap.clear();
    CHECK(heap.empty());
    CHECK(heap.size() == 0);
    CHECK(! heap.is_sso()); // never back to sso
}

TEST_CASE("push_back() transitions from SSO to heap") {
    SSOBytes s;
    for (std::size_t i = 0; i < SSOBytes::SSO_CAPACITY; ++i) s.push_back('a');
    CHECK(s.is_sso());
    s.push_back('b'); // force into heap
    CHECK(! s.is_sso());
    CHECK(s.size() == SSOBytes::SSO_CAPACITY + 1);
    CHECK(s.back() == 'b');
}

TEST_CASE("pop_back() in both SSO and heap") {
    SUBCASE("SSO mode") {
        SSOBytes s;
        s.push_back('a');
        s.push_back('b');
        s.pop_back();
        CHECK(s.size() == 1);
        CHECK(s.back() == 'a');
    }

    SUBCASE("heap mode") {
        SSOBytes s;
        for (int i = 0; i < 100; ++i) s.push_back('x');
        CHECK(! s.is_sso());
        s.pop_back();
        CHECK(s.size() == 99);
    }
}

TEST_CASE("pop_back() throws on empty container") {
    SSOBytes s;
    CHECK_THROWS_AS(s.pop_back(), std::runtime_error);

    SSOBytes heap_s;
    for (int i = 0; i < 80; ++i) heap_s.push_back('x');
    while (! heap_s.empty()) heap_s.pop_back();
    CHECK_THROWS_AS(heap_s.pop_back(), std::runtime_error);
}

TEST_CASE("append(Byte*, len) in SSO mode") {
    SSOBytes s;
    const char* hello = "hello";
    s.append(reinterpret_cast<const Byte*>(hello), 5);
    CHECK(s.size() == 5);
    CHECK(std::memcmp(s.data(), hello, 5) == 0);
    CHECK(s.is_sso());
}

TEST_CASE("append(Byte*, len) triggers heap promotion") {
    SSOBytes s;
    const char* str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    s.append(reinterpret_cast<const Byte*>(str), std::strlen(str));
    CHECK(s.size() == std::strlen(str));
    CHECK(! s.is_sso());
    CHECK(std::memcmp(s.data(), str, s.size()) == 0);
}

TEST_CASE("append(const char*)") {
    SSOBytes s;
    s.append("abc");
    CHECK(s.size() == 3);
    CHECK(s[0] == 'a');
    CHECK(s[1] == 'b');
    CHECK(s[2] == 'c');
}

TEST_CASE("append(const std::string&)") {
    SSOBytes s;
    std::string msg = "hello world";
    s.append(msg);
    CHECK(s.size() == msg.size());
    CHECK(std::equal(s.begin(), s.end(), msg.begin()));
}

TEST_CASE("append(std::initializer_list<Byte>)") {
    SSOBytes s;
    s.append({'x', 'y', 'z'});
    CHECK(s.size() == 3);
    CHECK(s[0] == 'x');
    CHECK(s[1] == 'y');
    CHECK(s[2] == 'z');
}

TEST_CASE("append empty string does not alter size or state") {
    SSOBytes s;
    s.append("");
    CHECK(s.size() == 0);
    CHECK(s.is_sso());

    s.append({});
    CHECK(s.size() == 0);

    std::string empty = "";
    s.append(empty);
    CHECK(s.size() == 0);
}

TEST_CASE("append respects boundary at SSO_CAPACITY") {
    SSOBytes s;
    std::string chunk(SSOBytes::SSO_CAPACITY, 'a');
    s.append(chunk);
    CHECK(s.size() == SSOBytes::SSO_CAPACITY);
    CHECK(s.is_sso());

    s.append("b"); // force into heap
    CHECK(s.size() == SSOBytes::SSO_CAPACITY + 1);
    CHECK(! s.is_sso());
    CHECK(s.back() == 'b');
}

TEST_CASE("append(nullptr, 0) is safe") {
    SSOBytes s;
    CHECK_NOTHROW(s.append(nullptr, 0));
    CHECK(s.empty());
}

TEST_CASE("insert(pos, byte) within SSO") {
    SSOBytes s;
    s.append("ace");
    s.insert(1, 'b');
    CHECK(s.size() == 4);
    CHECK(std::string(s.begin(), s.end()) == "abce");
}

TEST_CASE("insert(pos, byte) into heap") {
    SSOBytes s;
    s.resize(SSOBytes::SSO_CAPACITY + 1, 'x');
    CHECK(! s.is_sso());
    s.insert(1, 'y');
    CHECK(! s.is_sso());
    CHECK(s[1] == 'y');
    CHECK(s.size() == SSOBytes::SSO_CAPACITY + 2);
}

TEST_CASE("insert(pos, byte) invalid") {
    SSOBytes s;
    CHECK_THROWS_AS(s.insert(1, 'z'), std::out_of_range);
}

TEST_CASE("insert(pos, It, It) into SSO") {
    SSOBytes s;
    s.append("ab");
    const char* insert = "123";
    s.insert(1, insert, insert + 3);
    CHECK(std::string(s.begin(), s.end()) == "a123b");
}

TEST_CASE("insert(pos, It, It) into heap") {
    SSOBytes s;
    s.resize(SSOBytes::SSO_CAPACITY + 1, 'x');
    CHECK(! s.is_sso());
    const char* insert = "AB";
    s.insert(0, insert, insert + 2);
    CHECK(s[0] == 'A');
    CHECK(s[1] == 'B');
    CHECK(s.size() == SSOBytes::SSO_CAPACITY + 3);
}

TEST_CASE("insert(pos, It, It) invalid") {
    SSOBytes s;
    const char* data = "hi";
    CHECK_THROWS_AS(s.insert(3, data, data + 2), std::out_of_range);
}

TEST_CASE("resize() to larger in SSO") {
    SSOBytes s;
    s.append("abc");
    s.resize(5, 'z');
    CHECK(std::string(s.begin(), s.end()) == "abczz");
    CHECK(s.is_sso());
}

TEST_CASE("resize() triggers heap") {
    SSOBytes s;
    s.resize(SSOBytes::SSO_CAPACITY + 5, 'x');
    CHECK(! s.is_sso());
    CHECK(s.size() == SSOBytes::SSO_CAPACITY + 5);
    for (auto c : s) CHECK(c == 'x');
}

TEST_CASE("reserve() promotes to heap") {
    SSOBytes s;
    s.reserve(SSOBytes::SSO_CAPACITY + 10);
    CHECK(! s.is_sso());
    CHECK(s.capacity() >= SSOBytes::SSO_CAPACITY + 10);
    s.reserve(SSOBytes::SSO_CAPACITY + 100);
    CHECK(! s.is_sso());
    CHECK(s.capacity() >= SSOBytes::SSO_CAPACITY + 100);
}

TEST_CASE("erase() from SSO") {
    SSOBytes s;
    s.append("hello");
    s.erase(1);
    CHECK(std::string(s.begin(), s.end()) == "hllo");
}

TEST_CASE("erase() from heap") {
    SSOBytes s;
    s.resize(SSOBytes::SSO_CAPACITY + 5, 'a');
    CHECK(! s.is_sso());
    s.erase(3);
    CHECK(s.size() == SSOBytes::SSO_CAPACITY + 4);
}

TEST_CASE("erase() out-of-bounds") {
    SSOBytes s;
    CHECK_THROWS_AS(s.erase(0), std::out_of_range);
}

TEST_CASE("assign(It, It) into SSO") {
    SSOBytes s;
    const char* src = "hello";
    s.assign(src, src + 5);
    CHECK(std::string(s.begin(), s.end()) == "hello");
    CHECK(s.is_sso());
}

TEST_CASE("assign(It, It) into heap") {
    SSOBytes s;
    std::string big(SSOBytes::SSO_CAPACITY + 20, 'x');
    s.assign(big.begin(), big.end());
    CHECK(! s.is_sso());
    CHECK(s.size() == big.size());
}

TEST_CASE("assign(initializer_list)") {
    SSOBytes s;
    s.assign({'x', 'y', 'z'});
    CHECK(s.size() == 3);
    CHECK(std::string(s.begin(), s.end()) == "xyz");
}

TEST_CASE("SSOBytes(char* pattern, size_t count)") {
    SUBCASE("Null pattern") {
        const char* nullstr = nullptr;
        SSOBytes s(nullstr, 10); // must not crash
        CHECK(s.empty());
    }

    SUBCASE("Count zero") {
        SSOBytes s("abc", 0);
        CHECK(s.empty());
    }
}

TEST_CASE("SSOBytes(Byte ch)") {
    SSOBytes s('Q');
    CHECK(s.size() == 1);
    CHECK(s[0] == 'Q');
}

TEST_CASE("SSOBytes(const char* pattern)") {
    SSOBytes s("ok");
    CHECK(s.size() == 2);
    CHECK(std::string(s.begin(), s.end()) == "ok");
}

TEST_CASE("SSOBytes(const std::string&)") {
    std::string str = "world";
    SSOBytes s(str);
    CHECK(s.size() == 5);
    CHECK(std::string(s.begin(), s.end()) == "world");
}

TEST_CASE("shrink_to_fit works for heap mode only") {
    SSOBytes s(std::string('x', SSOBytes::SSO_CAPACITY + 100)); // force heap
    auto old_capacity = s.capacity();
    s.resize(s.size() / 2); // shrink
    s.shrink_to_fit();
    CHECK(s.capacity() < old_capacity); // shrink
}

TEST_CASE("swap: SSO <-> SSO") {
    SSOBytes a("abc");
    SSOBytes b("xyz");

    a.swap(b);
    CHECK(std::string(a.begin(), a.end()) == "xyz");
    CHECK(std::string(b.begin(), b.end()) == "abc");

    swap(a, b);
    CHECK(std::string(a.begin(), a.end()) == "abc");
    CHECK(std::string(b.begin(), b.end()) == "xyz");
}

TEST_CASE("swap: heap <-> heap") {
    SSOBytes a("abc");
    a.resize(100); // heap
    SSOBytes b("xyz");
    b.resize(100); // heap

    a.swap(b);
    CHECK(std::string(a.begin(), a.begin() + 3) == "xyz");
    CHECK(std::string(b.begin(), b.begin() + 3) == "abc");
}

TEST_CASE("swap: SSO <-> heap") {
    SSOBytes a("abc");
    SSOBytes b("xyz");
    b.resize(100);
    bool a_sso = a.is_sso();
    bool b_sso = b.is_sso();

    a.swap(b);

    // 内容检查没问题
    CHECK(std::string(a.begin(), a.begin() + 3) == "xyz");
    CHECK(std::string(b.begin(), b.begin() + 3) == "abc");

    // 模式是否互换
    CHECK(a.is_sso() == b_sso);
    CHECK(b.is_sso() == a_sso);
}

TEST_CASE("iterator correctness") {
    SSOBytes s;
    std::string ref;
    for (char c = 'a'; c <= 'z'; ++c) {
        s.push_back(Byte(c));
        ref += c;
    }

    std::string iter;
    for (auto b : s) {
        iter += static_cast<char>(b);
    }
    CHECK(iter == ref);

    iter.clear();
    const SSOBytes s2(s);
    for (const auto b : s2) {
        iter += static_cast<char>(b);
    }
    CHECK(iter == ref);

    iter.clear();
    for (auto it = s2.cbegin(); it != s2.cend(); it++) {
        iter += static_cast<char>(*it);
    }
    CHECK(iter == ref);
}
