
include_directories(include)

set (ARGON2_SRC
    "argon2.c"
    "core.c"
    "blake2b.c"
    "thread.c"
    "encoding.c"
)

if (ANDROID OR ARM)
  list(APPEND ARGON2_SRC "ref.c")
else()
  list(APPEND ARGON2_SRC "opt.c")
endif()

add_library (argon2 STATIC ${ARGON2_SRC})
