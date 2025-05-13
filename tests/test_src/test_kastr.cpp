#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <stdexcept>
#include <doctest/doctest.h>
#include "../../include/kastring/kastring.hpp"

using namespace kastring;

TEST_CASE("KAStr basic ASCII-only operations") {
    SUBCASE("empty string") {
        KAStr s;
        CHECK(s.empty());
        CHECK(s.byte_size() == 0);
        CHECK(s == "");
        CHECK(s == "");
    }

    SUBCASE("from const char* constructor") {
        const char* raw = "hello";
        KAStr s(raw);
        CHECK(! s.empty());
        CHECK(s.byte_size() == 5);
        CHECK(s == "hello");
        CHECK(s[0] == 'h');
        CHECK(s[4] == 'o');
        CHECK_THROWS_AS(s[5], std::out_of_range);
    }

    SUBCASE("from (char*, size_t) constructor") {
        const char* raw = "worldwide";
        KAStr s(raw, 5); // "world"
        CHECK(s.byte_size() == 5);
        CHECK(s == "world");
    }

    SUBCASE("byte_at access") {
        KAStr s = "abc";
        CHECK(s.byte_at(0) == 'a');
        CHECK(s.byte_at(1) == 'b');
        CHECK(s.byte_at(2) == 'c');
        CHECK_THROWS_AS(s.byte_at(3), std::out_of_range);
    }

    SUBCASE("begin()/end() iterators") {
        KAStr s("xyz");
        std::string collect(s.begin(), s.end());
        CHECK(collect == "xyz");
    }

    SUBCASE("rbegin()/rend() iterators") {
        KAStr s("xyz");
        std::string collect(s.rbegin(), s.rend());
        CHECK(collect == "zyx");
    }
}

TEST_CASE("KAStr::is_all_lower / is_all_upper") {
    SUBCASE("pure lower case") {
        CHECK(KAStr("abc").is_all_lower());
        CHECK_FALSE(KAStr("abc").is_all_upper());
    }

    SUBCASE("pure upper case") {
        CHECK(KAStr("XYZ").is_all_upper());
        CHECK_FALSE(KAStr("XYZ").is_all_lower());
    }

    SUBCASE("mixed case") {
        CHECK_FALSE(KAStr("AbC").is_all_lower());
        CHECK_FALSE(KAStr("AbC").is_all_upper());
    }

    SUBCASE("non-alpha characters") {
        CHECK_FALSE(KAStr("123").is_all_lower());
        CHECK_FALSE(KAStr("123").is_all_upper());
        CHECK_FALSE(KAStr("hello!").is_all_lower()); // 若使用严格版本
        CHECK_FALSE(KAStr("HELLO!").is_all_upper());
    }

    SUBCASE("empty string") {
        CHECK(KAStr("").is_all_lower());
        CHECK(KAStr("").is_all_upper());
    }
}

