#ifndef BALLOON_H
#define BALLOON_H

#ifdef __cplusplus
extern "C" {
#endif

void balloon(const unsigned char* input, char* output, int length, const unsigned char* salt, int salt_length);

#ifdef __cplusplus
}
#endif

#endif
