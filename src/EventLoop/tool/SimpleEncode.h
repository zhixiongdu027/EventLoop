//
// Created by adugeek on 12/17/15.
//

#ifndef EVENTLOOP_TOOL_SIMPLEENCODE_H
#define EVENTLOOP_TOOL_SIMPLEENCODE_H

#include <stddef.h>

#define KEY "90~!@#12345ZXCVBNMLKJHGFD678$%^&*()_+`-=:zxcvbnm<>?qwertyuiop[]asdfghjkl;'SAPOIUYTREWQ"
#define KEY_LEN sizeof(KEY)-1

class SimpleEncode {
public:
    static inline void encode(char *data, size_t len) {
        xor_(data, len, len % 7);
    };

    static inline void decode(char *data, size_t len) {
        xor_(data, len, len % 7);
    }

private:
    static inline void xor_(char *data, size_t data_len, size_t key_pos) {
        for (size_t i = 0; i < data_len; ++i) {
            key_pos = (key_pos > KEY_LEN ? 13 : key_pos);
            data[i] ^= KEY[key_pos++];
        }
    }
};

#endif //EVENTLOOP_TOOL_SIMPLEENCODE_H