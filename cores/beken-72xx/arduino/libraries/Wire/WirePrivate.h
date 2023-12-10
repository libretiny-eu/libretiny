/* Copyright (c) Kuba Szczodrzyński 2023-12-08. */

#pragma once

#include <ArduinoPrivate.h>
#include <Wire.h>

struct WireData {
	RingBuffer *buf;
};

typedef volatile struct {
	union {
		struct {
			uint32_t en : 1;		 /**< bit[0] */
			uint32_t start : 1;		 /**< bit[1] */
			uint32_t stop : 1;		 /**< bit[2] */
			uint32_t tx_ack : 1;	 /**< bit[3] */
			uint32_t tx_mode : 1;	 /**< bit[4] */
			uint32_t reserved0 : 1;	 /**< bit[5] */
			uint32_t freq_div : 10;	 /**< bit[6:15] */
			uint32_t sm_int : 1;	 /**< bit[16] */
			uint32_t rx_ack : 1;	 /**< bit[17] */
			uint32_t ack_req : 1;	 /**< bit[18] */
			uint32_t busy : 1;		 /**< bit[19] */
			uint32_t reserved1 : 12; /**< bit[20:31] */
		};

		uint32_t value;
	} config;

	union {
		struct {
			uint32_t data : 8;		/**< bit[0:7] */
			uint32_t reserved : 24; /**< bit[8:31] */
		};

		uint32_t value;
	} data;
} i2c1_hw_t;

typedef volatile struct {
	union {
		struct {
			uint32_t idle_cr : 3;	  /**< bit[0:2] */
			uint32_t scl_cr : 3;	  /**< bit[3:5] */
			uint32_t freq_div : 10;	  /**< bit[6:15] */
			uint32_t slave_addr : 10; /**< bit[16:25] */
			uint32_t clk_src : 2;	  /**< bit[26:27] */
			uint32_t timeout_en : 1;  /**< bit[28] */
			uint32_t idle_det_en : 1; /**< bit[29] */
			uint32_t inh : 1;		  /**< bit[30] */
			uint32_t en : 1;		  /**< bit[31] */
		};

		uint32_t value;
	} config;

	union {
		struct {
			uint32_t sm_int : 1;		/**< bit[0] */
			uint32_t scl_timeout : 1;	/**< bit[1] */
			uint32_t reserved0 : 1;		/**< bit[2] */
			uint32_t arb_lost : 1;		/**< bit[3] */
			uint32_t rx_fifo_empty : 1; /**< bit[4] */
			uint32_t tx_fifo_full : 1;	/**< bit[5] */
			uint32_t int_mode : 2;		/**< bit[6:7] */
			uint32_t ack : 1;			/**< bit[8] */
			uint32_t stop : 1;			/**< bit[9] */
			uint32_t start : 1;			/**< bit[10] */
			uint32_t addr_match : 1;	/**< bit[11] */
			uint32_t ack_req : 1;		/**< bit[12] */
			uint32_t tx_mode : 1;		/**< bit[13] */
			uint32_t master : 1;		/**< bit[14] */
			uint32_t busy : 1;			/**< bit[15] */
			uint32_t reserved1 : 16;	/**< bit[16:31] */
		};

		uint32_t value;
	} status;

	union {
		struct {
			uint32_t data : 8;		/**< bit[0:7] */
			uint32_t reserved : 24; /**< bit[8:31] */
		};

		uint32_t value;
	} data;
} i2c2_hw_t;

#define REG_I2C1 ((i2c1_hw_t *)I2C1_BASE_ADDR)
#define REG_I2C2 ((i2c2_hw_t *)I2C2_BASE_ADDR)
