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

#ifndef __MAHJONG_ALGORITHM__STRINGIFY_H__
#define __MAHJONG_ALGORITHM__STRINGIFY_H__

#include "tile.h"

namespace mahjong {

using std::size_t;

/**
 * @brief 字符串格式：
 * - 数牌：万=m 条=s 饼=p。后缀使用小写字母，一连串同花色的数牌可合并使用用一个后缀，如123m、678s等等。
 * - 字牌：东南西北=ESWN，中发白=CFP。使用大写字母。
 * - 吃、碰、杠用英文[]，可选用数字表示供牌来源。数字的具体规则如下：
 *    - 吃：表示第几张牌是由上家打出，如[567m2]表示57万吃6万（第2张）。对于不指定数字的，默认为吃第1张。
 *    - 碰：表示由哪家打出，1为上家，2为对家，3为下家，如[999s3]表示碰下家的9条。对于不指定数字的，默认为碰上家。
 *    - 杠：与碰类似，但对于不指定数字的，则认为是暗杠。例如：[SSSS]表示暗杠南；[8888p1]表示大明杠上家的8饼。当数字为5、6、7时，表示加杠。例如：[1111s6]表示碰对家的1条后，又摸到1条加杠。
 * - 范例
 *    - [EEEE][CCCC][FFFF][PPPP]NN
 *    - 1112345678999s9s
 *    - [WWWW1][444s]45m678pFF6m
 *    - [EEEE]288s349pSCFF2p
 *    - [123p1][345s2][999s3]6m6pEW1m
 *    - 356m18s1579pWNFF9p
 */

/**
 * @addtogroup stringify
 * @{
 */

template <class Dummy = int>
struct tile_name {
    static const char *text[34];
    static const char suffix[3];
    static const char honor[7];
};

template <class Dummy>
const char *tile_name<Dummy>::text[] = {
    "1m", "2m", "3m", "4m", "5m", "6m", "7m", "8m", "9m",
    "1s", "2s", "3s", "4s", "5s", "6s", "7s", "8s", "9s",
    "1p", "2p", "3p", "4p", "5p", "6p", "7p", "8p", "9p",
    "E" , "S" , "W" , "N" , "C" , "F" , "P"
};

template <class Dummy>
const char tile_name<Dummy>::suffix[] = { 'm', 's', 'p' };

template <class Dummy>
const char tile_name<Dummy>::honor[] = { 'E', 'S', 'W', 'N', 'C', 'F', 'P' };

/**
 * @name error codes
 * @{
 *  解析牌的错误码
 */
#define PARSE_NO_ERROR 0                                ///< 无错误
#define PARSE_ERROR_ILLEGAL_CHARACTER -1                ///< 非法字符
#define PARSE_ERROR_SUFFIX -2                           ///< 后缀错误
#define PARSE_ERROR_WRONG_TILES_COUNT_FOR_FIXED_PACK -3 ///< 副露包含错误的牌数目
#define PARSE_ERROR_CANNOT_MAKE_FIXED_PACK -4           ///< 无法正确解析副露
#define PARSE_ERROR_TOO_MANY_FIXED_PACKS -5             ///< 过多组副露（一副合法手牌最多4副露）
#define PARSE_ERROR_TOO_MANY_TILES -6                   ///< 过多牌
#define PARSE_ERROR_TILE_COUNT_GREATER_THAN_4 -7        ///< 某张牌出现超过4枚

 /**
  * @}
  */

/**
 * @brief 字符串转换为手牌结构和上牌
 * @param [in] str 字符串
 * @param [in] len 字符串长度
 * @param [out] hand_tiles 手牌结构
 * @param [out] serving_tile 上的牌
 * @retval PARSE_NO_ERROR 无错误
 * @retval PARSE_ERROR_ILLEGAL_CHARACTER 非法字符
 * @retval PARSE_ERROR_NO_SUFFIX_AFTER_DIGIT 数字后面缺少后缀
 * @retval PARSE_ERROR_WRONG_TILES_COUNT_FOR_FIXED_PACK 副露包含错误的牌数目
 * @retval PARSE_ERROR_CANNOT_MAKE_FIXED_PACK 无法正确解析副露
 * @retval PARSE_ERROR_TOO_MANY_FIXED_PACKS 过多组副露（一副合法手牌最多4副露）
 * @retval PARSE_ERROR_TOO_MANY_TILES 过多牌
 * @retval PARSE_ERROR_TILE_COUNT_GREATER_THAN_4 某张牌出现超过4枚
 */
int parse_hand_tiles(const char *str, size_t len, hand_tiles_t *hand_tiles, tile_t *serving_tile);

/**
 * @brief 手牌结构转换为字符串
 * @param [in] hand_tiles 手牌结构
 * @param [out] str 字符串
 * @param [in] max_size 字符串最大长度（注意：转换结果包含\0在内最长为35）
 * @return size_t 字符串长度
 */
size_t hand_tiles_to_string(const hand_tiles_t *hand_tiles, char *str, size_t max_size);

/**
 * end group
 * @}
 */

}

#endif
