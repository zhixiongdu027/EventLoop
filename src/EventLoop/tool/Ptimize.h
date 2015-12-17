//
// Created by adugeek on 12/16/15.
//

#ifndef EVENTLOOP_TOOL_PTIMIZE_H
#define EVENTLOOP_TOOL_PTIMIZE_H

#ifdef __GNUC__
# define LIKELY(X) __builtin_expect(!!(X), 1)
# define UNLIKELY(X) __builtin_expect(!!(X), 0)
#else
# define LIKELY(X) (X)
# define UNLIKELY(X) (X)
#endif


#endif //EVENTLOOP_TOOL_PTIMIZE_H
