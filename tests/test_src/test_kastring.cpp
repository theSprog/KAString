#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <unordered_map>
#include <doctest/doctest.h>
#include "../../include/kastring.hpp"
#include "../../include/tail.hpp"

using namespace kastring;

TEST_CASE("KAString basic ASCII-only operations") {
    SUBCASE("empty string") {
        KAString s;
        CHECK(s.empty());
        CHECK(s.byte_size() == 0);
        CHECK(s.char_size() == 0);
        CHECK(s == "");
    }

    SUBCASE("construct from c-string and std::string") {
        KAString s1("hello");
        KAString s2(std::string("world"));

        CHECK(s1.byte_size() == 5);
        CHECK(s2.byte_size() == 5);

        CHECK(s1[0] == 'h');
        CHECK(s2[4] == 'd');

        CHECK(s1 == "hello");
        CHECK(s2 == "world");
    }

    SUBCASE("initializer list construction") {
        KAString s = {'A', 'B', 'C'};
        CHECK(s.byte_size() == 3);
        CHECK(s[0] == 'A');
        CHECK(s[2] == 'C');
        CHECK(s.byte_at(1) == 'B');
        CHECK(s.byte_at(1) == s.char_at(1));
        s.clear();
        CHECK(s.byte_size() == 0);
    }

    SUBCASE("own ownership") {
        KAStr s = "hello world";
        KAString s_own = s.own();
        CHECK(s_own == "hello world");
    }

    SUBCASE("output to ostream") {
        KAString s("print");
        std::ostringstream oss;
        oss << s;
        CHECK(oss.str() == "print");

        auto s2 = (std::ostringstream{} << KAString("ab\0cd", 5)).str();
        CHECK(s2 == std::string("ab\0cd", 5)); // 不能使用字面量 "ab\0cd", 会被截断
    }
}

TEST_CASE("KAString mutable interface works as expected") {
    KAString s("hello");

    SUBCASE("mutable operator[]") {
        s[0] = 'H';
        s[4] = 'O';
        CHECK(s == "HellO");
    }

    SUBCASE("modify via begin()/end()") {
        for (Byte& b : s) {
            b = static_cast<Byte>(std::toupper(b));
        }
        CHECK(s == "HELLO");
    }

    SUBCASE("reverse iteration and mutation") {
        auto it = s.rbegin();
        *it = '!';
        ++it;
        *it = 'O';
        CHECK(s == "helO!");
    }

    SUBCASE("out-of-bound access throws") {
        CHECK_THROWS_AS(s[100] = 'x', std::out_of_range);
        CHECK_THROWS_AS(s.byte_at(100), std::out_of_range);
    }
}

TEST_CASE("KAString operator overloads work correctly") {
    KAString a("hello");
    KAString b("world");
    KAString empty;

    SUBCASE("operator== and operator!=") {
        CHECK(a == "hello");
        CHECK("hello" == a);
        CHECK_FALSE(a == "hellO");
        CHECK_FALSE("HELLO" == a);

        CHECK(a != "hellO");
        CHECK("HELLO" != a);

        CHECK(a == std::string("hello"));
        CHECK(std::string("hello") == a);
        CHECK(a != std::string("hell"));
        CHECK(std::string("hell") != a);

        CHECK(empty == "");
        CHECK("" == empty);
        CHECK(empty != "nonempty");
    }

    SUBCASE("operator+ KAString + KAString") {
        KAString c = a + b;
        CHECK(c == "helloworld");
    }

    SUBCASE("operator+ KAString + const char*") {
        KAString c = a + "!";
        CHECK(c == "hello!");
    }

    SUBCASE("operator+ const char* + KAString") {
        KAString c = "Say " + a;
        CHECK(c == "Say hello");
    }

    SUBCASE("operator+ KAString + std::string") {
        std::string suffix = "!";
        KAString c = a + suffix;
        CHECK(c == "hello!");
    }

    SUBCASE("operator+ std::string + KAString") {
        std::string prefix = "Say ";
        KAString c = prefix + a;
        CHECK(c == "Say hello");
    }

    SUBCASE("operator+ KAString + char") {
        KAString c = a + '!';
        CHECK(c == "hello!");
    }

    SUBCASE("operator+ char + KAString") {
        KAString c = '*' + a;
        CHECK(c == "*hello");
    }

    SUBCASE("operator+= with KAString") {
        KAString s = a;
        s += b;
        CHECK(s == "helloworld");
    }

    SUBCASE("operator+= with const char*") {
        KAString s = a;
        s += "!";
        CHECK(s == "hello!");
    }

    SUBCASE("operator+= with std::string") {
        KAString s = a;
        std::string suffix = " world";
        s += suffix;
        CHECK(s == "hello world");
    }

    SUBCASE("operator+= with char") {
        KAString s = a;
        s += '!';
        CHECK(s == "hello!");
    }

    SUBCASE("operator+= with KAStr") {
        KAString s = a;
        s += KAStr("!");
        CHECK(s == "hello!");
    }
}

