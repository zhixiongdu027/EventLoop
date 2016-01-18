//
// Created by adugeek on 15-7-14.
//

#include "HTTP.h"

static int url_dump(const char *data, size_t len, size_t *url_pos, size_t *url_len) {
    if (len < strlen("GET / HTTP/1.1\r\n")) {
        return -1;
    }
    const char *iterator = data + strlen("GET ");
    if (*iterator == ' ' || *iterator == '\t') {
        iterator += 1;
    }
    if (*iterator != '/' && *iterator != 'h') {
        return -1;
    }
    const char *url = iterator;
    iterator = data + len - strlen("HTTP/1.1\r\n");
    while (iterator > url) {
        iterator--;
        if (*iterator == ' ' || *iterator == '\t') {
            continue;
        }
        *url_pos = url - data;
        *url_len = iterator - url + 1;
        return 0;
    }
    return -1;
}

static int response_dump(const char *data, size_t len, size_t *response_pos, size_t *response_len) {
    if (len < strlen("HTTP/1.1 200")) {
        return -1;
    }
    const char *pos = data + strlen("HTTP/1.1 ");
    for (int i = 0; i < 3; ++i) {
        if (!isdigit(*(pos + i))) {
            return -1;
        }
    }
    *response_pos = pos - data;
    *response_len = 3;
    return 0;
}

static int head_dump(const char *data, size_t len, size_t *key_pos, size_t *key_len, size_t *value_pos,
                     size_t *value_len) {
    const char *pos = data;
    if (*pos == ' ' || *pos == '\t') {
        pos++;
    }
    if (!isalpha(*pos)) {
        return -1;
    }
    const char *key = pos;
    *key_pos = key - data;
    const char *end = data + len - strlen("\r\n");
    while (pos < end) {
        pos++;
        if (*pos == ' ' || *pos == '\t') {
            *key_len = pos - key;
            pos++;
            if (*pos != ':') {
                return -1;
            }
            goto after_get_key;
        }
        else if (*pos == ':') {
            *key_len = pos - key;
            goto after_get_key;
        }
        continue;
    }
    return -1;

    after_get_key:
    assert(*pos == ':');
    pos++;
    if (*pos == ' ' || *pos == '\t') {
        pos++;
    }
    const char *value = pos;
    pos = end;
    while (pos > value) {
        pos--;
        if (*pos == ' ' || *pos == '\t') {
            continue;
        }
        *value_pos = value - data;
        *value_len = pos - value + 1;
        return 0;
    }
    return -1;
}

static bool have_chunck_end(const char *data, size_t length) {
    static const char *chunck_end_str = "0\r\n\r\n";
    static const size_t chunck_end_length = strlen(chunck_end_str);
    if (length < chunck_end_length) {
        return false;
    }
    const char *post = data + length - chunck_end_length;
    return memcmp(post, chunck_end_str, chunck_end_length) == 0;
}

HTTP::LINE_STATUS HTTP::get_http_line(const char *sz_buff, size_t sz_length, const char **line, size_t *line_len) {
    const char *split = static_cast<const char *>(memmem(sz_buff, sz_length, "\r\n", strlen("\r\n")));
    if (split != NULL) {
        *line = sz_buff;
        *line_len = split - sz_buff + strlen("\r\n");
        return LINE_OK;
    }
    return LINE_OPEN;
}