TEST_CASE("KAStr: brutal match and slicing tests") {

    SUBCASE("find: basic cases") {
        KAStr s("abracadabra");
        CHECK(s.find("dasdasdasdasdasdwqedqwd") == knpos);
        CHECK(s.find("abra") == 0);
        CHECK(s.find("cad") == 4);
        CHECK(s.find("xyz") == std::string::npos);
        CHECK(s.find("") == 0); // 空串永远匹配位置 0

        KAStr empty("");
        CHECK(empty.find("anything") == std::string::npos);
        CHECK(s.find("a") == 0);
    }

    SUBCASE("rfind: rightmost match") {
        KAStr s("abracadabra");
        CHECK(s.rfind("dasdasdasdasdasdwqedqwd") == knpos);
        CHECK(s.rfind("abra") == 7);
        CHECK(s.rfind("a") == 10);
        CHECK(s.rfind("xyz") == std::string::npos);
        CHECK(s.rfind("") == s.byte_size()); // 空串 rfind 返回长度
    }

    SUBCASE("case insensitive find/contains") {
        KAStr s("HelloWorld");
        CHECK(s.find("woRld", false) == 5);
        CHECK(s.rfind("woRld", false) == 5);
        CHECK(s.contains("woRld", false));
        CHECK(s.contains("h", false));
        CHECK_FALSE(s.contains("z", false));
    }

    SUBCASE("contains") {
        KAStr s("hello world");
        CHECK(s.contains("hello"));
        CHECK(s.contains("world"));
        CHECK_FALSE(s.contains("bye"));
        CHECK(s.contains("")); // 空串也认为包含
    }

    SUBCASE("starts_with") {
        KAStr s("banana");
        CHECK(s.starts_with("ban"));
        CHECK_FALSE(s.starts_with("BAN", true));
        CHECK(s.starts_with("BAN", false));
        CHECK_FALSE(s.starts_with("nan"));
        CHECK(s.starts_with(""));                // 空前缀总是匹配
        CHECK(KAStr("").starts_with(""));        // empty starts_with empty → true
        CHECK_FALSE(KAStr("").starts_with("a")); // empty 不会 start with 非空
    }

    SUBCASE("ends_with") {
        KAStr s("banana");
        CHECK(s.ends_with("ana"));
        CHECK_FALSE(s.ends_with("ANA", true));
        CHECK(s.ends_with("ANA", false));
        CHECK_FALSE(s.ends_with("ban"));
        CHECK(s.ends_with(""));         // 空后缀总是匹配
        CHECK(KAStr("").ends_with("")); // empty ends_with empty → true
        CHECK_FALSE(KAStr("").ends_with("x"));
    }

    SUBCASE("substr slicing torture") {
        KAStr s("abcdefgh");
        KAStr null_delim("\0", 1);

        CHECK(s.substr(0, 3) == "abc");
        CHECK(s.substr(2, 4) == "cdef");
        CHECK(s.substr(5) == "fgh");
        CHECK(s.subrange(1, 6) == "bcdef");
        CHECK(s.subrange(3) == "defgh");

        // 超范围切片：允许 start==size()，返回空串
        CHECK(s.substr(8) == "");
        CHECK(s.subrange(8) == "");

        // 崩溃边缘：非法切片越界
        CHECK(s.substr(9).empty());
        CHECK(s.substr(100).empty());
        CHECK(s.subrange(4, 20) == "efgh");
        CHECK(s.subrange(6, 3).empty()); // 逆序非法

        CHECK(s.substr_until("def") == "abc");
        CHECK(s.substr_until("df") == "abcdefgh");
        CHECK(KAStr("abc\0def", 7).substr_until(null_delim) == "abc");
        CHECK_THROWS_AS(s.substr_until(""), std::invalid_argument);
        CHECK(KAStr("").substr_until("other") == "");
        CHECK(KAStr("a").substr_until("other") == "a");

        CHECK(s.substr_from("def") == "gh");
        CHECK(s.substr_from("df") == "");
        CHECK(KAStr("abc\0def", 7).substr_from(null_delim) == "def");
        CHECK_THROWS_AS(s.substr_from(""), std::invalid_argument);
        CHECK(KAStr("").substr_from("other") == "");
        CHECK(KAStr("a").substr_from("other") == "");

        CHECK(KAStr("abc[hello]def").substr_between("[", "]") == "hello");                     // => "hello"
        CHECK(KAStr("abc\0hello\0def", 13).substr_between(null_delim, null_delim) == "hello"); // => "hello"
        CHECK(KAStr("abc[[hello]]]def").substr_between("[[", "]]]") == "hello");               // => "hello"
        CHECK(KAStr("abchello]def").substr_between("[", "]") == "");
        CHECK(KAStr("abc[hellodef").substr_between("[", "]") == "");
        CHECK(KAStr("abc[h[el]lod]ef").substr_between("[", "]") == "h[el]lod");
        CHECK(KAStr("abc]hellod[ef").substr_between("[", "]") == "");

        CHECK_THROWS_AS(KAStr("abc[h[el]lod]ef").substr_between("[", ""), std::invalid_argument);
        CHECK_THROWS_AS(KAStr("abc[h[el]lod]ef").substr_between("", "]"), std::invalid_argument);
    }

    SUBCASE("== and !=") {
        KAStr a("test");
        KAStr b("test");
        KAStr c("TEST");
        KAStr d("test1");

        CHECK(a == b);
        CHECK_FALSE(a != b);
        CHECK(a != c);
        CHECK(a != d);
        CHECK(a == "test");
        CHECK_FALSE(a == "nope");
    }

    SUBCASE("operator<< stream output") {
        KAStr s("streaming");
        std::ostringstream oss;
        oss << s;
        CHECK(oss.str() == "streaming");

        KAStr empty("");
        std::ostringstream oss2;
        oss2 << empty;
        CHECK(oss2.str() == "");
    }

    SUBCASE("trick edge cases with special chars") {
        KAStr s("abc\0def", 7); // 带 '\0'
        CHECK(s.byte_size() == 7);
        CHECK(s[3] == '\0');
        CHECK(s == std::string("abc\0def", 7));
        CHECK(std::string("abc\0def", 7) == s);
        CHECK(s.substr(3, 1) == std::string("\0", 1));
        CHECK(s.contains(KAStr("\0", 1)));
    }
}