TEST_CASE("KAString compare, operator< and std::hash") {
    KAString a("apple");
    KAString b("banana");
    KAString a2("apple");
    KAString empty;

    SUBCASE("compare behaves like strcmp") {
        CHECK(a.compare(b) < 0);
        CHECK(b.compare(a) > 0);
        CHECK(a.compare(a2) == 0);
        CHECK(a.compare(empty) > 0);
        CHECK(empty.compare(a) < 0);
        CHECK(empty.compare(empty) == 0);
    }

    SUBCASE("operator< enables sorting") {
        std::set<KAString> sorted = {b, a, a2, empty};
        std::vector<std::string> expected = {"", "apple", "banana"};

        std::vector<std::string> actual;
        for (const auto& s : sorted) {
            actual.push_back(s);
        }

        CHECK(actual == expected);
    }

    SUBCASE("std::hash supports unordered_map") {
        std::unordered_map<KAString, int> map;
        map[a] = 1;
        map[b] = 2;
        map[empty] = 0;

        CHECK(map[a] == 1);
        CHECK(map[b] == 2);
        CHECK(map[a2] == 1); // same content
        CHECK(map[empty] == 0);

        // Sanity: keys with different content must not collide
        CHECK(map.find("nonexistent") == map.end());
    }
}

TEST_CASE("KAString::chop and chopped behavior") {
    SUBCASE("basic chop") {
        KAString s("hello.cpp");
        s.chop(4);
        CHECK(s == "hello");

        s.chop(5); // remove "hello"
        CHECK(s == "");

        s.chop(10); // chop beyond length
        CHECK(s == "");
    }

    SUBCASE("chop on empty string") {
        KAString s("");
        s.chop(0);
        CHECK(s == "");

        s.chop(1); // still safe
        CHECK(s == "");
    }

    SUBCASE("basic chopped") {
        KAString s("hello.cpp");
        KAString r1 = s.chopped(4);
        CHECK(r1 == "hello");
        CHECK(s == "hello.cpp"); // original should remain

        KAString r2 = r1.chopped(5);
        CHECK(r2 == "");

        KAString r3 = r2.chopped(1); // excessive chopped
        CHECK(r3 == "");
    }

    SUBCASE("chopped on empty string") {
        KAString s("");
        KAString r = s.chopped(0);
        CHECK(r == "");

        r = s.chopped(10);
        CHECK(r == "");
    }

    SUBCASE("chop/chopped equivalence in side effects") {
        KAString orig("abc.def");
        KAString sliced = orig.chopped(4);
        CHECK(sliced == "abc");

        orig.chop(4);
        CHECK(orig == "abc");
    }
}

TEST_CASE("KAString::fill behavior") {


    SUBCASE("fill with default size (preserve current size)") {
        KAString s("hello");
        s.fill('x');
        CHECK(s.byte_size() == 5);
        CHECK(s == "xxxxx");
    }

    SUBCASE("fill with explicit size smaller than original") {
        KAString s("hello world");
        s.fill('z', 3);
        CHECK(s.byte_size() == 3);
        CHECK(s == "zzz");
    }

    SUBCASE("fill with size larger than original") {
        KAString s("ok");
        s.fill('a', 5);
        CHECK(s.byte_size() == 5);
        CHECK(s == "aaaaa");
    }

    SUBCASE("fill with size = 0") {
        KAString s("nonempty");
        s.fill('x', 0);
        CHECK(s.byte_size() == 0);
        CHECK(s == "");
    }

    SUBCASE("fill empty string") {
        KAString s("");
        s.fill('x'); // size == 0 → no effect
        CHECK(s.byte_size() == 0);
    }

    SUBCASE("chained fill") {
        KAString s("abc");
        s.fill('k').fill('m');
        CHECK(s == "mmm");
    }
}

TEST_CASE("KAString::prepend torture") {
    SUBCASE("prepend to empty string") {
        KAString s;
        s.prepend("abc");
        CHECK(s == "abc");
    }

    SUBCASE("prepend empty string") {
        KAString s("world");
        s.prepend("");
        CHECK(s == "world");
    }

    SUBCASE("prepend both empty") {
        KAString s;
        s.prepend("");
        CHECK(s == "");
    }

    SUBCASE("prepend multiple times") {
        KAString s("base");
        s.prepend("B").prepend("A").prepend("X");
        CHECK(s == "XABbase");
    }

    SUBCASE("prepend long string to trigger heap promotion") {
        KAString s("short");
        s.prepend(std::string(100, 'x')); // 100 chars
        CHECK(s.byte_size() == 105);
        CHECK(s.starts_with(std::string(100, 'x')));
    }
}

TEST_CASE("KAString::repeated torture") {
    SUBCASE("repeat zero times") {
        KAString s("data");
        auto r = s.repeated(0);
        CHECK(r.empty());
    }

    SUBCASE("repeat once") {
        KAString s("data");
        auto r = s.repeated(1);
        CHECK(r == "data");
    }

    SUBCASE("repeat multiple times") {
        KAString s("x");
        auto r = s.repeated(5);
        CHECK(r == "xxxxx");
    }

    SUBCASE("repeat with empty string") {
        KAString s;
        auto r = s.repeated(100);
        CHECK(r.empty());
    }

    SUBCASE("repeat negative (defensive)") {
        KAString s("oops");
        auto r = s.repeated(-3);
        CHECK(r.empty()); // no UB
    }
}

