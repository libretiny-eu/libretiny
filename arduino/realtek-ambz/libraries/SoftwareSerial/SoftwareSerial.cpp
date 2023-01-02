/* Copyright (c) Kuba Szczodrzyński 2022-07-03. */

#include "SoftwareSerial.h"

extern "C" {

#include <timer_api.h>

} // extern "C"

#define TIMER_MAX 3
#define OBJ		  ((gtimer_t *)this->param)

static uint32_t timNum[TIMER_MAX]  = {TIMER1, TIMER2, TIMER3};
static gtimer_t *timObj[TIMER_MAX] = {NULL, NULL, NULL};

static void callback(SoftSerial *data) {
	SoftData *tx = &data->tx;

	switch (tx->state) {
		case SS_IDLE:
			goto finish;

		case SS_END:
		case SS_START:
			if (!tx->buf->available())
				goto finish;
			tx->byte = tx->buf->read_char();
			digitalWrite(tx->pin, LOW);
			tx->state = SS_DATA0;
			return;

		case SS_STOP:
			digitalWrite(tx->pin, HIGH);
			break;

		default:
			digitalWrite(tx->pin, (tx->byte & 0x1) ^ data->invert);
			tx->byte /= 2;
			break;
	}

	tx->state = (SoftState)(tx->state + 1);
	return;

finish:
	gtimer_stop((gtimer_t *)data->param);
	data->tx.state = SS_IDLE;
	return;
}

void SoftwareSerial::begin(unsigned long baudrate, uint16_t config) {
	if (data.rx.buf || data.tx.buf)
		return;

	uint8_t i;
	for (i = 0; i < TIMER_MAX; i++) {
		if (timObj[i] == NULL)
			break;
	}
	if (i == TIMER_MAX) {
		LT_E("No more timers for SoftwareSerial");
		return;
	}

	pinMode(data.tx.pin, OUTPUT);
	digitalWrite(data.tx.pin, HIGH);

	data.rx.buf	  = new RingBuffer();
	data.tx.buf	  = new RingBuffer();
	data.rx.state = SS_IDLE;
	data.tx.state = SS_IDLE;

	uint32_t us = 1E6 / baudrate;

	timObj[i] = (gtimer_t *)malloc(sizeof(gtimer_t));
	param = data.param = data.tx.param = timObj[i];
	gtimer_init(OBJ, timNum[i]);
	OBJ->is_periodcal = true;
	OBJ->handler	  = (void *)callback;
	OBJ->hid		  = (uint32_t)&data;
	gtimer_reload(OBJ, us);
}

void SoftwareSerial::end() {
	if (!(bool)this)
		return;
	gtimer_stop(OBJ);
	gtimer_deinit(OBJ);
	free(OBJ);
	delete data.rx.buf;
	delete data.tx.buf;
}

void SoftwareSerial::startTx() {
	gtimer_start(OBJ);
}

void SoftwareSerial::endTx() {
	gtimer_stop(OBJ);
}