TEST_CASE("KAStr: brutal split/split_once/split_at testing") {
    KAStr base("a,b,c,d,e");

    SUBCASE("split_at & split_exclusive_at") {
        auto splited = base.split_at(3);
        auto l = splited.first;
        auto r = splited.second;
        CHECK(l == "a,b");
        CHECK(r == ",c,d,e");

        auto splited2 = base.split_exclusive_at(3);
        auto lx = splited2.first;
        auto rx = splited2.second;
        CHECK(lx == "a,b");
        CHECK(rx == "c,d,e");

        CHECK_THROWS(base.split_at(100));
        CHECK_THROWS(base.split_exclusive_at(100));
    }

    SUBCASE("split_count: basic, full split") {
        auto result = base.split_count(",", 10);
        std::vector<std::string> expected = {"a", "b", "c", "d", "e"};
        REQUIRE(result.size() == expected.size());
        for (std::size_t i = 0; i < expected.size(); ++i) CHECK(result[i] == expected[i]);
    }

    SUBCASE("split_count: limited split") {
        auto result = base.split_count(",", 2);
        CHECK(result.size() == 3);
        CHECK(result[0] == "a");
        CHECK(result[1] == "b");
        CHECK(result[2] == "c,d,e");
    }

    SUBCASE("split_count: empty delimiter with limit") {
        KAStr s("abcde");

        auto r0 = s.split_count("", 0);
        CHECK(r0.size() == 1);
        CHECK(r0[0] == "abcde");

        auto r1 = s.split_count("", 1);
        CHECK(r1.size() == 2);
        CHECK(r1[0] == "a");
        CHECK(r1[1] == "bcde");

        auto r4 = s.split_count("", 4);
        CHECK(r4.size() == 5);
        CHECK(r4[0] == "a");
        CHECK(r4[1] == "b");
        CHECK(r4[2] == "c");
        CHECK(r4[3] == "d");
        CHECK(r4[4] == "e");

        auto r10 = s.split_count("", 10);
        CHECK(r10.size() == 5); // 最多 len 个字符
    }

    SUBCASE("rsplit_count: basic, full reverse split") {
        auto result = base.rsplit_count(",", 10);
        std::vector<std::string> expected = {"e", "d", "c", "b", "a"};
        REQUIRE(result.size() == expected.size());
        for (std::size_t i = 0; i < expected.size(); ++i) CHECK(result[i] == expected[i]);
    }

    SUBCASE("rsplit_count: limited split") {
        auto result = base.rsplit_count(",", 2);
        CHECK(result.size() == 3);
        CHECK(result[0] == "e");
        CHECK(result[1] == "d");
        CHECK(result[2] == "a,b,c");
    }

    SUBCASE("rsplit_count: empty delimiter with limit") {
        KAStr s("abcde");

        auto r0 = s.rsplit_count("", 0);
        CHECK(r0.size() == 1);
        CHECK(r0[0] == "abcde");

        auto r2 = s.rsplit_count("", 2);
        CHECK(r2.size() == 3);
        CHECK(r2[0] == "e");
        CHECK(r2[1] == "d");
        CHECK(r2[2] == "abc");

        auto r5 = s.rsplit_count("", 5);
        CHECK(r5.size() == 5);
        CHECK(r5[0] == "e");
        CHECK(r5[1] == "d");
        CHECK(r5[2] == "c");
        CHECK(r5[3] == "b");
        CHECK(r5[4] == "a");
    }


    SUBCASE("split / rsplit = split_count / rsplit_count with no limit") {
        auto a = base.split(",");
        auto b = base.split_count(",", -1);
        REQUIRE(a.size() == b.size());
        for (std::size_t i = 0; i < a.size(); ++i) CHECK(a[i] == b[i]);

        auto ar = base.rsplit(",");
        auto br = base.rsplit_count(",", -1);
        REQUIRE(ar.size() == br.size());
        for (std::size_t i = 0; i < ar.size(); ++i) CHECK(ar[i] == br[i]);
    }

    SUBCASE("split_once") {
        std::pair<KAStr, KAStr> result = base.split_once(",");
        CHECK(result.first == "a");
        CHECK(result.second == "b,c,d,e");

        std::pair<KAStr, KAStr> not_found = base.split_once("z");
        CHECK(not_found.first == base);
        CHECK(not_found.second == "");
    }

    SUBCASE("rsplit_once") {
        std::pair<KAStr, KAStr> result = base.rsplit_once(",");
        CHECK(result.first == "e");
        CHECK(result.second == "a,b,c,d");

        std::pair<KAStr, KAStr> not_found = base.rsplit_once("z");
        CHECK(not_found.first == base);
        CHECK(not_found.second == "");
    }

    SUBCASE("edge: delimiter at boundaries") {
        KAStr s(",a,b,");
        auto result = s.split_count(",", 10);
        std::vector<std::string> expected = {"", "a", "b", ""};
        REQUIRE(result.size() == expected.size());
        for (std::size_t i = 0; i < expected.size(); ++i) CHECK(result[i] == expected[i]);
    }

    SUBCASE("edge: empty input") {
        KAStr empty("");
        auto result = empty.split(",");
        CHECK(result.size() == 1);
        CHECK(result[0] == "");
    }

    SUBCASE("edge: delimiter longer than input") {
        KAStr s("x");
        auto result = s.split("xyz");
        CHECK(result.size() == 1);
        CHECK(result[0] == "x");
    }

    SUBCASE("edge: split by full string") {
        KAStr s("hello");
        auto result = s.split("hello");
        CHECK(result.size() == 2);
        CHECK(result[0] == "");
        CHECK(result[1] == "");
    }

    SUBCASE("edge: repeated delimiter") {
        KAStr s("a--b--c");
        auto result = s.split("--");
        std::vector<std::string> expected = {"a", "b", "c"};
        REQUIRE(result.size() == expected.size());
        for (std::size_t i = 0; i < expected.size(); ++i) CHECK(result[i] == expected[i]);
    }
}

TEST_CASE("KAStr whitespace & line splitting torture") {

    SUBCASE("split_whitespace: basic + tabs + newlines") {
        KAStr s(" \t  abc \n def  \r ghi\t\n  ");
        auto parts = s.split_whitespace();

        REQUIRE(parts.size() == 3);
        CHECK(parts[0] == "abc");
        CHECK(parts[1] == "def");
        CHECK(parts[2] == "ghi");
    }

    SUBCASE("split_whitespace: multiple spaces") {
        KAStr s("a   b    c");
        auto parts = s.split_whitespace();

        REQUIRE(parts.size() == 3);
        CHECK(parts[0] == "a");
        CHECK(parts[1] == "b");
        CHECK(parts[2] == "c");
    }

    SUBCASE("split_whitespace: empty input") {
        KAStr s("");
        auto parts = s.split_whitespace();
        CHECK(parts.empty());
    }

    SUBCASE("split_whitespace: only whitespace") {
        KAStr s(" \t\n\v\f\r ");
        auto parts = s.split_whitespace();
        CHECK(parts.empty());
    }

    SUBCASE("split_whitespace: no split") {
        KAStr s("singleword");
        auto parts = s.split_whitespace();
        REQUIRE(parts.size() == 1);
        CHECK(parts[0] == "singleword");
    }

    SUBCASE("lines: \\n splitting") {
        KAStr s("a\nb\nc");
        auto lines = s.lines();

        REQUIRE(lines.size() == 3);
        CHECK(lines[0] == "a");
        CHECK(lines[1] == "b");
        CHECK(lines[2] == "c");
    }

    SUBCASE("lines: mixed \\n \\r\\n \\r") {
        KAStr s("a\nb\r\nc\rd");
        auto lines = s.lines();

        REQUIRE(lines.size() == 4);
        CHECK(lines[0] == "a");
        CHECK(lines[1] == "b");
        CHECK(lines[2] == "c");
        CHECK(lines[3] == "d");
    }

    SUBCASE("lines: ending with newline") {
        KAStr s("a\nb\n");
        auto lines = s.lines();

        REQUIRE(lines.size() == 2);
        CHECK(lines[0] == "a");
        CHECK(lines[1] == "b");
    }

    SUBCASE("lines: empty and blank lines") {
        KAStr s("\n\nabc\n\ndef");
        auto lines = s.lines();

        REQUIRE(lines.size() == 5);
        CHECK(lines[0] == "");
        CHECK(lines[1] == "");
        CHECK(lines[2] == "abc");
        CHECK(lines[3] == "");
        CHECK(lines[4] == "def");
    }
}

