/*
    This is a simple example in C of using the rich presence API asynchronously.
*/

#define _CRT_SECURE_NO_WARNINGS /* thanks Microsoft */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mpv/client.h>

#include "discord_rpc.h"
#include "more_headers.h"

static const char *APPLICATION_ID = "968883870520987678";
static int IsEnable = 1;
static int Debug = 1;
static int64_t StartTime;

static void handleDiscordReady(const DiscordUser *connectedUser)
{
    if (!connectedUser->discriminator[0] || strcmp(connectedUser->discriminator, "0") == 0 || strcmp(connectedUser->discriminator, "0000") == 0)
    {
        printf("\nDiscord: connected to user @%s (%s) - %s\n",
               connectedUser->username,
               connectedUser->globalName,
               connectedUser->userId);
    }
    else
    {
        printf("\nDiscord: connected to user %s#%s (%s) - %s\n",
               connectedUser->username,
               connectedUser->discriminator,
               connectedUser->globalName,
               connectedUser->userId);
    }
}

static void handleDiscordDisconnected(int errcode, const char *message)
{
    printf("\nDiscord: disconnected (%d: %s)\n", errcode, message);
}

static void handleDiscordError(int errcode, const char *message)
{
    printf("\nDiscord: error (%d: %s)\n", errcode, message);
}

static void handleDebug(char isOut,
                        const char *opcodeName,
                        const char *message,
                        uint32_t messageLength)
{
    if (!Debug)
        return;

    unsigned int len = (messageLength > 7 ? messageLength : 7) + 6 + 7 + 7 + 1;
    char *buf = (char *)malloc(len);
    char *direction = isOut ? "send" : "receive";
    if (!messageLength || !message || !message[0])
    {
        snprintf(buf, len, "[%s] [%s] <empty>", direction, opcodeName);
    }
    else
    {
        int written = snprintf(buf, len, "[%s] [%s] ", direction, opcodeName);
        int remaining = len - written;
        int toWrite = remaining > (messageLength + 1) ? (messageLength + 1) : remaining;
        int written2 = snprintf(buf + written, toWrite, message);
    }
    printf("[DEBUG] %s\n", buf);
    free(buf);
}

static void handleDiscordJoin(const char *secret)
{
    printf("\nDiscord: join (%s)\n", secret);
}

static void handleDiscordSpectate(const char *secret)
{
    printf("\nDiscord: spectate (%s)\n", secret);
}

static void handleDiscordJoinRequest(const DiscordUser *request)
{
}

static void handleDiscordInvited(/* DISCORD_ACTIVITY_ACTION_TYPE_ */ int8_t type,
                                 const DiscordUser *user,
                                 const DiscordRichPresence *activity,
                                 const char *sessionId,
                                 const char *channelId,
                                 const char *messageId)
{
    printf("Received invite type: %i, from user: %s, with activity state: %s, with session id: %s, "
           "from channel id: %s, with message id: %s",
           type,
           user->username,
           activity->state,
           sessionId,
           channelId,
           messageId);

    // Discord_AcceptInvite(user->userId, type, sessionId, channelId, messageId);
}

static void populateHandlers(DiscordEventHandlers *handlers)
{
    memset(handlers, 0, sizeof(handlers));
    handlers->ready = handleDiscordReady;
    handlers->disconnected = handleDiscordDisconnected;
    handlers->errored = handleDiscordError;
    handlers->debug = handleDebug;
    handlers->joinGame = handleDiscordJoin;
    handlers->spectateGame = handleDiscordSpectate;
    handlers->joinRequest = handleDiscordJoinRequest;
    handlers->invited = handleDiscordInvited;
}

static void discordUpdateHandlers()
{
    DiscordEventHandlers handlers;
    populateHandlers(&handlers);
    Discord_UpdateHandlers(&handlers);
}

static void discordInit()
{
    DiscordEventHandlers handlers;
    populateHandlers(&handlers);
    Discord_Initialize(APPLICATION_ID, &handlers, 1, NULL);
}

static void discordShutdown()
{
    if (Debug)
    {
        printf("[DEBUG] [local] Discord_Shutdown\n");
    }
    Discord_Shutdown();
}

static void discordCallback()
{
#ifdef DISCORD_DISABLE_IO_THREAD
    Discord_UpdateConnection();
#endif
    Discord_RunCallbacks();
}

static void secondToClock(int secs, char *buf)
{
    int hours = secs / 3600;
    int minutes = (secs - hours * 3600) / 60;
    int seconds = secs - (hours * 3600 + minutes * 60);
    snprintf(buf, 9, "%02d:%02d:%02d", hours, minutes, seconds);
}

static int isStream(mpv_handle *handle)
{
    int ret;
    char *path = mpv_get_property_string(handle, "path");

    if (strstr(path, "http://") != NULL || strstr(path, "https://") != NULL)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }

    mpv_free(path);
    return ret;
}

