//
// Created by adugeek on 12/16/15.
//

#ifndef EVENTLOOP_TOOL_LIKELY_H
#define EVENTLOOP_TOOL_LIKELY_H

#undef LIKELY
#undef UNLIKELY

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x)   (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif


#endif //EVENTLOOP_TOOL_LIKELY_H