TEST_CASE("KAStr prefix/suffix/trim") {

    SUBCASE("strip_prefix: match & no match") {
        KAStr s("foobar");

        CHECK(s.strip_prefix("foo") == "bar");
        CHECK(s.strip_prefix("bar") == "foobar");   // no match
        CHECK(s.strip_prefix("") == "foobar");      // empty prefix
        CHECK(KAStr("").strip_prefix("foo") == ""); // empty input
    }

    SUBCASE("strip_prefix: full match") {
        KAStr s("abc");
        CHECK(s.strip_prefix("abc") == "");
    }

    SUBCASE("strip_suffix: match & no match") {
        KAStr s("helloworld");

        CHECK(s.strip_suffix("world") == "hello");
        CHECK(s.strip_suffix("hello") == "helloworld"); // no match
        CHECK(s.strip_suffix("") == "helloworld");
        CHECK(KAStr("").strip_suffix("x") == "");
    }

    SUBCASE("strip_suffix: full match") {
        KAStr s("xyz");
        CHECK(s.strip_suffix("xyz") == "");
    }

    SUBCASE("trim_start: crazy spaces") {
        KAStr s(" \t\n\r\v\fHello");
        CHECK(s.trim_start() == "Hello");

        KAStr s2(" \t \n\r");
        CHECK(s2.trim_start() == ""); // all whitespace

        CHECK(KAStr("").trim_start() == "");
    }

    SUBCASE("trim_end: chaotic endings") {
        KAStr s("Goodbye \t\n\r\v\f");
        CHECK(s.trim_end() == "Goodbye");

        KAStr s2(" \t \n\r");
        CHECK(s2.trim_end() == "");

        CHECK(KAStr("").trim_end() == "");
    }

    SUBCASE("trim: full frontal assault") {
        KAStr s(" \n  \tHello World  \v \r\n ");
        CHECK(s.trim() == "Hello World");

        KAStr s2(" \t\r\n");
        CHECK(s2.trim() == "");

        CHECK(KAStr("").trim() == "");
    }

    SUBCASE("trim vs strip_* conflict") {
        KAStr s("  prefixmiddlepostfix  ");
        auto trimmed = s.trim();

        CHECK(trimmed == "prefixmiddlepostfix");

        auto stripped = trimmed.strip_prefix("prefix").strip_suffix("postfix");

        CHECK(stripped == "middle");
    }

    SUBCASE("user uses partial prefix/suffix") {
        KAStr s("banana");
        CHECK(s.strip_prefix("bananaz") == "banana");
        CHECK(s.strip_suffix("anana") == "b"); // only suffix match fails
        CHECK(s.strip_suffix("na") == "bana");
    }

    SUBCASE("user inputs longer than string") {
        KAStr s("abc");
        CHECK(s.strip_prefix("abcdef") == "abc");
        CHECK(s.strip_suffix("abcdef") == "abc");
    }
}

TEST_CASE("KAStr::count - ASCII-only") {
    SUBCASE("non-overlapping, case-sensitive") {
        CHECK(KAStr("abcabcabc").count(KAStr("abc")) == 3);
        CHECK(KAStr("abcabcabc").count(KAStr("ab")) == 3);
        CHECK(KAStr("aaaaa").count(KAStr("aa")) == 2); // "aa" at 0 and 2
    }

    SUBCASE("overlapping, case-sensitive") {
        CHECK(KAStr("aaaaa").count_overlapping(KAStr("aa")) == 4); // at 0,1,2,3
        CHECK(KAStr("abcabcabc").count_overlapping(KAStr("bc")) == 3);
    }

    SUBCASE("case-insensitive matching") {
        CHECK(KAStr("AbCabCabc").count(KAStr("abc"), false) == 3);
        CHECK(KAStr("aaAAaA").count(KAStr("aaa"), false) == 2); // matches: "aaA", "AaA"
    }

    SUBCASE("case-insensitive overlapping") {
        CHECK(KAStr("aAaAaA").count_overlapping("aa", false) == 5); // aA, Aa, aA, Aa, aA
    }

    SUBCASE("empty pattern or oversized pattern") {
        CHECK(KAStr("abc").count("") == 0);
        CHECK(KAStr("abc").count("abcdef") == 0);
        CHECK(KAStr("").count("abc") == 0);
    }

    SUBCASE("pattern equals input") {
        CHECK(KAStr("abc").count("abc") == 1);
        CHECK(KAStr("abc").count_overlapping("abc") == 1);
    }

    SUBCASE("single char pattern") {
        CHECK(KAStr("aaabaa").count("a") == 5);
        CHECK(KAStr("aaabaa").count_overlapping("a") == 5);
    }
}

TEST_CASE("KAStr constructors and basic accessors") {
    SUBCASE("Default constructor yields empty string") {
        KAStr s;
        CHECK(s.empty());
        CHECK(s.byte_size() == 0);
    }

    SUBCASE("Construct from C string") {
        KAStr s = "hello";
        CHECK(! s.empty());
        CHECK(s.byte_size() == 5);
        CHECK(s.as_bytes().data()[0] == 'h');
    }

    SUBCASE("Construct from char pointer + size") {
        const char* raw = "world!!";
        KAStr s(raw, 5); // only take "world"
        CHECK(! s.empty());
        CHECK(s.byte_size() == 5);
    }

    SUBCASE("Construct from ByteSpan") {
        const char* raw = "abc";
        KAStr s(raw);
        CHECK(s.byte_size() == 3);
    }
}