TEST_CASE("KAString::remove series torture") {
    SUBCASE("remove single occurrence") {
        KAString s("abcde");
        s.remove("cd");
        CHECK(s == "abe");
        s.remove_first();
        CHECK(s == "be");
        s.remove_last();
        CHECK(s == "b");
    }

    SUBCASE("remove all repeated matches") {
        KAString s("aaa");
        s.remove("a");
        CHECK(s.empty());
    }

    SUBCASE("remove with overlap") {
        KAString s("aaaaa");
        s.remove("aa"); // non-overlapping by default → removes at 0 and 2
        CHECK(s == "a");
    }

    SUBCASE("remove nothing (no match)") {
        KAString s("abcdef");
        s.remove("xyz");
        CHECK(s == "abcdef");
    }

    SUBCASE("remove empty string") {
        KAString s("data");
        s.remove("");
        CHECK(s == "data");
    }

    SUBCASE("remove at out-of-bounds") {
        KAString s("abc");
        CHECK_THROWS_AS(s.remove_at(10), std::out_of_range);
    }

    SUBCASE("remove_at valid") {
        KAString s("abc");
        s.remove_at(1);
        CHECK(s == "ac");
    }

    SUBCASE("remove_first on empty string") {
        KAString s;
        CHECK_THROWS_AS(s.remove_first(), std::out_of_range);
    }

    SUBCASE("remove_last on empty string") {
        KAString s;
        CHECK_THROWS_AS(s.remove_last(), std::out_of_range);
    }

    SUBCASE("remove_if only digits") {
        KAString s("abc123xyz");
        s.remove_if([](char c) { return std::isdigit(c); });
        CHECK(s == "abcxyz");
        s.append("qw1er2tyu3iop4as5df6gh7jkl");
        s.remove_if([](char c) { return std::isdigit(c); });
        CHECK(s == "abcxyzqwertyuiopasdfghjkl");
    }

    SUBCASE("remove_if delete all") {
        KAString s("111");
        s.remove_if([](char) { return true; });
        CHECK(s.empty());
    }

    SUBCASE("remove_if delete nothing") {
        KAString s("abc");
        s.remove_if([](char) { return false; });
        CHECK(s == "abc");
    }

    SUBCASE("remove large content") {
        KAString s(std::string(10000, 'x') + "Y" + std::string(10000, 'x'));
        s.remove("Y");
        CHECK(s.byte_size() == 20000);
    }
}

TEST_CASE("KAString::replace(pos, len, after) torture") {
    SUBCASE("normal replace middle range") {
        KAString s("abcXYZdef");
        s.replace(3, 3, "123");
        CHECK(s == "abc123def");
    }

    SUBCASE("replace with shorter content") {
        KAString s("abcdefgh");
        s.replace(2, 4, "x");
        CHECK(s == "abxgh");
    }

    SUBCASE("replace with longer content") {
        KAString s("hello");
        s.replace(1, 2, "ABCDE");
        CHECK(s == "hABCDElo");
    }

    SUBCASE("replace with empty") {
        KAString s("abcdef");
        s.replace(2, 3, "");
        CHECK(s == "abf");
    }

    SUBCASE("replace with same length → in-place") {
        KAString s("abcde");
        s.replace(1, 3, "XYZ"); // len == 3
        CHECK(s == "aXYZe");
    }

    SUBCASE("replace at start") {
        KAString s("12345");
        s.replace(0, 2, "AB");
        CHECK(s == "AB345");
    }

    SUBCASE("replace at end") {
        KAString s("12345");
        s.replace(3, 2, "ZZZ");
        CHECK(s == "123ZZZ");
    }

    SUBCASE("replace out-of-bounds") {
        KAString s("abc");
        CHECK_THROWS_AS(s.replace(4, 1, "x"), std::out_of_range);
    }

    SUBCASE("replace length overshoot") {
        KAString s("abc");
        CHECK_THROWS_AS(s.replace(2, 99, "x"), std::out_of_range);
    }
}

TEST_CASE("KAString::replace_all torture") {
    SUBCASE("replace all matches") {
        KAString s("aabbccbbdd");
        s.replace_all("bb", "XX");
        CHECK(s == "aaXXccXXdd");
    }

    SUBCASE("replace nothing") {
        KAString s("abc");
        s.replace_all("xyz", "ZZ");
        CHECK(s == "abc");
    }

    SUBCASE("replace with empty string") {
        KAString s("xxyxx");
        s.replace_all("x", "");
        CHECK(s == "y");
    }

    SUBCASE("replace empty → should skip") {
        KAString s("abc");
        s.replace_all("", "X");
        CHECK(s == "abc"); // no infinite loop!
    }

    SUBCASE("replace with same content → no-op") {
        KAString s("abcabc");
        s.replace_all("abc", "abc");
        CHECK(s == "abcabc");
    }

    SUBCASE("replace overlapping safe") {
        KAString s("aaaaa");
        s.replace_all("aa", "x");
        CHECK(s == "xxa"); // non-overlapping match
    }
}

TEST_CASE("KAString::replace_first / replace_last torture") {
    SUBCASE("replace_first basic") {
        KAString s("abcabcabc");
        s.replace_first("abc", "X");
        CHECK(s == "Xabcabc");
    }

    SUBCASE("replace_last basic") {
        KAString s("abcabcabc");
        s.replace_last("abc", "X");
        CHECK(s == "abcabcX");
    }

    SUBCASE("replace_first no match") {
        KAString s("abc");
        s.replace_first("xyz", "Q");
        CHECK(s == "abc");
    }

    SUBCASE("replace_last no match") {
        KAString s("abc");
        s.replace_last("xyz", "Q");
        CHECK(s == "abc");
    }

    SUBCASE("replace_first == last") {
        KAString s("abc");
        s.replace_first("abc", "x");
        CHECK(s == "x");
    }

    SUBCASE("replace_last with empty") {
        KAString s("hello world");
        s.replace_last("world", "");
        CHECK(s == "hello ");
    }
}