static void updatePresence(DiscordRichPresence precence)
{
    if (Debug)
    {
        printf("[DEBUG] [local] Discord_UpdatePresence\n");
        printf("\tstate: %s\n", precence.state);
        printf("\tdetails: %s\n", precence.details);
        printf("\tstartTimestamp: %d\n", precence.startTimestamp);
        printf("\tendTimestamp: %d\n", precence.endTimestamp);
        printf("\tlargeImageKey: %s\n", precence.largeImageKey);
        printf("\tlargeImageText: %s\n", precence.largeImageText);
        printf("\tsmallImageKey: %s\n", precence.smallImageKey);
        printf("\tsmallImageText: %s\n", precence.smallImageText);
        printf("\tpartyId: %d\n", precence.partyId);
        printf("\tpartySize: %d\n", precence.partySize);
        printf("\tpartyMax: %d\n", precence.partyMax);
        printf("\tpartyPrivacy: %d\n", precence.partyPrivacy);
        printf("\tmatchSecret: %s\n", precence.matchSecret);
        printf("\tjoinSecret: %s\n", precence.joinSecret);
        printf("\tspectateSecret: %s\n", precence.spectateSecret);
        printf("\tinstance: %d\n", precence.instance);
    }

    Discord_UpdatePresence(&precence);
    discordCallback();
}

static void getMPV_State(mpv_handle *handle, int *isPlaying, char *smallStateStr, char *smallStateImg)
{
    mpv_get_property(handle, "pause", MPV_FORMAT_FLAG, isPlaying);
    *isPlaying = !*isPlaying;

    if (!*isPlaying)
    {
        strcpy(smallStateStr, "Paused");
        strcpy(smallStateImg, "pause");
        return;
    }

    char *loopFile = mpv_get_property_string(handle, "loop-file");
    if (strncmp(loopFile, "inf", 3) == 0)
    {
        strcpy(smallStateStr, "Looping");
        strcpy(smallStateImg, "loop");
        return;
    }
    mpv_free(loopFile);

    char *loopPlaylist = mpv_get_property_string(handle, "loop-playlist");
    if (strncmp(loopPlaylist, "inf", 3) == 0)
    {
        strcpy(smallStateStr, "Loop all");
        strcpy(smallStateImg, "loop_playlist");
        return;
    }
    mpv_free(loopPlaylist);

    strcpy(smallStateStr, "Playing");
    strcpy(smallStateImg, "play");
}

static void setRPC_ByMPVState(mpv_handle *handle)
{
    int isPlaying;
    char state[256];
    char *smallStateStr = malloc(9);  // max 8 bytes + \0 = 9
    char *smallStateImg = malloc(14); // max 13 bytes + \0 = 14

    // metadata
    char *songTitle = mpv_get_property_string(handle, "media-title");
    char *songArtist = mpv_get_property_string(handle, "metadata/by-key/artist");
    if (songArtist != NULL)
    {
        snprintf(state, 256, "%s - %s", songArtist, songTitle);
    }
    else
    {
        snprintf(state, 256, "%s", songTitle);
    }
    mpv_free(songTitle);
    mpv_free(songArtist);

    // init rpc embed
    getMPV_State(handle, &isPlaying, smallStateStr, smallStateImg);
    DiscordRichPresence richPresence = {
        .state = state,
        .details = smallStateStr,
        .smallImageKey = smallStateImg,
        .smallImageText = smallStateStr,
        .largeImageKey = "mpvlogo",
        .largeImageText = "mpv",
    };

    // timestamp
    int64_t currentTime = time(0);
    int64_t startTimestamp, endTimestamp;
    mpv_get_property(handle, "time-pos", MPV_FORMAT_INT64, &startTimestamp);
    if (isPlaying)
    {
        mpv_get_property(handle, "duration", MPV_FORMAT_INT64, &endTimestamp);
        richPresence.startTimestamp = currentTime + startTimestamp;
        richPresence.endTimestamp = currentTime + endTimestamp;
    }
    else
    {
        richPresence.startTimestamp = currentTime + startTimestamp;
        richPresence.endTimestamp = 0;
    }

    updatePresence(richPresence);
    free(smallStateStr);
    free(smallStateImg);
}

static void setRPC_Idle()
{
    DiscordRichPresence richPresence = {
        .details = "Idling",
        .smallImageKey = "idle",
        .smallImageText = "Idle",
        .largeImageKey = "mpvlogo",
        .largeImageText = "mpv",
    };
    updatePresence(richPresence);
}

int mpv_open_cplugin(mpv_handle *handle)
{
    int isIdle;
    int IsRunning = 1;

    // init
    discordInit();
    mpv_observe_property(handle, 0, "pause", MPV_FORMAT_FLAG);

    // main loop
    while (IsRunning)
    {
        mpv_event *event = mpv_wait_event(handle, -1);
        if (!IsEnable)
            continue;

        switch (event->event_id)
        {
        case MPV_EVENT_FILE_LOADED:
        case MPV_EVENT_PROPERTY_CHANGE:
            setRPC_ByMPVState(handle);
            break;
        case MPV_EVENT_IDLE:
            mpv_get_property(handle, "idle-active", MPV_FORMAT_FLAG, &isIdle);
            if (isIdle)
                setRPC_Idle();
            break;
        case MPV_EVENT_SHUTDOWN:
            IsRunning = 0;
            discordShutdown();
            break;
        default:
            if (Debug)
            {
                const char *event_name = mpv_event_name(event->event_id);
                if (event_name != NULL)
                    printf("mpv event %s\n", event_name);
            }
            break;
        }
    }

    return 0;
}