TEST_CASE("KAStr operater") {
    KAStr s("你好abc");
    CHECK("你好abc" == s);
    CHECK(s == "你好abc");
    CHECK(std::string("你好abc") == s);
    CHECK(s == std::string("你好abc"));

    CHECK(KAStr("abcd") < KAStr("abcde"));
    CHECK(KAStr("abcd") <= KAStr("abcde"));
    CHECK(KAStr("abce") > KAStr("abcd"));
    CHECK(KAStr("abce") >= KAStr("abcd"));
}

TEST_CASE("KAStr split family basic functionality and corner cases") {
    KAStr s("a,b,c,,d");

    SUBCASE("split normal") {
        auto parts = s.split(",");
        REQUIRE(parts.size() == 5);
        CHECK(parts[0] == "a");
        CHECK(parts[1] == "b");
        CHECK(parts[2] == "c");
        CHECK(parts[3] == "");
        CHECK(parts[4] == "d");
    }

    SUBCASE("rsplit normal") {
        auto parts = s.rsplit(",");
        REQUIRE(parts.size() == 5);
        CHECK(parts[0] == "d");
        CHECK(parts[1] == "");
        CHECK(parts[2] == "c");
        CHECK(parts[3] == "b");
        CHECK(parts[4] == "a");
    }

    SUBCASE("split_count limits splits") {
        auto parts = s.split_count(",", 2);
        REQUIRE(parts.size() == 3);
        CHECK(parts[0] == "a");
        CHECK(parts[1] == "b");
        CHECK(parts[2] == "c,,d");
    }

    SUBCASE("rsplit_count limits splits") {
        auto parts = s.rsplit_count(",", 2);
        REQUIRE(parts.size() == 3);
        CHECK(parts[0] == "d");
        CHECK(parts[1] == "");
        CHECK(parts[2] == "a,b,c");
    }

    SUBCASE("split_once finds first") {
        auto parts = s.split_once(",");
        CHECK(parts.first == "a");
        CHECK(parts.second == "b,c,,d");
    }

    SUBCASE("rsplit_once finds last") {
        auto parts = s.rsplit_once(",");
        CHECK(parts.first == "d");
        CHECK(parts.second == "a,b,c,");
    }

    SUBCASE("split with no delimiter present") {
        auto parts = KAStr("abc").split("-");
        REQUIRE(parts.size() == 1);
        CHECK(parts[0] == "abc");
    }

    SUBCASE("rsplit with no delimiter present") {
        auto parts = KAStr("abc").rsplit("-");
        REQUIRE(parts.size() == 1);
        CHECK(parts[0] == "abc");
    }

    SUBCASE("split_once with no delimiter") {
        auto parts = KAStr("abc").split_once("-");
        CHECK(parts.first == "abc");
        CHECK(parts.second == "");
    }

    SUBCASE("rsplit_once with no delimiter") {
        auto parts = KAStr("abc").rsplit_once("-");
        CHECK(parts.first == "abc");
        CHECK(parts.second == "");
    }

    SUBCASE("split with empty string") {
        auto parts = KAStr("").split(",");
        REQUIRE(parts.size() == 1);
        CHECK(parts[0] == "");
    }

    SUBCASE("split with empty delimiter should split one char") {
        auto splited = KAStr("abc").split_count("", 1);
        CHECK(splited[0] == "a");
        CHECK(splited[1] == "bc");
        auto splited2 = KAStr("abc").rsplit_count("", 1);
        CHECK(splited2[0] == "c");
        CHECK(splited2[1] == "ab");
    }

    SUBCASE("split_count with max_splits = 0 returns whole") {
        auto parts = KAStr("a,b,c").split_count(",", 0);
        REQUIRE(parts.size() == 1);
        CHECK(parts[0] == "a,b,c");
    }

    SUBCASE("rsplit_count with max_splits = 0 returns whole") {
        auto parts = KAStr("a,b,c").rsplit_count(",", 0);
        REQUIRE(parts.size() == 1);
        CHECK(parts[0] == "a,b,c");
    }

    SUBCASE("split/rsplit UTF-8 multi-byte character as delimiter") {
        KAStr utf_str("你哈你哈你");
        KAStr delim("哈");

        auto parts = utf_str.split(delim);
        REQUIRE(parts.size() == 3);
        CHECK(parts[0] == "你");
        CHECK(parts[1] == "你");
        CHECK(parts[2] == "你");

        auto rparts = utf_str.rsplit(delim);
        REQUIRE(rparts.size() == 3);
        CHECK(rparts[0] == "你");
        CHECK(rparts[1] == "你");
        CHECK(rparts[2] == "你");
    }
}

TEST_CASE("KAStr lines handles \\n \\r \\r\\n and trailing newlines") {
    KAStr s("line1\nline2\rline3\r\nline4");
    auto lines = s.lines();
    REQUIRE(lines.size() == 4);
    CHECK(lines[0] == "line1");
    CHECK(lines[1] == "line2");
    CHECK(lines[2] == "line3");
    CHECK(lines[3] == "line4");
}

TEST_CASE("KAStr lines trailing line ending not results in empty string") {
    KAStr s("a\nb\n");
    auto lines = s.lines();
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "a");
    CHECK(lines[1] == "b");
}

TEST_CASE("KAStr lines empty string returns single empty line") {
    KAStr s("");
    auto lines = s.lines();
    REQUIRE(lines.size() == 0);
}

TEST_CASE("KAStr lines with only newlines") {
    KAStr s("\n\r\n\r"); // [1]\n[2]\r\n[3]\r
    auto lines = s.lines();
    REQUIRE(lines.size() == 3);
    CHECK(lines[0] == "");
    CHECK(lines[1] == "");
    CHECK(lines[2] == "");

    KAStr s2("\n");
    auto lines2 = s2.lines();
    REQUIRE(lines2.size() == 1);
    CHECK(lines2[0] == "");
}