TEST_CASE("KAString::replace_if torture") {
    SUBCASE("replace digit with underscore") {
        KAString s("a1b2c3");
        s.replace_char_if([](char c) { return std::isdigit(c); }, "_");
        CHECK(s == "a_b_c_");
    }

    SUBCASE("replace vowel with string") {
        KAString s("hello");
        s.replace_char_if([](char c) { return std::string("aeiou").find(c) != std::string::npos; }, "!");
        CHECK(s == "h!ll!");
    }

    SUBCASE("replace all with same char") {
        KAString s("xxxxx");
        s.replace_char_if([](char) { return true; }, "y");
        CHECK(s == "yyyyy");
    }

    SUBCASE("replace none") {
        KAString s("abc");
        s.replace_char_if([](char) { return false; }, "!");
        CHECK(s == "abc");
    }

    SUBCASE("replace with longer string") {
        KAString s("a1b2");
        s.replace_char_if([](char c) { return std::isdigit(c); }, "[num]");
        CHECK(s == "a[num]b[num]");
    }

    SUBCASE("replace with empty string = remove") {
        KAString s("123abc456");
        s.replace_char_if([](char c) { return std::isdigit(c); }, "");
        CHECK(s == "abc");
    }

    SUBCASE("replace with self-character") {
        KAString s("abc");
        s.replace_char_if([](char c) { return c == 'b'; }, KAStr("b"));
        CHECK(s == "abc"); // no-op optimization
    }

    SUBCASE("replace multi-byte safe expansion") {
        KAString s("hello");
        s.replace_char_if([](char c) { return c == 'o'; }, "\xe4\xb8\xad"); // U+4E2D, "中"
        CHECK(s.byte_size() == 7);                                          // "hell中"
    }

    SUBCASE("empty input") {
        KAString s = "";
        s.replace_char_if([](char) { return true; }, "_");
        CHECK(s == "");
    }

    SUBCASE("no match") {
        KAString s = "abc";
        s.replace_char_if([](char c) { return c == 'x'; }, "_");
        CHECK(s == "abc");
    }

    SUBCASE("all match, empty replacement = erase") {
        KAString s = "xxxx";
        s.replace_char_if([](char c) { return c == 'x'; }, "");
        CHECK(s == "");
    }

    SUBCASE("replace every vowel") {
        KAString s = "aebcdf";
        s.replace_char_if([](char c) { return std::string("aeiou").find(c) != std::string::npos; }, "_");
        CHECK(s == "__bcdf");
    }

    SUBCASE("replace with multi-byte string") {
        KAString s = "abc";
        s.replace_char_if([](char c) { return c == 'b'; }, "--");
        CHECK(s == "a--c");
    }

    SUBCASE("max_replace == 1") {
        KAString s = "xxyyzz";
        s.replace_char_if([](char c) { return c == 'y' || c == 'z'; }, "_", 1);
        CHECK(s == "xx_yzz");
    }
}

TEST_CASE("KAString::replace_groups_if torture") {
    SUBCASE("no match → should no-op") {
        KAString s("abcDEF");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "[num]");
        CHECK(s == "abcDEF");
    }

    SUBCASE("entire string is one group") {
        KAString s("123456");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "[num]");
        CHECK(s == "[num]");
    }

    SUBCASE("multiple digit groups") {
        KAString s("abc123xyz4567def");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "[num]");
        CHECK(s == "abc[num]xyz[num]def");
    }

    SUBCASE("mixed groups and singles") {
        KAString s("a1b22c333d");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "_");
        CHECK(s == "a_b_c_d");
    }

    SUBCASE("interleaved single-char groups") {
        KAString s("1a2b3c");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "!");
        CHECK(s == "!a!b!c");
    }

    SUBCASE("leading and trailing groups") {
        KAString s("123abc456");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "[#]");
        CHECK(s == "[#]abc[#]");
    }

    SUBCASE("all matched but interspersed") {
        KAString s("1 2 3");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "_");
        CHECK(s == "_ _ _");
    }

    SUBCASE("replace groups with empty string = remove") {
        KAString s("a11b22c33");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "");
        CHECK(s == "abc");
    }

    SUBCASE("replace group of whitespace") {
        KAString s("a  \tb\t\nc");
        s.replace_groups_if([](char c) { return std::isspace(static_cast<unsigned char>(c)); }, "_");
        CHECK(s == "a_b_c");
    }

    SUBCASE("consecutive non-overlapping") {
        KAString s("#%%#!#abc!!!xyz%%%");
        s.replace_groups_if([](char c) { return c == '#' || c == '!' || c == '%'; }, "*");
        CHECK(s == "*abc*xyz*");
    }

    SUBCASE("alternating matched/unmatched") {
        KAString s("1a2b3c4");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "_");
        CHECK(s == "_a_b_c_");
    }

    SUBCASE("replace same length as original group") {
        KAString s("a111b222");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "XYZ");
        CHECK(s == "aXYZbXYZ");
    }

    SUBCASE("replace with longer string than group") {
        KAString s("abc123xyz");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "[NUMBER]");
        CHECK(s == "abc[NUMBER]xyz");
    }

    SUBCASE("replace with shorter string than group") {
        KAString s("abc123xyz456");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "X");
        CHECK(s == "abcXxyzX");
    }

    SUBCASE("complex mix of group lengths") {
        KAString s("1ab22cd333ef4");
        s.replace_groups_if([](char c) { return std::isdigit(c); }, "*");
        CHECK(s == "*ab*cd*ef*");
    }

    SUBCASE("replace space groups with single _") {
        KAString s = "a  b    c";
        s.replace_groups_if([](char c) { return c == ' '; }, "_");
        CHECK(s == "a_b_c");
    }

    SUBCASE("empty replacement (delete groups)") {
        KAString s = "aaabbbbccc";
        s.replace_groups_if([](char c) { return c == 'b'; }, "");
        CHECK(s == "aaaccc");
    }

    SUBCASE("group at beginning") {
        KAString s = "   abc";
        s.replace_groups_if([](char c) { return c == ' '; }, "_");
        CHECK(s == "_abc");
    }

    SUBCASE("group at end") {
        KAString s = "abc   ";
        s.replace_groups_if([](char c) { return c == ' '; }, "_");
        CHECK(s == "abc_");
    }

    SUBCASE("consecutive groups") {
        KAString s = "a   b     c";
        s.replace_groups_if([](char c) { return c == ' '; }, "_", 1);
        CHECK(s == "a_b     c");
    }
}