ExecuteState HTTP::parse(const char *buff, size_t length) {
    assert(parsed_index_ <= length);
    if (todo_ == TODO_GET_URL_OR_RESPONSE) {
        goto get_url_or_response;
    }
    else if (todo_ == TODO_GET_HEAD) {
        goto get_head;
    }
    else if (todo_ == TODO_GET_BODY) {
        goto get_body;
    }
    else {
        goto error;
    }

    get_url_or_response:
    const char *line;
    size_t line_length;
    HTTP::LINE_STATUS line_status;//= get_http_line(buff + parsed_index_, length - parsed_index_, &line, &line_length);
    line_status = get_http_line(buff + parsed_index_, length - parsed_index_, &line, &line_length);
    if (line_status == LINE_OPEN) {
        return ExecuteProcessing;
    }
    parsed_index_ += line_length;
    size_t url_or_response_pos;
    size_t url_or_response_len;
    if (strncmp(line, "HT", strlen("HT")) == 0 &&
        response_dump(line, line_length, &url_or_response_pos, &url_or_response_len) == 0) {
        head_info["STATUS"] = std::move(std::make_pair(line - buff + url_or_response_pos, url_or_response_len));
        todo_ = TODO_GET_HEAD;
    } else if (url_dump(line, line_length, &url_or_response_pos, &url_or_response_len) == 0) {

        head_info["URL"] = std::move(std::make_pair(line - buff + url_or_response_pos, url_or_response_len));
        todo_ = TODO_GET_HEAD;
    }
    else {
        goto error;
    }

    get_head:
    while (true) {
        const char *line;
        size_t line_length;
        LINE_STATUS line_status = get_http_line(buff + parsed_index_, length - parsed_index_, &line, &line_length);
        if (line_status == LINE_OPEN) {
            return ExecuteProcessing;
        }
        parsed_index_ += line_length;
        if (line_length == 2) {
            todo_ = TODO_GET_BODY;
            goto get_body;
        }
        size_t head_key_pos;
        size_t head_key_len;
        size_t head_value_pos;
        size_t head_value_len;

        if (head_dump(line, line_length, &head_key_pos, &head_key_len, &head_value_pos, &head_value_len) == 0) {
            std::string head_filed = std::string(line + head_key_pos, head_key_len);
            for (std::string::iterator iterator = head_filed.begin(); iterator != head_filed.end(); ++iterator) {
                *iterator = toupper(*iterator);
            }
            head_info[head_filed] = std::move(std::make_pair(line - buff + head_value_pos, head_value_len));
            continue;
        }
        else {
            goto error;
        }
    }
    get_body:
    if (body_len_type_ == UNKNOWN) {
        if (head_info.find("CONTENT-LENGTH") != head_info.end()) {
            body_len_value_ = atoi(buff + head_info["CONTENT-LENGTH"].first);
            body_len_type_ = LENGTH;
        }
        else if (head_info.find("TRANSFER-ENCODING") != head_info.end() &&
                 strncmp(buff + head_info["TRANSFER-ENCODING"].first, "chunked", strlen("chunked")) == 0) {
            body_len_type_ = CHUNCK;
        }
        else {
            body_len_type_ = NOLENGTH;
        }
    }
    switch (body_len_type_) {
        case LENGTH:
            if (parsed_index_ + body_len_value_ > length) {
                return ExecuteProcessing;
            }
            else if (parsed_index_ + body_len_value_ == length) {
                body_table_.push_back(std::make_pair(parsed_index_, body_len_value_));
                parsed_index_ = length;
                return ExecuteDone;
            }
            else {
                return ExecuteError;
            }
        case CHUNCK:
            if (!have_chunck_end(buff, length)) {
                return ExecuteProcessing;
            }
            else {
                while (true) {
                    const char *chunck_split = (const char *) memmem(buff + parsed_index_, length - parsed_index_,
                                                                     "\r\n", strlen("\r\n"));
                    if (chunck_split == NULL) {
                        return ExecuteError;
                    }
                    int data_length = strtol(buff + parsed_index_, NULL, 16);
                    const char *data_begin = chunck_split + strlen("\r\n");
                    const char *data_end = data_begin + data_length;

                    if (data_end + strlen("\r\n") > buff + length) {
                        return ExecuteProcessing;
                    }
                    assert(*(data_end) == '\r' && *(data_end + 1) == '\n');

                    parsed_index_ = data_end + strlen("\r\n") - buff;
                    if (data_length == 0) {
                        assert(parsed_index_ == length);
                        return ExecuteDone;
                    }
                    body_table_.push_back(std::make_pair(data_begin - buff, data_length));
                }
            }
        case NOLENGTH:
            body_table_.push_back(std::make_pair(parsed_index_, length - parsed_index_));
            return ExecuteDone;
        default:
            return ExecuteError;
    }

    error:
    return ExecuteError;
}
