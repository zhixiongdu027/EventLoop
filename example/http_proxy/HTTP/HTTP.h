//
// Created by adugeek on 15-7-14.
//

#ifndef HTTPPARSER_H
#define HTTPPARSER_H


#include <stddef.h>
#include <assert.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <string.h>
#include "EventLoop/tool/ExecuteState.h"

/*  http 解析 基于以下假定
 * 1: 所有进入数据都是标准 http 格式--未严格检查错误
 * 2：一个请求包发送完毕，在接收所有响应数据之前不会再次发送请求
 */
class HTTP {
public:
    enum TODO {
        TODO_GET_URL_OR_RESPONSE,
        TODO_GET_HEAD,
        TODO_GET_BODY
    };

private:
    enum LINE_STATUS {
        LINE_OPEN = 0, LINE_OK = 1
    };

    enum BODY_LEN_TYPE {
        UNKNOWN,
        NOLENGTH,
        CHUNKED,
        LENGTH
    };

public:
    int todo_;

    HTTP() {
        parsed_index_ = 0;
        todo_ = TODO_GET_URL_OR_RESPONSE;
        body_len_value_ = 0;
        body_len_type_ = UNKNOWN;
    }

    ExecuteState parse(const char *buff, size_t length);

    void clear() {
        parsed_index_ = 0;
        todo_ = TODO_GET_URL_OR_RESPONSE;
        body_len_type_ = UNKNOWN;
        head_info.clear();
        body_table_.clear();
    }

private:
    LINE_STATUS get_http_line(const char *sz_buff, size_t sz_length, const char **line, size_t *line_len);

public:
    std::unordered_map<std::string, std::pair<size_t, size_t> > head_info;
    std::vector<std::pair<size_t, size_t> > body_table_;
private:
    size_t parsed_index_;
    long body_len_value_;
    int body_len_type_;
};

#endif //HTTPPARSER_H