TEST_CASE("KAStr match/match_indices basic and edge cases") {
    KAStr s_g("a123bb4567cc89");

    auto is_digit = [](char ch) { return std::isdigit(ch); };

    SUBCASE("match - continuous digit runs") {
        auto parts = s_g.match(is_digit);
        REQUIRE(parts.size() == 3);
        CHECK(parts[0] == "123");
        CHECK(parts[1] == "4567");
        CHECK(parts[2] == "89");
    }

    SUBCASE("match_indices - reports correct indices and substrings") {
        auto result = s_g.match_indices(is_digit);
        REQUIRE(result.size() == 3);
        CHECK(result[0].first == 1);
        CHECK(result[0].second == "123");
        CHECK(result[1].first == 6);
        CHECK(result[1].second == "4567");
        CHECK(result[2].first == 12);
        CHECK(result[2].second == "89");
    }

    SUBCASE("match - no match") {
        KAStr s2("abcXYZ");
        auto parts = s2.match(is_digit);
        CHECK(parts.empty());
    }

    SUBCASE("match - full match") {
        KAStr s3("123456");
        auto parts = s3.match(is_digit);
        REQUIRE(parts.size() == 1);
        CHECK(parts[0] == "123456");
    }


    SUBCASE("basic ASCII digit match") {
        KAStr s("ab123cd45e6fg");
        auto digits = s.match(is_digit);
        REQUIRE(digits.size() == 3);
        CHECK(digits[0] == "123");
        CHECK(digits[1] == "45");
        CHECK(digits[2] == "6");

        auto indices = s.match_indices(is_digit);
        REQUIRE(indices.size() == 3);
        CHECK(indices[0].first == 2);
        CHECK(indices[0].second == "123");
        CHECK(indices[1].first == 7);
        CHECK(indices[1].second == "45");
        CHECK(indices[2].first == 10);
        CHECK(indices[2].second == "6");
    }

    SUBCASE("no match found") {
        KAStr s("abcdef");
        CHECK(s.match(is_digit).empty());
        CHECK(s.match_indices(is_digit).empty());
    }

    SUBCASE("everything matches") {
        KAStr s("0123456789");
        auto all = s.match(is_digit);
        REQUIRE(all.size() == 1);
        CHECK(all[0] == "0123456789");

        auto indices = s.match_indices(is_digit);
        REQUIRE(indices.size() == 1);
        CHECK(indices[0].first == 0);
        CHECK(indices[0].second == "0123456789");
    }

    SUBCASE("empty string") {
        KAStr s("");
        CHECK(s.match(is_digit).empty());
        CHECK(s.match_indices(is_digit).empty());
    }
}

TEST_CASE("KAStr trim functions") {
    SUBCASE("empty string") {
        KAStr s("");
        CHECK(s.trim() == "");
        CHECK(s.trim_start() == "");
        CHECK(s.trim_end() == "");
    }

    SUBCASE("all ASCII whitespace") {
        KAStr s(" \t\n\r ");
        CHECK(s.trim() == "");
        CHECK(s.trim_start() == "");
        CHECK(s.trim_end() == "");
    }

    SUBCASE("no whitespace") {
        KAStr s("abc123");
        CHECK(s.trim() == "abc123");
        CHECK(s.trim_start() == "abc123");
        CHECK(s.trim_end() == "abc123");
    }

    SUBCASE("whitespace at front") {
        KAStr s(" \t\nabc");
        CHECK(s.trim_start() == "abc");
        CHECK(s.trim_end() == " \t\nabc");
        CHECK(s.trim() == "abc");
    }

    SUBCASE("whitespace at end") {
        KAStr s("abc  \n");
        CHECK(s.trim_end() == "abc");
        CHECK(s.trim_start() == "abc  \n");
        CHECK(s.trim() == "abc");
    }

    SUBCASE("whitespace at both ends") {
        KAStr s(" \tabc\n ");
        CHECK(s.trim() == "abc");
    }

    SUBCASE("internal whitespace not trimmed") {
        KAStr s("ab\t \n cd");
        CHECK(s.trim() == "ab\t \n cd");
    }

    SUBCASE("trim_matches - remove dashes") {
        KAStr s("---abc---");
        auto trimmed = s.trim_matches([](char c) { return c == '-'; });
        CHECK(trimmed == "abc");
    }

    SUBCASE("trim_start_matches only") {
        KAStr s("***data**");
        auto trimmed = s.trim_start_matches([](char c) { return c == '*'; });
        CHECK(trimmed == "data**");
    }

    SUBCASE("trim_end_matches only") {
        KAStr s("==hello==");
        auto trimmed = s.trim_end_matches([](char c) { return c == '='; });
        CHECK(trimmed == "==hello");
    }
}

