#pragma once

#include "./kastr.hpp"

namespace kastring {
class StyledKAStr {
    KAStr text_;
    std::string fg_code_;
    std::string bg_code_;
    bool bold_ = false;
    bool underline_ = false;
    bool italic_ = false;

  public:
    StyledKAStr() = default;

    explicit StyledKAStr(const KAStr& str) : text_(str) {}

    // 样式标志
    StyledKAStr& bold() {
        bold_ = true;
        return *this;
    }

    StyledKAStr& italic() {
        italic_ = true;
        return *this;
    }

    StyledKAStr& underline() {
        underline_ = true;
        return *this;
    }

    StyledKAStr& color(int ansi_code) {
        fg_code_ = std::to_string(ansi_code);
        return *this;
    }

    StyledKAStr& background(int ansi_code) {
        bg_code_ = std::to_string(ansi_code);
        return *this;
    }

    StyledKAStr& color_rgb(uint8_t r, uint8_t g, uint8_t b) {
        fg_code_ = "38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b);
        return *this;
    }

    StyledKAStr& background_rgb(uint8_t r, uint8_t g, uint8_t b) {
        bg_code_ = "48;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b);
        return *this;
    }

    // 前景色（普通）
    StyledKAStr& black() {
        return color(30);
    }

    StyledKAStr& red() {
        return color(31);
    }

    StyledKAStr& green() {
        return color(32);
    }

    StyledKAStr& yellow() {
        return color(33);
    }

    StyledKAStr& blue() {
        return color(34);
    }

    StyledKAStr& magenta() {
        return color(35);
    }

    StyledKAStr& cyan() {
        return color(36);
    }

    StyledKAStr& white() {
        return color(37);
    }

    // 背景色（普通）
    StyledKAStr& on_black() {
        return background(40);
    }

    StyledKAStr& on_red() {
        return background(41);
    }

    StyledKAStr& on_green() {
        return background(42);
    }

    StyledKAStr& on_yellow() {
        return background(43);
    }

    StyledKAStr& on_blue() {
        return background(44);
    }

    StyledKAStr& on_magenta() {
        return background(45);
    }

    StyledKAStr& on_cyan() {
        return background(46);
    }

    StyledKAStr& on_white() {
        return background(47);
    }

    KAString to_ansi() const;
    KAString own() const;
    friend std::ostream& operator<<(std::ostream& os, const StyledKAStr& s);
};

} // namespace kastring
