
include_directories(include)

set (ARGON2_SRC
    "argon2.c"
    "core.c"
    "blake2b.c"
    "thread.c"
    "encoding.c"
)

list(APPEND ARGON2_SRC "opt.c")

add_library (argon2 STATIC ${ARGON2_SRC})
