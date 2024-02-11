#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TimeStamp
    {
        int64_t startTimestamp;
        int64_t endTimestamp;
    } TimeStamp;

    typedef struct LargeImage
    {
        const char *largeImageKey;  /* max 32 bytes */
        const char *largeImageText; /* max 128 bytes */
    } LargeImage;

    typedef struct SmallImage
    {
        const char *smallImageKey;  /* max 32 bytes */
        const char *smallImageText; /* max 128 bytes */
    } SmallImage;

    typedef struct PartyProperties
    {
        const char *partyId; /* max 128 bytes */
        int partySize;
        int partyMax;
        int partyPrivacy;
    } PartyProperties;

    typedef struct Secrets
    {
        const char *matchSecret;    /* max 128 bytes */
        const char *joinSecret;     /* max 128 bytes */
        const char *spectateSecret; /* max 128 bytes */
    } Secrets;

#ifdef __cplusplus
} /* extern "C" */
#endif