TEST_CASE("replace_char_if rev: torture cases") {
    KAString s;

    SUBCASE("replace last vowel only") {
        s = "hello world";
        s.rreplace_char_if([](char c) { return std::string("aeiou").find(c) != std::string::npos; }, "*", 1);
        CHECK(s == "hello w*rld");
    }

    SUBCASE("multiple rev replace") {
        s = "x--x--x";
        s.rreplace_char_if([](char c) { return c == 'x'; }, "Y");
        CHECK(s == "Y--Y--Y");
    }

    SUBCASE("multi-byte rev replace") {
        s = "xYx";
        s.rreplace_char_if([](char c) { return c == 'x'; }, "[!]");
        CHECK(s == "[!]Y[!]");
    }
}

TEST_CASE("replace_groups_if rev: torture cases") {
    SUBCASE("rev replace last space group") {
        KAString s = "a  b    c";
        s.rreplace_groups_if([](char c) { return c == ' '; }, "_", 1);
        CHECK(s == "a  b_c");
    }

    SUBCASE("rev replace all tabs") {
        KAString s = "a\t\tb\tc";
        s.rreplace_groups_if([](char c) { return c == '\t'; }, "-", 2);
        CHECK(s == "a-b-c");
    }

    SUBCASE("entire string matched group") {
        KAString s = "     ";
        s.rreplace_groups_if([](char c) { return c == ' '; }, "X");
        CHECK(s == "X");
    }

    SUBCASE("replacement longer than group") {
        KAString s = "abbbcd";
        s.rreplace_groups_if([](char c) { return c == 'b'; }, "XYZ");
        CHECK(s == "aXYZcd");
    }

    SUBCASE("replacement shorter than group") {
        KAString s = "abbbbbcd";
        s.rreplace_groups_if([](char c) { return c == 'b'; }, "!");
        CHECK(s == "a!cd");
    }
}

TEST_CASE("KAString::replace_count torture") {
    SUBCASE("replace 0 times (should no-op)") {
        KAString s("abcabcabc");
        s.replace_count("abc", "X", 0);
        CHECK(s == "abcabcabc");
    }

    SUBCASE("replace exactly 1 time") {
        KAString s("abcabcabc");
        s.replace_count("abc", "X", 1);
        CHECK(s == "Xabcabc");
    }

    SUBCASE("replace exactly 2 times") {
        KAString s("abcabcabc");
        s.replace_count("abc", "X", 2);
        CHECK(s == "XXabc");
    }

    SUBCASE("replace more than needed (infinite)") {
        KAString s("abcabcabc");
        s.replace_count("abc", "X", 1000);
        CHECK(s == "XXX");
    }

    SUBCASE("replace with same string → no-op") {
        KAString s("abcabc");
        s.replace_count("abc", "abc", 3);
        CHECK(s == "abcabc");
    }

    SUBCASE("replace with empty → acts as remove") {
        KAString s("abcabcabc");
        s.replace_count("abc", "", 2);
        CHECK(s == "abc");
    }

    SUBCASE("replace non-match → no-op") {
        KAString s("abcabc");
        s.replace_count("xyz", "Q", 3);
        CHECK(s == "abcabc");
    }
}

TEST_CASE("KAString::replace_count case-insensitive torture") {
    SUBCASE("basic ci match") {
        KAString s("ABCabcAbC");
        s.replace_count("abc", "X", 2, false);
        CHECK(s == "XXAbC"); // 前两个替换
    }

    SUBCASE("ci match with empty after") {
        KAString s("abcABCabc");
        s.replace_count("ABC", "", 2, false);
        CHECK(s == "abc");
    }

    SUBCASE("ci match with different cases") {
        KAString s("AbCdEf");
        s.replace_count("cde", "x", 1, false);
        CHECK(s == "Abxf");
    }

    SUBCASE("ci mismatch under sensitive mode") {
        KAString s("abcABC");
        s.replace_count("ABC", "x", 2, true);
        CHECK(s == "abcx");
    }

    SUBCASE("ci no match because of strict mode") {
        KAString s("Abc");
        s.replace_count("abc", "x", 1, true);
        CHECK(s == "Abc"); // 没替换
    }
}

