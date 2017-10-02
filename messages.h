#ifndef MESSAGES_H
#define MESSAGES_H

#include "yojimbo.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

using namespace yojimbo;

const uint64_t ProtocolId = 0x11223344556677ULL;

const int ClientPort = 30000;
const int ServerPort = 40000;

inline int GetNumBitsForMessage( uint16_t sequence )
{
    static int messageBitsArray[] = { 1, 320, 120, 4, 256, 45, 11, 13, 101, 100, 84, 95, 203, 2, 3, 8, 512, 5, 3, 7, 50 };
    const int modulus = sizeof( messageBitsArray ) / sizeof( int );
    const int index = sequence % modulus;
    return messageBitsArray[index];
}

struct SkillzMessage : public Message
{
    uint16_t sequence;

    SkillzMessage()
    {
        sequence = 0;
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_bits( stream, sequence, sizeof(uint16_t) * 8 );

        int numBits = GetNumBitsForMessage( sequence );
        int numWords = numBits / 32;
        uint32_t dummy = 0;
        for ( int i = 0; i < numWords; ++i )
            serialize_bits( stream, dummy, 32 );
        int numRemainderBits = numBits - numWords * 32;
        if ( numRemainderBits > 0 )
            serialize_bits( stream, dummy, numRemainderBits );

        return true;
    }

    YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

struct SkillzBlockMessage : public BlockMessage
{
    uint16_t sequence;

    SkillzBlockMessage()
    {
        sequence = 0;
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_bits( stream, sequence, sizeof(uint16_t) * 8 );
        return true;
    }

    YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

// Game Start Message sent by the server to the clients
struct GameStart : public BlockMessage
{
    uint16_t matchId;

    /**
     * TODO: define what should be put into the Game Start message
     * The Game Start message should include the matchId of the server
     * and the two clientIds to make sure the server sent the "Game Start"
     * message to the right players
     * Currently, the matchId is hard-code for testing usage, but will be
     * changed to use the actual matchId of the server
     */
    GameStart()
    {
        matchId = 12345;
    }

    template <typename Stream> bool Serialize( Stream & stream )
    {
        serialize_bits( stream, matchId, sizeof(uint16_t) * 8 );
        return true;
    }

    YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

struct TestSerializeFailOnReadMessage : public Message
{
    template <typename Stream> bool Serialize( Stream & /*stream*/ )
    {
        return !Stream::IsReading;
    }

    YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

struct TestExhaustStreamAllocatorOnReadMessage : public Message
{
    template <typename Stream> bool Serialize( Stream & stream )
    {
        if ( Stream::IsReading )
        {
            const int NumBuffers = 100;

            void * buffers[NumBuffers];

            memset( buffers, 0, sizeof( buffers ) );

            for ( int i = 0; i < NumBuffers; ++i )
            {
                buffers[i] = YOJIMBO_ALLOCATE( stream.GetAllocator(), 1024 * 1024 );
            }

            for ( int i = 0; i < NumBuffers; ++i )
            {
                YOJIMBO_FREE( stream.GetAllocator(), buffers[i] );
            }
        }

        return true;
    }

    YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

enum SkillzMessageType
{
    SKILLZ_MESSAGE,
    SKILLZ_BLOCK_MESSAGE,
    GAME_START,
    TEST_SERIALIZE_FAIL_ON_READ_MESSAGE,
    TEST_EXHAUST_STREAM_ALLOCATOR_ON_READ_MESSAGE,
    NUM_TEST_MESSAGE_TYPES
};

YOJIMBO_MESSAGE_FACTORY_START( TestMessageFactory, NUM_TEST_MESSAGE_TYPES );
    YOJIMBO_DECLARE_MESSAGE_TYPE( SKILLZ_MESSAGE, SkillzMessage );
    YOJIMBO_DECLARE_MESSAGE_TYPE( GAME_START, GameStart );
    YOJIMBO_DECLARE_MESSAGE_TYPE( SKILLZ_BLOCK_MESSAGE, SkillzBlockMessage );
    YOJIMBO_DECLARE_MESSAGE_TYPE( TEST_SERIALIZE_FAIL_ON_READ_MESSAGE, TestSerializeFailOnReadMessage );
    YOJIMBO_DECLARE_MESSAGE_TYPE( TEST_EXHAUST_STREAM_ALLOCATOR_ON_READ_MESSAGE, TestExhaustStreamAllocatorOnReadMessage );
YOJIMBO_MESSAGE_FACTORY_FINISH();

class TestAdapter : public Adapter
{
public:

    MessageFactory * CreateMessageFactory( Allocator & allocator )
    {
        return YOJIMBO_NEW( allocator, TestMessageFactory, allocator );
    }
};

static TestAdapter adapter;

#endif // #ifndef MESSAGES_H