TEST_CASE("KAStr::strip_prefix and strip_suffix") {
    SUBCASE("empty string and empty prefix/suffix") {
        KAStr s("");
        CHECK(s.strip_prefix("") == "");
        CHECK(s.strip_suffix("") == "");
    }

    SUBCASE("strip_prefix - match") {
        KAStr s("hello world");
        CHECK(s.strip_prefix("hello") == " world");
        CHECK(s.strip_prefix("hello ") == "world");
    }

    SUBCASE("strip_prefix - no match") {
        KAStr s("hello world");
        CHECK(s.strip_prefix("world") == s);  // no change
        CHECK(s.strip_prefix("heLLo") == s);  // case sensitive
        CHECK(s.strip_prefix("helloo") == s); // longer than prefix
    }

    SUBCASE("strip_suffix - match") {
        KAStr s("hello world");
        CHECK(s.strip_suffix("world") == "hello ");
        CHECK(s.strip_suffix(" world") == "hello");
    }

    SUBCASE("strip_suffix - no match") {
        KAStr s("hello world");
        CHECK(s.strip_suffix("hello") == s);
        CHECK(s.strip_suffix("WORLD") == s);
        CHECK(s.strip_suffix("worldd") == s);
    }

    SUBCASE("strip_prefix and strip_suffix do not repeat") {
        KAStr s("xxxabcxxx");
        CHECK(s.strip_prefix("xxx") == "abcxxx");                     // 只裁一次
        CHECK(s.strip_suffix("xxx") == "xxxabc");                     // 只裁一次
        CHECK(s.strip_prefix("xxx").strip_prefix("xxx") == "abcxxx"); // 仍保留后缀
    }

    SUBCASE("UTF-8 prefix/suffix match") {
        KAStr s("你好世界");
        CHECK(s.strip_prefix("你好") == "世界");
        CHECK(s.strip_suffix("世界") == "你好");
    }

    SUBCASE("UTF-8 prefix/suffix mismatch") {
        KAStr s("你好世界");
        CHECK(s.strip_prefix("你a") == s);
        CHECK(s.strip_suffix("界a") == s);
    }
}

TEST_CASE("KAStr integer and float conversion: basic correctness") {
    SUBCASE("basic int parsing") {
        CHECK(KAStr("42").to_int() == 42);
        CHECK(KAStr("-123").to_int() == -123);
        CHECK(KAStr("0").to_uint() == 0u);
        CHECK(KAStr("65535").to_ushort() == 65535);
        CHECK(KAStr("-32768").to_short() == -32768);
        CHECK(KAStr("32767").to_short() == 32767);
        CHECK(KAStr("42").to_long() == 42l);
        CHECK(KAStr("-42").to_long() == -42l);
        CHECK(KAStr("4294967295").to_ulong() == 4294967295ul);
        CHECK(KAStr("4294967295").to_ulonglong() == 4294967295ull);
        CHECK(KAStr("9223372036854775807").to_longlong() == 9223372036854775807ll);
    }

    SUBCASE("basic float parsing") {
        CHECK(KAStr("3.14").to_float() == doctest::Approx(3.14f));
        CHECK(KAStr("-2.5").to_double() == doctest::Approx(-2.5));
        CHECK(KAStr("0.0").to_float() == doctest::Approx(0.0f));
        CHECK(KAStr("1e10").to_double() == doctest::Approx(1e10));
    }

    SUBCASE("binary, octal, hex") {
        CHECK(KAStr("111").to_int(2) == 7);
        CHECK(KAStr("777").to_int(8) == 511);
        CHECK(KAStr("ff").to_int(16) == 255);
        CHECK(KAStr("7fffffff").to_int(16) == 0x7fffffff);
    }

    SUBCASE("invalid strings") {
        CHECK_THROWS_AS(KAStr("abc").to_int(), std::invalid_argument);
        CHECK(KAStr("1.2").to_int() == 1);
        CHECK_THROWS_AS(KAStr("").to_int(), std::invalid_argument);
    }

    SUBCASE("invalid base") {
        CHECK_THROWS_AS(KAStr("123").to_int(1), std::invalid_argument);
        CHECK_THROWS_AS(KAStr("123").to_int(37), std::invalid_argument);
    }

    SUBCASE("overflow/underflow on narrow cast") {
        CHECK_THROWS_AS(KAStr("4294967296").to_uint(), std::out_of_range);
        CHECK_THROWS_AS(KAStr("-1").to_ushort(), std::out_of_range);
        CHECK_THROWS_AS(KAStr("70000").to_ushort(), std::out_of_range);
        CHECK_THROWS_AS(KAStr("9999999999").to_int(), std::out_of_range);
    }
}

TEST_CASE("KAStr numeric conversion: limits and edges") {
    SUBCASE("int limits") {
        CHECK(KAStr(std::to_string(std::numeric_limits<int>::min())).to_int() ==
              std::numeric_limits<int>::min());
        CHECK(KAStr(std::to_string(std::numeric_limits<int>::max())).to_int() ==
              std::numeric_limits<int>::max());
    }

    SUBCASE("unsigned int limit") {
        CHECK(KAStr("4294967295").to_uint() == 4294967295u);
        CHECK_THROWS_AS(KAStr("4294967296").to_uint(), std::out_of_range);
    }

    SUBCASE("float/double limits") {
        auto fmax = std::to_string(std::numeric_limits<float>::max());
        auto dmax = std::to_string(std::numeric_limits<double>::max());
        CHECK(KAStr(fmax).to_float() == doctest::Approx(std::numeric_limits<float>::max()));
        CHECK(KAStr(dmax).to_double() == doctest::Approx(std::numeric_limits<double>::max()));
    }

    SUBCASE("exotic input") {
        CHECK_THROWS_AS(KAStr("1e99999").to_double(), std::out_of_range);
        CHECK_THROWS_AS(KAStr("9999999999999999999999999").to_longlong(), std::out_of_range);
    }
}

TEST_CASE("KAStr::to_float/double handles special floats") {
    SUBCASE("NaN and Inf") {
        auto nan = KAStr("nan");
        auto inf = KAStr("inf");
        CHECK(std::isnan(nan.to_double()));
        CHECK(std::isinf(inf.to_double()));
    }

    SUBCASE("negative zero") {
        CHECK(KAStr("-0.0").to_double() == doctest::Approx(0.0));
    }

    SUBCASE("scientific notation") {
        CHECK(KAStr("1.23e4").to_float() == doctest::Approx(12300.0f));
    }

    SUBCASE("exponent overflow") {
        CHECK_THROWS_AS(KAStr("1e10000").to_double(), std::out_of_range);
    }
}