TEST_CASE("KAString::rreplace_count torture") {
    SUBCASE("rreplace 1 from back") {
        KAString s("abcabcabc");
        s.rreplace_count("abc", "Z", 1);
        CHECK(s == "abcabcZ");
    }

    SUBCASE("rreplace 2 from back") {
        KAString s("abcabcabc");
        s.rreplace_count("abc", "Z", 2);
        CHECK(s == "abcZZ");
    }

    SUBCASE("rreplace all") {
        KAString s("abcabcabc");
        s.rreplace_count("abc", "!", 1000);
        CHECK(s == "!!!");
    }

    SUBCASE("rreplace with longer string") {
        KAString s("abcabc");
        s.rreplace_count("abc", "___", 2);
        CHECK(s == "______");
    }

    SUBCASE("rreplace with empty string") {
        KAString s("123123");
        s.rreplace_count("123", "", 1);
        CHECK(s == "123");
    }

    SUBCASE("rreplace nothing") {
        KAString s("abcabc");
        s.rreplace_count("xyz", "!", 1);
        CHECK(s == "abcabc");
    }

    SUBCASE("rreplace count == 0") {
        KAString s("abcabc");
        s.rreplace_count("abc", "!", 0);
        CHECK(s == "abcabc");
    }
}

TEST_CASE("KAString::rreplace_count case-insensitive torture") {
    SUBCASE("rreplace case-insensitive match") {
        KAString s("abcAbCabC");
        s.rreplace_count("abc", "!", 2, false);
        CHECK(s == "abc!!");
    }

    SUBCASE("rreplace with longer") {
        KAString s("xABCxabcx");
        s.rreplace_count("abc", "###", 1, false);
        CHECK(s == "xABCx###x");
    }

    SUBCASE("rreplace with empty case-insensitive") {
        KAString s("ABCabcAbC");
        s.rreplace_count("ABC", "", 2, false);
        CHECK(s == "ABC");
    }

    SUBCASE("rreplace with partial match") {
        KAString s("AaBBcc");
        s.rreplace_count("bb", "!", 1, false);
        CHECK(s == "Aa!cc");
    }
}

TEST_CASE("KAString::replace_nth torture") {
    SUBCASE("replace 0th match") {
        KAString s("abcabcabc");
        s.replace_nth("abc", "!", 0);
        CHECK(s == "!abcabc");
    }

    SUBCASE("replace 1st match") {
        KAString s("abcabcabc");
        s.replace_nth("abc", "!", 1);
        CHECK(s == "abc!abc");
    }

    SUBCASE("replace nth > match count") {
        KAString s("abcabc");
        s.replace_nth("abc", "!", 5);
        CHECK(s == "abcabc");
    }

    SUBCASE("replace_nth with empty after = remove") {
        KAString s("xabcabcabcx");
        s.replace_nth("abc", "", 1);
        CHECK(s == "xabcabcx");
    }

    SUBCASE("replace_nth with longer") {
        KAString s("one two three two one");
        s.replace_nth("two", "2two2", 1);
        CHECK(s == "one two three 2two2 one");
    }
    SUBCASE("replace_nth with longer") {
        KAString s("aaaaaaaa");
        CHECK(s.replace_nth("aa", "X", 2) == "aaaaXaa");
    }

    SUBCASE("replace_nth with longer") {
        KAString s("abcdefg");
        CHECK(s.replace_first("abc", "x") == s.replace_nth("abc", "x", 0));
    }
}

TEST_CASE("KAString::replace_nth case-insensitive torture") {
    SUBCASE("nth match by ignoring case") {
        KAString s("abcABCAbc");
        s.replace_nth("abc", "!", 2, false);
        CHECK(s == "abcABC!");
    }

    SUBCASE("nth match with mismatch in strict") {
        KAString s("abcABCabc");
        s.replace_nth("abc", "!", 1, true); // only match lowercase
        CHECK(s == "abcABC!");
    }

    SUBCASE("nth match with multi-case sequence") {
        KAString s("aBcABcABC");
        s.replace_nth("abc", "X", 1, false);
        CHECK(s == "aBcXABC");
    }

    SUBCASE("nth no match") {
        KAString s("aaa");
        s.replace_nth("abc", "x", 0, false);
        CHECK(s == "aaa");
    }
}

TEST_CASE("KAString::rreplace_nth torture") {
    SUBCASE("replace last match (0th from back)") {
        KAString s("abcabcabc");
        s.rreplace_nth("abc", "Z", 0);
        CHECK(s == "abcabcZ");
    }

    SUBCASE("replace second-to-last match (1st from back)") {
        KAString s("abcabcabc");
        s.rreplace_nth("abc", "Z", 1);
        CHECK(s == "abcZabc");
    }

    SUBCASE("replace nth too large") {
        KAString s("abcabcabc");
        s.rreplace_nth("abc", "Z", 10);
        CHECK(s == "abcabcabc");
    }

    SUBCASE("rreplace_nth with longer replacement") {
        KAString s("a a a");
        s.rreplace_nth("a", "AAA", 1);
        CHECK(s == "a AAA a");
    }

    SUBCASE("rreplace_nth with empty") {
        KAString s("del del del");
        s.rreplace_nth("del", "", 1);
        CHECK(s == "del  del");
    }

    SUBCASE("rreplace_nth same as original → no-op") {
        KAString s("foo bar foo");
        s.rreplace_nth("foo", "foo", 0);
        CHECK(s == "foo bar foo");
    }
}

