/****************************************************************************
 Copyright (c) 2016-2027 Jeff Wang <summer_insects@163.com>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 ****************************************************************************/

#include "stringify.h"
#include <cstring>

namespace mahjong {

int parse_hand_tiles(const char *str, size_t len, hand_tiles_t *hand_tiles, tile_t *serving_tile) {
    bool bracket = false;
    pack_t fixed_packs[4];  // 副露
    uint8_t pack_cnt = 0;
    tile_t standing_tiles[14];  // 立牌，包括最新上牌
    uint8_t tile_cnt = 0, tile_max = 14;  // 立牌长度、最大长度
    uint8_t digit_rank[14];  // 数字串
    uint8_t digit_cnt = 0, digit_max = 14;  // 数字串长度、最大长度
    tile_t temp_tile[14];  // 临时牌
    uint8_t temp_cnt = 0, temp_max = 14;  // 临时牌长度、最大长度
    tile_table_t tile_table = { 0 };  // 牌表

    for (size_t k = 0; k < len; ++k) {
        suit_t submit_suit = TILE_SUIT_NONE;  // 待提交的花色
        tile_t submit_honor = 0;  // 待提交的字牌

        char ch = str[k];
        switch (ch) {
        case '1':case '2':case '3':
        case '4':case '5':case '6':
        case '7':case '8':case '9':
            // 添加到数字串
            // 如果在括号里，则不需要考虑临时牌长度
            // 如果在括号外，临时牌已经满时，再添加已经没有意义了，反正这些牌也不会用到
            if ((bracket || temp_cnt < temp_max) && digit_cnt < digit_max) {
                digit_rank[digit_cnt++] = ch - '0';
            }
            break;
        case 'm': submit_suit = TILE_SUIT_CHARACTERS; break;
        case 's': submit_suit = TILE_SUIT_BAMBOO; break;
        case 'p': submit_suit = TILE_SUIT_DOTS; break;
        case 'E': submit_honor = TILE_E; break;
        case 'S': submit_honor = TILE_S; break;
        case 'W': submit_honor = TILE_W; break;
        case 'N': submit_honor = TILE_N; break;
        case 'C': submit_honor = TILE_C; break;
        case 'F': submit_honor = TILE_F; break;
        case 'P': submit_honor = TILE_P; break;
        case '[':
            // 开始副露
            if (bracket) return PARSE_ERROR_ILLEGAL_CHARACTER;
            if (pack_cnt == 4) return PARSE_ERROR_TOO_MANY_FIXED_PACKS;
            if (digit_cnt != 0) return PARSE_ERROR_SUFFIX;

            // 把临时牌提交到手牌，并打表
            for (uint8_t i = 0; i < temp_cnt && tile_cnt < tile_max; ++i) {
                tile_t t = temp_tile[i];
                if (++tile_table[t] > 4) return PARSE_ERROR_TILE_COUNT_GREATER_THAN_4;
                standing_tiles[tile_cnt++] = t;
            }

            // 手牌空间不足3张，说明这组副露多余的
            if (tile_cnt + 3 > tile_max) return PARSE_ERROR_TOO_MANY_TILES;

            temp_cnt = 0;
            bracket = true;
            temp_max = 5;  // 副露最多4张牌，为了能提示超出错误，多预留1
            digit_max = 5;
            break;
        case ']': {
            // 结束副露
            if (!bracket) return PARSE_ERROR_ILLEGAL_CHARACTER;
            if (temp_cnt == 0) return PARSE_ERROR_WRONG_TILES_COUNT_FOR_FIXED_PACK;

            // 解析副露来源
            uint8_t o = 0;
            if (digit_cnt == 1) {
                o = digit_rank[0];
                digit_cnt = 0;
            }
            // 存在多余的数字
            if (digit_cnt != 0) return PARSE_ERROR_ILLEGAL_CHARACTER;

            switch (temp_cnt) {
            case 3: {
                // 3张牌的情况
                tile_t t0 = temp_tile[0], t1 = temp_tile[1], t2 = temp_tile[2];
                if (o == 0) o = 1;

                // 3张牌的副露来源只能是123
                if (o > 3) return PARSE_ERROR_ILLEGAL_CHARACTER;

                // 相同，构成碰
                if (t0 == t1 && t0 == t2) {
                    fixed_packs[pack_cnt++] = make_pack(o, PACK_TYPE_PUNG, t0);
                }
                else {
                    // 不相同，尝试构成吃
                    if (is_numbered_suit_quick(t0)) {
                        if ((t0 + 1 == t1 && t1 + 1 == t2) || (t2 + 1 == t1 && t1 + 1 == t0)) {
                            fixed_packs[pack_cnt++] = make_pack(o, PACK_TYPE_CHOW, t1);
                        }
                        else if ((t0 + 1 == t2 && t2 + 1 == t1) || (t1 + 1 == t2 && t2 + 1 == t0)) {
                            fixed_packs[pack_cnt++] = make_pack(o, PACK_TYPE_CHOW, t2);
                        }
                        else if ((t1 + 1 == t0 && t0 + 1 == t2) || (t2 + 1 == t0 && t0 + 1 == t1)) {
                            fixed_packs[pack_cnt++] = make_pack(o, PACK_TYPE_CHOW, t0);
                        }
                        else {
                            return PARSE_ERROR_CANNOT_MAKE_FIXED_PACK;
                        }
                    }
                    else {
                        return PARSE_ERROR_CANNOT_MAKE_FIXED_PACK;
                    }
                }
                break;
            }
            case 4: {
                // 4张牌只能是杠
                tile_t t = temp_tile[0];
                if (t == temp_tile[1] && t == temp_tile[2] && t == temp_tile[3]) {
                    // 4张牌的副露来源可以是123或567
                    if (o > 7 || o == 4) return PARSE_ERROR_ILLEGAL_CHARACTER;

                    fixed_packs[pack_cnt++] = make_pack(o, PACK_TYPE_KONG, t);
                }
                else {
                    return PARSE_ERROR_CANNOT_MAKE_FIXED_PACK;
                }
                break;
            }
            default:
                // 其他张数都是错误
                return PARSE_ERROR_WRONG_TILES_COUNT_FOR_FIXED_PACK;
            }

            // 打表
            for (uint8_t i = 0; i < temp_cnt; ++i) {
                if (++tile_table[temp_tile[i]] > 4) return PARSE_ERROR_TILE_COUNT_GREATER_THAN_4;
            }
            temp_cnt = 0;
            bracket = false;
            tile_max -= 3;  // NOTE: 这里可以直接-3，因为左括号的情况已经判断了够减
            temp_max = tile_max;
            digit_max = tile_max;
            break;
        }
        default: return PARSE_ERROR_ILLEGAL_CHARACTER;
        }

        // 把数字串转成数牌提交到临时牌中
        if (submit_suit != TILE_SUIT_NONE) {
            if (digit_cnt == 0) return PARSE_ERROR_SUFFIX;
            for (uint8_t i = 0; i < digit_cnt && temp_cnt < temp_max; ++i) {
                temp_tile[temp_cnt++] = make_tile(submit_suit, digit_rank[i]);
            }
            digit_cnt = 0;
        }

        // 提交字牌
        if (submit_honor != 0) {
            if (digit_cnt != 0) return PARSE_ERROR_SUFFIX;
            if (temp_cnt < temp_max) {
                temp_tile[temp_cnt++] = submit_honor;
            }
        }

        // 满了，提前退出
        if (!bracket && temp_cnt == temp_max) {
            break;
        }
    }

    if (digit_cnt != 0) return PARSE_ERROR_SUFFIX;

    // 处理余下的临时牌
    // 把临时牌提交到手牌，并打表
    for (uint8_t i = 0; i < temp_cnt && tile_cnt < tile_max; ++i) {
        tile_t t = temp_tile[i];
        if (++tile_table[t] > 4) return PARSE_ERROR_TILE_COUNT_GREATER_THAN_4;
        standing_tiles[tile_cnt++] = t;
    }

    // 无错误时再写回数据
    if (tile_cnt == tile_max) {
        std::memcpy(hand_tiles->standing_tiles, standing_tiles, (tile_max - 1) * sizeof(tile_t));
        hand_tiles->tile_count = tile_max - 1;
        *serving_tile = standing_tiles[tile_max - 1];
    }
    else {
        std::memcpy(hand_tiles->standing_tiles, standing_tiles, tile_cnt * sizeof(tile_t));
        hand_tiles->tile_count = tile_cnt;
        *serving_tile = 0;
    }

    hand_tiles->pack_count = pack_cnt;
    if (pack_cnt != 0) {
        std::memcpy(hand_tiles->fixed_packs, fixed_packs, pack_cnt * sizeof(pack_t));
    }

    return PARSE_NO_ERROR;
}

// 手牌结构转换为字符串
size_t hand_tiles_to_string(const hand_tiles_t *hand_tiles, char *str, size_t max_size) {
    const auto &suffix_chars = tile_name<>::suffix;
    const auto &honor_chars = tile_name<>::honor;

    // 每组副露最大长度8（即：[1111m1]），每张牌最大长度2
    // 0副露最大长度=2*13=26
    // 1副露最大长度=8+2*10=28
    // 2副露最大长度=8*2+2*7=30
    // 3副露最大长度=8*3+2*4=32
    // 4副露最大长度=8*4+2*1=34
    char buf[35];

    char *p = buf;
    intptr_t pack_count = hand_tiles->pack_count;
    for (intptr_t i = 0; i < pack_count; ++i) {
        pack_t pack = hand_tiles->fixed_packs[i];

        tile_t t = pack_get_tile(pack);
        if (standard_tiles<>::check(t)) {
            uint8_t o = pack_get_offer(pack);
            uint8_t pt = pack_get_type(pack);
            suit_t s = tile_get_suit(t);
            rank_t r = tile_get_rank(t);
            switch (pt) {
            case PACK_TYPE_CHOW:
                *p++ = '[';
                if (s != TILE_SUIT_HONORS) {
                    *p++ = '0' + r - 1;
                    *p++ = '0' + r;
                    *p++ = '0' + r + 1;
                    *p++ = suffix_chars[s - 1];
                    if (o != 1) {
                        *p++ = '0' + o;
                    }
                }
                *p++ = ']';
                break;
            case PACK_TYPE_PUNG:
                *p++ = '[';
                if (s != TILE_SUIT_HONORS) {
                    *p++ = '0' + r;
                    *p++ = '0' + r;
                    *p++ = '0' + r;
                    *p++ = suffix_chars[s - 1];
                }
                else {
                    char ch = honor_chars[r - 1];
                    *p++ = ch;
                    *p++ = ch;
                    *p++ = ch;
                }
                if (o != 1) {
                    *p++ = '0' + o;
                }
                *p++ = ']';
                break;
            case PACK_TYPE_KONG:
                *p++ = '[';
                if (s != TILE_SUIT_HONORS) {
                    *p++ = '0' + r;
                    *p++ = '0' + r;
                    *p++ = '0' + r;
                    *p++ = '0' + r;
                    *p++ = suffix_chars[s - 1];
                }
                else {
                    char ch = honor_chars[r - 1];
                    *p++ = ch;
                    *p++ = ch;
                    *p++ = ch;
                    *p++ = ch;
                }
                if (o != 0) {
                    *p++ = '0' + (is_promoted_kong(pack) ? o | 0x4 : o);
                }
                *p++ = ']';
                break;
            }
        }
    }

    intptr_t max_count = 13 - pack_count * 3;
    intptr_t tile_count = hand_tiles->tile_count;
    if (tile_count > max_count) {
        tile_count = max_count;
    }

    suit_t suit = TILE_SUIT_NONE;
    for (intptr_t i = 0; i < tile_count; ++i) {
        tile_t t = hand_tiles->standing_tiles[i];
        if (standard_tiles<>::check(t)) {
            suit_t s = tile_get_suit(t);
            rank_t r = tile_get_rank(t);

            // 花色有变化，写入一个后缀，结束之前的花色
            if (suit != s) {
                if (suit != TILE_SUIT_NONE && suit != TILE_SUIT_HONORS) {
                    *p++ = suffix_chars[suit - 1];
                }
                suit = s;
            }

            if (s != TILE_SUIT_HONORS) {
                // 数牌，先写入数字
                *p++ = '0' + r;
            }
            else {
                // 字牌，直接写相应字符
                *p++ = honor_chars[r - 1];
            }
        }
    }

    // 补充未写入的花色后缀
    if (suit != TILE_SUIT_NONE && suit != TILE_SUIT_HONORS) {
        *p++ = suffix_chars[suit - 1];
    }

    size_t res = p - buf;
    if (res > max_size) {
        res = max_size;
    }
    std::memcpy(str, buf, res);

    // 补充结尾符
    if (res < max_size) {
        str[res] = '\0';
    }

    return res;
}

}
