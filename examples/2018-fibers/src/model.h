#pragma once

#include "calqueue.h"

#define COMPLETE_CALLS		15000

#define TA			0.16
#define TA_DURATION		60
#define CHANNELS_PER_CELL	1000
#define TA_CHANGE		150.0

#define CROSS_PATH_GAIN		0.00000000000005
#define PATH_GAIN		0.0000000001
#define MIN_POWER		3
#define MAX_POWER		3000
#define SIR_AIM			10

#define CHAN_BUSY	1
#define CHAN_FREE	0

#define START_CALL	20
#define END_CALL	21
#define HANDOFF_LEAVE	30
#define HANDOFF_RECV	31
#define FADING_RECHECK	40

#define MSK 0x1
#define SET_CHANNEL_BIT(B,K) ( B |= (MSK << K) )
#define RESET_CHANNEL_BIT(B,K) ( B &= ~(MSK << K) )
#define CHECK_CHANNEL_BIT(B,K) ( B & (MSK << K) )

#define BITS (sizeof(int) * 8)

#define CHECK_CHANNEL(P,I) ( CHECK_CHANNEL_BIT(						\
	((unsigned int*)(((lp_state_type*)P)->channel_state))[(int)((int)I / BITS)],	\
	((int)I % BITS)) )
#define SET_CHANNEL(P,I) ( SET_CHANNEL_BIT(						\
	((unsigned int*)(((lp_state_type*)P)->channel_state))[(int)((int)I / BITS)],	\
	((int)I % BITS)) )
#define RESET_CHANNEL(P,I) ( RESET_CHANNEL_BIT(						\
	((unsigned int*)(((lp_state_type*)P)->channel_state))[(int)((int)I / BITS)],	\
	((int)I % BITS)) )

#define INIT 0

typedef struct _msg_t {
	unsigned int 	sender;
	unsigned int	receiver;
	int   			type;
	double			timestamp;
	int				channel;
} msg_t;

typedef struct _sir_data_per_cell{
    double fading; // Fading of the call
    double power; // Power allocated to the call
} sir_data_per_cell;

// Taglia di 16 byte
typedef struct _channel{
	int channel_id; // Number of the channel
	sir_data_per_cell *sir_data; // Signal/Interference Ratio data
	struct _channel *next;
	struct _channel *prev;
} channel;

typedef struct _lp_state_type{
	unsigned int channel_counter; // How many channels are currently free
	unsigned int arriving_calls; // How many calls have been delivered within this cell
	unsigned int complete_calls; // Number of calls which were completed within this cell
	unsigned int blocked_on_setup; // Number of calls blocked due to lack of free channels
	unsigned int blocked_on_handoff; // Number of calls blocked due to lack of free channels in HANDOFF_RECV
	unsigned int leaving_handoffs; // How many calls were diverted to a different cell
	unsigned int arriving_handoffs; // How many calls were received from other cells
	unsigned int cont_no_sir_aim; // Used for fading recheck
	unsigned int executed_events; // Total number of events

	double lvt; // Last executed event was at this simulation time

	double ta; // Current call interarrival frequency for this cell
	double ref_ta; // Initial call interarrival frequency (same for all cells)
	double ta_duration; // Average duration of a call
	double ta_change; // Average time after which a call is diverted to another cell

	int channels_per_cell; // Total channels in this cell

	unsigned int *channel_state;
	struct _channel *channels;
	int dummy;
	bool dummy_flag;
} lp_state_type;

extern long long ProcessEvent(msg_t *msg, lp_state_type *state, calqueue *q);
extern void deallocation(unsigned int lp, lp_state_type *state, int channel, double);
extern int allocation(lp_state_type *state);
extern double recompute_ta(double ref_ta, double time_now);