TEST_CASE("KAString::rreplace_nth case-insensitive torture") {
    SUBCASE("rreplace_nth case-insensitive") {
        KAString s("AbCabcABC");
        s.rreplace_nth("abc", "!", 0, false);
        CHECK(s == "AbCabc!");
    }

    SUBCASE("rreplace_nth finds correct nth from back") {
        KAString s("aBcABcABC");
        s.rreplace_nth("abc", "Z", 1, false);
        CHECK(s == "aBcZABC");
    }

    SUBCASE("rreplace_nth with empty result") {
        KAString s("abcABCabc");
        s.rreplace_nth("abc", "", 2, false);
        CHECK(s == "ABCabc");
    }

    SUBCASE("rreplace_nth with strict fails") {
        KAString s("AbCabc");
        s.rreplace_nth("abc", "Z", 1, true);
        CHECK(s == "AbCabc");
    }

    SUBCASE("ci match but equal content") {
        KAString s("ABCabc");
        s.replace_all("abc", "ABC", false); // logical equal
        CHECK(s == "ABCABC");
    }
}

TEST_CASE("KAString::ljust / rjust torture") {
    SUBCASE("ljust shorter → pad right") {
        KAString s("hi");
        CHECK(s.ljust(5, '_') == "hi___");
        CHECK(s.ljust(5, '*', true) == "hi***");
    }

    SUBCASE("rjust shorter → pad left") {
        KAString s("hi");
        CHECK(s.rjust(5, '_') == "___hi");
        CHECK(s.rjust(5, '&', true) == "&&&hi");
    }

    SUBCASE("ljust with truncate = true") {
        KAString s("abcdef");
        CHECK(s.ljust(3, '-', true) == "abc");
    }

    SUBCASE("rjust with truncate = true") {
        KAString s("abcdef");
        CHECK(s.rjust(3, '.', true) == "def");
    }

    SUBCASE("ljust equal width → no change") {
        KAString s("abc");
        CHECK(s.ljust(3, '_') == "abc");
    }

    SUBCASE("rjust equal width → no change") {
        KAString s("abc");
        CHECK(s.rjust(3, '*') == "abc");
    }

    SUBCASE("ljust empty string") {
        KAString s("");
        CHECK(s.ljust(4, '!') == "!!!!");
    }

    SUBCASE("rjust empty string") {
        KAString s("");
        CHECK(s.rjust(4, '!') == "!!!!");
    }

    SUBCASE("ljust longer than width (no truncate)") {
        KAString s("abcdef");
        CHECK(s.ljust(3, '.', false) == "abcdef");
    }

    SUBCASE("rjust longer than width (no truncate)") {
        KAString s("abcdef");
        CHECK(s.rjust(3, '.', false) == "abcdef");
    }
}

TEST_CASE("KAString::center(width, fill) torture") {
    SUBCASE("basic centering odd width") {
        KAString s("abc");
        CHECK(s.center(7, '_') == "__abc__");
    }

    SUBCASE("basic centering even width") {
        KAString s("abc");
        CHECK(s.center(6, '.') == ".abc..");
    }

    SUBCASE("center width equals string length") {
        KAString s("hello");
        CHECK(s.center(5, '*') == "hello");
    }

    SUBCASE("center width less than string length") {
        KAString s("abcdef");
        CHECK(s.center(3, '-') == "abcdef"); // no truncate
    }

    SUBCASE("center empty string") {
        KAString s("");
        CHECK(s.center(4, '#') == "####");
    }

    SUBCASE("center width zero") {
        KAString s("abc");
        CHECK(s.center(0, '-') == "abc");
    }

    SUBCASE("center with default fill") {
        KAString s("ok");
        CHECK(s.center(6) == "  ok  "); // default fill is space
    }

    SUBCASE("center odd padding split left-right") {
        KAString s("abc");
        // width = 8, len = 3 → pad = 5 → left = 2, right = 3
        CHECK(s.center(8, '+') == "++abc+++");
    }

    SUBCASE("center on 1-char string") {
        KAString s("x");
        CHECK(s.center(5, '_') == "__x__");
    }

    SUBCASE("center single wide pad") {
        KAString s("abc");
        CHECK(s.center(4, '_') == "abc_");
    }
}

TEST_CASE("KAString::fromNum(int) torture") {
    SUBCASE("fromNum base 10") {
        CHECK(KAString::from_num(123) == "123");
        CHECK(KAString::from_num(-456) == "-456");
    }

    SUBCASE("fromNum base 2") {
        CHECK(KAString::from_num(5, 2) == "101");
    }

    SUBCASE("fromNum base 16") {
        CHECK(KAString::from_num(255, 16) == "ff");
    }

    SUBCASE("fromNum base 36") {
        CHECK(KAString::from_num(1295, 36) == "zz");
    }

    SUBCASE("fromNum invalid base (<2)") {
        CHECK_THROWS_AS(KAString::from_num(10, 1), std::invalid_argument);
    }

    SUBCASE("fromNum invalid base (>36)") {
        CHECK_THROWS_AS(KAString::from_num(10, 37), std::invalid_argument);
    }

    SUBCASE("fromNum zero") {
        CHECK(KAString::from_num(0) == "0");
    }

    SUBCASE("fromNum INT_MIN") {
        KAString int_min = KAString::from_num(std::numeric_limits<int>::min());
        CHECK(int_min.starts_with("-"));
    }

    SUBCASE("fromNum INT_MAX") {
        KAString int_max = KAString::from_num(std::numeric_limits<int>::max());
        CHECK(int_max.byte_size() > 0);
    }
}

