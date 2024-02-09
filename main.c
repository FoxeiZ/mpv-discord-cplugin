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

static const char *APPLICATION_ID = "345229890980937739";
static char SendPresence = 1;
static char Debug = 1;

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
    int response = -1;
    char yn[4];
    printf("\nDiscord: join request from %s#%s - %s\n",
           request->username,
           request->discriminator,
           request->userId);
    do
    {
        printf("Accept? (y/n)");
        if (!prompt(yn, sizeof(yn)))
        {
            break;
        }

        if (!yn[0])
        {
            continue;
        }

        if (yn[0] == 'y')
        {
            response = DISCORD_REPLY_YES;
            break;
        }

        if (yn[0] == 'n')
        {
            response = DISCORD_REPLY_NO;
            break;
        }
    } while (1);
    if (response != -1)
    {
        Discord_Respond(request->userId, response);
    }
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
    if (Debug)
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

int mpv_open_cplugin(mpv_handle *handle)
{
    discordInit();
    while (1)
    {

        mpv_event *event = mpv_wait_event(handle, -1);
        if (event->event_id == MPV_EVENT_SHUTDOWN)
            break;

        char *event_name = mpv_event_name(event->event_id);
        int max_len = strlen(event_name) + 11;
        char *event_string = (char *)malloc(max_len);
        snprintf(event_string, max_len, "Event: %s", event_name);
        printf("%s\n", event_string);

        DiscordRichPresence richPresence = {
            .buttons = NULL,
            .details = event_string,
            .state = "Playing",
        };
        Discord_UpdatePresence(&richPresence);
        discordCallback();
        free(event_string);
    }

    discordShutdown();
    return 0;
}
