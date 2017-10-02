#include "nec_decode.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_tim.h"
#include "main.h"
#include "common.h"

static NEC_FRAME frame;
static NEC_FRAME lastFrame;
static int8_t pos = 0;	// change
static uint32_t rawdata = 0;
static uint8_t status = 0; // failed=3, received=1 , undetermined=0
static uint8_t AGC_OK = 0;
static uint8_t AGC_REP = 0;
static uint32_t thigh = 0;
static uint32_t tlow = 0;
static uint8_t p = 0;
static uint32_t dbg_th[60], dbg_tl[60], dbg_s=0;

extern TIM_HandleTypeDef htim4;
static TIM_HandleTypeDef* nec_timer = &htim4;

void NEC_Init(void) {

}

void NEC_DeInit(void) {

}

uint32_t NEC_GetRawData() {
	return rawdata;
}

void NEC_Reset() {
	AGC_OK = 0;
	AGC_REP = 0;
	pos = 0;	// change
	p=0;
	dbg_s = 0;
	thigh=0;
	tlow=0;
	status = 0;
	rawdata = 0;
	frame.Address = 0;
	frame.Command = 0;
}

void NEC_TimerRanOut() {
	NEC_StopTimer();
	// LOG_DBG("Ov, AGC=%d, pos=%d",(int)AGC_OK,(int)pos);
	if (AGC_REP == 1) {
		frame = lastFrame;
		// NEC_ReceiveInterrupt(frame); //repeated command received
		NEC_Reset();
	} else {
		NEC_Reset();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == nec_timer) {
		NEC_TimerRanOut();
	}
}

void NEC_PushBit(uint8_t bit) {
	bit &= 1;
	if (pos == 31) { // This is the last bit
		rawdata |= bit << pos;
		// LOG_DBG("R %x",rawdata);

		if (EXTENDED == 0) { // if it is not extended check consistency of address and command
			// Check the received data for consistency
			uint8_t a = 0;
			uint8_t na = 0;

			a = (uint8_t)(rawdata & 0xFF);
			na = (uint8_t)((rawdata & 0xFF00) >> 8);

			if (~a == na) { // address is correct

				uint8_t c = 0;
				uint8_t nc = 0;

				c = (uint8_t)((rawdata & 0xFF0000) >> 16);
				nc = (uint8_t)((rawdata & 0xFF000000) >> 24);

				if (~c == nc) { // command is correct
					frame.Address = a;
					frame.Command = c;
					NEC_ReceiveInterrupt(frame);
					lastFrame = frame;

					NEC_Reset();
					return;
				} else {
					status = 3;
					NEC_Reset();
					return;
				}

			} else {
				status = 3;
				NEC_Reset();
				return;
			}
		} else { // if it is extended check only command
			uint8_t ah = 0;
			uint8_t al = 0;
			uint16_t a = 0;
			uint8_t c = 0;
			uint8_t nc = 0;

			ah = (uint8_t)((rawdata & 0xFF00) >> 8);
			al = (uint8_t)(rawdata & 0xFF);
			a = (ah << 8) | al;
			c = (uint8_t)((rawdata & 0xFF0000) >> 16);
			nc = (uint8_t)((rawdata & 0xFF000000) >> 24);
			uint8_t r = ~c;

			if (r == nc) { // command is correct
				frame.Address = a;
				frame.Command = c;
				NEC_ReceiveInterrupt(frame);
				lastFrame = frame;
				NEC_Reset();
				return;
			} else {
				status = 3;
				NEC_Reset();
				return;
			}
		}

	} else if (pos >= 0) { // This is not the last bit
		rawdata |= bit << pos;
	}
	++pos;
}

static void dump_timing()
{
	for (int i = 0; i < dbg_s; ++i)
	{
		printf("%u %u\r\n", dbg_th[i],dbg_tl[i]);
	}
	printf("agc %d, pos %d\n", (int)AGC_OK,(int)pos);
}
void NEC_TimingDecode(uint32_t th, uint32_t tl) {
	if(dbg_s<sizeof(dbg_tl)/sizeof(dbg_tl[0])){
		dbg_tl[dbg_s] = tl;
		dbg_th[dbg_s] = th;
		dbg_s++;
	}
	// return;

	if (AGC_OK == 1) { // AGC Pulse has been received
		if ((th <= T_PULSE * (1.0 + T_TOLERANCE)) && (th >= T_PULSE * (1.0
				- T_TOLERANCE))) { // HIGH pulse is OK
			if ((tl <= T_ZERO_SPACE * (1.0 + T_TOLERANCE)) && (tl
					>= T_ZERO_SPACE * (1.0 - T_TOLERANCE))) { // LOW identified as ZERO
				NEC_PushBit(0);
			} else if ((tl <= T_ONE_SPACE * (1.0 + T_TOLERANCE)) && (tl
					>= T_ONE_SPACE * (1.0 - T_TOLERANCE))) { // LOW identified as ONE
				NEC_PushBit(1);
			} else {
				status = 3;
				// LOG_WARN("W2,%d,%d", dbg_s, tl);
				// dump_timing();
				NEC_Reset();
			}
		} else {
			status = 3;
			// LOG_WARN("W1,%d,%d", dbg_s, th);
			// dump_timing();
			NEC_Reset();
		}
	} else { //AGC Pulse has not been received yet
		if ((th <= T_AGC_PULSE * (1.0 + T_TOLERANCE)) && (th >= T_AGC_PULSE
				* (1.0 - T_TOLERANCE))) { // HIGH AGC Pulse is OK
			if ((tl <= T_AGC_SPACE * (1.0 + T_TOLERANCE)) && (tl >= T_AGC_SPACE
					* (1.0 - T_TOLERANCE))) { // LOW AGC Pulse OK
				AGC_OK = 1;
				return;
			}
			else if ((tl <= T_AGC_SPACE/2 * (1.0 + T_TOLERANCE)) && (tl >= T_AGC_SPACE/2
					* (1.0 - T_TOLERANCE))) { // LOW AGC Pulse OK
				AGC_REP = 1;
				return;
			}
		}
		// LOG_WARN("AGC fail,%u %u",th, tl);
	}
}

void NEC_HandleEXTI() {
	// GPIO_ToggleBits(GPIOD, GPIO_Pin_7);
	if (HAL_GPIO_ReadPin(IR_GPIO_Port, IR_Pin) == 0) { // pin is now HW:LOW,NEC:HIGH
		if (p == 1) {
			tlow = NEC_GetTime();
			NEC_StopTimer();
			NEC_TimingDecode(thigh, tlow);
		} else if (p == 0) {
			++p;
		}
		NEC_StartTimer();
	} else { // pin is now HW:HIGH,NEC:LOW
		thigh = NEC_GetTime();
		NEC_StopTimer();
		NEC_StartTimer();
	}
}

void NEC_StartTimer() { // Timer for overflow detection
	LL_TIM_SetCounter(nec_timer->Instance, 0);
	HAL_TIM_Base_Start_IT(nec_timer);
}

uint16_t NEC_GetTime() {
	return (uint16_t)LL_TIM_GetCounter(nec_timer->Instance);
}

void NEC_StopTimer() {
	HAL_TIM_Base_Stop_IT(nec_timer);
}