TEST_CASE("parse_format: basic and escape cases") {
    auto S = [](const char* s) -> KAStr { return KAStr(s); };

    SUBCASE("plain text only") {
        auto parts = parse_format(S("hello world"));
        CHECK(parts.size() == 1);
        CHECK(parts[0] == "hello world");
    }

    SUBCASE("escaped braces {{ and }}") {
        auto parts = parse_format(S("a{{b}}c"));
        CHECK(parts.size() == 5);
        CHECK(parts[0] == "a");
        CHECK(parts[1] == "{");
        CHECK(parts[2] == "b");
        CHECK(parts[3] == "}");
        CHECK(parts[4] == "c");
    }

    SUBCASE("single format field") {
        auto parts = parse_format(S("a{0}b"));
        CHECK(parts.size() == 3);
        CHECK(parts[0] == "a");
        CHECK(parts[1] == "{0}");
        CHECK(parts[2] == "b");
    }

    SUBCASE("multiple format fields") {
        auto parts = parse_format(S("{a}{b}{c}"));
        CHECK(parts.size() == 3);
        CHECK(parts[0] == "{a}");
        CHECK(parts[1] == "{b}");
        CHECK(parts[2] == "{c}");
    }

    SUBCASE("adjacent brace and field") {
        auto parts = parse_format(S("x{{}}{y}z}}"));
        CHECK(parts.size() == 6);
        CHECK(parts[0] == "x");
        CHECK(parts[1] == "{");
        CHECK(parts[2] == "}");
        CHECK(parts[3] == "{y}");
        CHECK(parts[4] == "z");
        CHECK(parts[5] == "}");
    }
}

TEST_CASE("parse_format: malformed format strings") {
    using std::string;
    auto S = [](const char* s) -> KAStr { return KAStr(s); };

    SUBCASE("lone { at end") {
        CHECK_THROWS_AS(parse_format(S("abc{")), std::invalid_argument);
    }

    SUBCASE("lone } at start") {
        CHECK_THROWS_AS(parse_format(S("}abc")), std::invalid_argument);
    }

    SUBCASE("reverse order }{") {
        CHECK_THROWS_AS(parse_format(S("abc}{xyz")), std::invalid_argument);
    }

    SUBCASE("unescaped } in middle") {
        CHECK_THROWS_AS(parse_format(S("abc}def")), std::invalid_argument);
    }

    SUBCASE("unclosed format field") {
        CHECK_THROWS_AS(parse_format(S("abc{xyz")), std::invalid_argument);
    }

    SUBCASE("only one brace") {
        CHECK_THROWS_AS(parse_format(S("{")), std::invalid_argument);
        CHECK_THROWS_AS(parse_format(S("}")), std::invalid_argument);
    }
}

TEST_CASE("parse_format: edge combinations") {
    auto S = [](const char* s) -> KAStr { return KAStr(s); };

    SUBCASE("nothing") {
        auto parts = parse_format(S(""));
        CHECK(parts.size() == 0);
    }

    SUBCASE("only escaped") {
        auto parts = parse_format(S("{{}}"));
        CHECK(parts.size() == 2);
        CHECK(parts[0] == "{");
        CHECK(parts[1] == "}");
    }

    SUBCASE("only format field") {
        auto parts = parse_format(S("{abc}"));
        CHECK(parts.size() == 1);
        CHECK(parts[0] == "{abc}");
    }

    SUBCASE("field at beginning and end") {
        auto parts = parse_format(S("{a}middle{b}"));
        CHECK(parts.size() == 3);
        CHECK(parts[0] == "{a}");
        CHECK(parts[1] == "middle");
        CHECK(parts[2] == "{b}");
    }
}

TEST_CASE("fmt torture") {
    SUBCASE("KAStr::fmt - basic formatting") {
        CHECK(KAStr("hello").fmt() == "hello");
        CHECK(KAStr("hi {}").fmt(42) == "hi 42");
        CHECK(KAStr("sum: {} + {} = {}").fmt(1, 2, 3) == "sum: 1 + 2 = 3");
    }

    SUBCASE("KAStr::fmt - integer base formatting") {
        CHECK(KAStr("dec: {:d}").fmt(42) == "dec: 42");
        CHECK(KAStr("hex: {:x}").fmt(255) == "hex: ff");
        CHECK(KAStr("HEX: {:X}").fmt(255) == "HEX: FF");
        CHECK(KAStr("bin: {:b}").fmt(5) == "bin: 101");
    }

    SUBCASE("KAStr::fmt - to_string fallback") {
        CHECK(KAStr("bool: {}").fmt(true) == "bool: true");
        CHECK(KAStr("vec: {}").fmt(std::vector<int>{1, 2, 3}) == "vec: [1, 2, 3]");
    }

    SUBCASE("KAStr::fmt - brace escaping") {
        CHECK(KAStr("{{}}").fmt() == "{}");
        CHECK(KAStr("a {{ b }} c").fmt() == "a { b } c");
        CHECK(KAStr("x {{{}}}").fmt(42) == "x {42}");
    }

    SUBCASE("KAStr::fmt - format errors") {
        CHECK_THROWS_AS(KAStr("hello {").fmt(42), std::invalid_argument);
        CHECK_THROWS_AS(KAStr("hi }{").fmt(42), std::invalid_argument);
        CHECK_THROWS_AS(KAStr("sum: {} + {} = {}").fmt(1, 2), std::invalid_argument);

        CHECK_THROWS_AS(KAStr("value: {:z}").fmt(42), std::invalid_argument); // unsupported spec
        CHECK_THROWS_AS(KAStr("value: {").fmt(42), std::invalid_argument);    // unmatched {
        CHECK_THROWS_AS(KAStr("value: }").fmt(42), std::invalid_argument);    // unmatched }
        CHECK_THROWS_AS(KAStr("value: {} {}").fmt(1), std::invalid_argument); // not enough args
    }
}