TEST_CASE("KAString::fromNum(double) torture") {
    SUBCASE("basic float formatting") {
        CHECK(KAString::from_num(3.14159, 'f', 2) == "3.14");
        CHECK(KAString::from_num(3.14159, 'e', 2).contains("e")); // e.g. "3.14e+00"
    }

    SUBCASE("format 'g' general") {
        CHECK(KAString::from_num(123.456, 'g', 6).contains("123.456"));
        CHECK(KAString::from_num(0.00001234, 'g', 3).contains("e")); // → "1.23e-5"
    }

    SUBCASE("zero and negative numbers") {
        CHECK(KAString::from_num(0.0, 'f', 3) == "0.000");
        CHECK(KAString::from_num(-2.5, 'f', 1) == "-2.5");
    }

    SUBCASE("NaN and Infinity") {
        KAString nan = KAString::from_num(std::nan(""), 'g', 3);
        KAString inf = KAString::from_num(std::numeric_limits<double>::infinity(), 'g', 3);
        bool contain_nan = nan.contains("nan") || nan.contains("NaN");
        CHECK(contain_nan);
        bool contain_inf = inf.contains("inf") || inf.contains("Inf");
        CHECK(contain_inf);
    }

    SUBCASE("invalid format char") {
        CHECK_THROWS_AS(KAString::from_num(3.14, 'x', 2), std::invalid_argument);
    }

    SUBCASE("very large/small values") {
        CHECK(KAString::from_num(1e308, 'g', 6).contains("e"));
        CHECK(KAString::from_num(1e-308, 'e', 2).contains("e"));
    }

    SUBCASE("precision edge cases") {
        CHECK(KAString::from_num(1.99999, 'f', 0) == "2");
        CHECK(KAString::from_num(123.456, 'f', 5).starts_with("123.45"));
    }
}

TEST_CASE("KAString upper/lower/to_upper/to_lower torture") {
    SUBCASE("basic upper") {
        KAString s("abcXYZ");
        CHECK(s.to_upper() == "ABCXYZ");
    }

    SUBCASE("basic lower") {
        KAString s("ABCxyz");
        CHECK(s.to_lower() == "abcxyz");
    }

    SUBCASE("already upper/lower → should not change") {
        KAString upper("HELLO");
        KAString lower("world");
        CHECK(upper.to_upper() == "HELLO");
        CHECK(lower.to_lower() == "world");
    }

    SUBCASE("mixed case in-place") {
        KAString s("aBc123!");
        s.upper_self();
        CHECK(s == "ABC123!");

        s.lower_self();
        CHECK(s == "abc123!");
    }

    SUBCASE("non-alphabetic characters remain") {
        KAString s("1234!@#");
        CHECK(s.to_upper() == "1234!@#");
        CHECK(s.to_lower() == "1234!@#");
    }

    SUBCASE("empty string") {
        KAString s("");
        CHECK(s.to_upper() == "");
        CHECK(s.to_lower() == "");
    }

    SUBCASE("repeated application is idempotent") {
        KAString s("heLLo");
        KAString upper1 = s.to_upper();
        KAString upper2 = upper1.to_upper();
        CHECK(upper1 == upper2);
    }

    SUBCASE("in-place modifies correctly") {
        KAString s("AaZz");
        s.upper_self();
        CHECK(s == "AAZZ");

        s.lower_self();
        CHECK(s == "aazz");
    }
}

TEST_CASE("KAString::simplified torture") {
    SUBCASE("basic space collapse") {
        KAString s("  Hello    World ");
        CHECK(s.simplified() == "Hello World");
    }

    SUBCASE("tabs, newlines, mix") {
        KAString s("\tHello\n   world\r\n  test\t");
        CHECK(s.simplified() == "Hello world test");
    }

    SUBCASE("empty string") {
        KAString s("");
        CHECK(s.simplified() == "");
    }

    SUBCASE("only whitespace") {
        KAString s("   \t\n\r  ");
        CHECK(s.simplified() == "");
    }

    SUBCASE("no whitespace → same result") {
        KAString s("NoWhiteSpace");
        CHECK(s.simplified() == "NoWhiteSpace");
    }

    SUBCASE("multiple segments") {
        KAString s("  this   is\t\ta    test  ");
        CHECK(s.simplified() == "this is a test");
    }

    SUBCASE("simplified is idempotent") {
        KAString s("   a   b   c   ");
        KAString once = s.simplified();
        KAString twice = once.simplified();
        CHECK(once == twice);
    }

    SUBCASE("simplified removes leading/trailing only once") {
        KAString s("   abc   def   ");
        CHECK(s.simplified().starts_with("abc"));
        CHECK(s.simplified().ends_with("def"));
    }
}

TEST_CASE("fmt case") {
    CHECK(KAString("sum = {}").fmt(1 + 2) == "sum = 3");
    CHECK(KAString("bin = {:b}").fmt(5) == "bin = 101");
    CHECK(KAString("vec = {}").fmt(std::vector<int>{1, 2, 3}) == "vec = [1, 2, 3]");
}

TEST_CASE("addtional test case") {
    SUBCASE("reverse") {
        KAString s = "abc";
        s.reverse();
        CHECK(s == "cba");
    }
}
