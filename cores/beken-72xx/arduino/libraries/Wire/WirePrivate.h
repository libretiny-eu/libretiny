/* Copyright (c) Kuba Szczodrzyński 2023-12-08. */

#pragma once

#include <ArduinoPrivate.h>
#include <Wire.h>

struct WireData {
	RingBuffer *buf;
};

// Register structures based on:
// - https://github.com/bekencorp/armino/blob/main/middleware/soc/bk7231n/soc/i2c_struct.h
// - https://wiki.bekencorp.com/pages/viewpage.action?pageId=23692262

typedef volatile struct {
	union {
		struct {
			uint32_t ENSMB : 1;		  //!< I2C interface enabled [RW] (0x0[0])
			uint32_t STA : 1;		  //!< Start/rep.start generation [RW] (0x0[1])
			uint32_t STO : 1;		  //!< Stop bit generation [RW] (0x0[2])
			uint32_t ACKTX : 1;		  //!< ACK TX generation [RW] (0x0[3])
			uint32_t TXMODE : 1;	  //!< Transmitter mode [RW] (0x0[4])
			uint32_t _reserved0 : 1;  //!< (0x0[5])
			uint32_t FREQ_DIV : 10;	  //!< Clock frequency divider [RW] (0x0[6:15])
			uint32_t SI : 1;		  //!< I2C RX/TX completed [RW] (0x0[16])
			uint32_t ACKRX : 1;		  //!< ACK RX detection [R] (0x0[17])
			uint32_t ACKRQ : 1;		  //!< ACK response request (RX mode) [R] (0x0[18])
			uint32_t BUSY : 1;		  //!< Bus busy [R] (0x0[19])
			uint32_t _reserved1 : 12; //!< (0x0[20:31])
		};

		uint32_t config;
	};

	union {
		struct {
			uint32_t SMB_DAT : 8;	  //!< Data register [RW] (0x1[0:7])
			uint32_t _reserved2 : 24; //!< (0x1[8:31])
		};

		uint32_t data;
	};
} i2c1_hw_t;

typedef volatile struct {
	union {
		struct {
			uint32_t IDLE_CR : 3;	//!< The bus idle detection threshold [RW] (0x0[0:2])
			uint32_t SCL_CR : 3;	//!< SCL low level timeout threshold [RW] (0x0[3:5])
			uint32_t FREQ_DIV : 10; //!< Clock frequency divider [RW] (0x0[6:15])
			uint32_t SLV_ADDR : 10; //!< Slave address [RW] (0x0[16:25])
			uint32_t SMBCS : 2;		//!< Interface clock source selection [RW] (0x0[26:27])
			uint32_t SMBTOE : 1;	//!< SCL low level timeout detection enable [RW] (0x0[28])
			uint32_t SMBFTE : 1;	//!< Bus idle detection enabled [RW] (0x0[29])
			uint32_t INH : 1;		//!< Slave mode (address matching) disabled [RW] (0x0[30])
			uint32_t ENSMB : 1;		//!< I2C interface enabled [RW] (0x0[31])
		};

		uint32_t config;
	};

	union {
		struct {
			uint32_t SI : 1;		   //!< I2C RX/TX completed [RW] (0x1[0])
			uint32_t SCL_TMOT : 1;	   //!< SCL low level timeout [RW] (0x1[1])
			uint32_t _reserved0 : 1;   //!< (0x1[2])
			uint32_t ARBLOST : 1;	   //!< Multi-master arbitration lost [RW] (0x1[3])
			uint32_t RXFIFO_EMPTY : 1; //!< Receive FIFO empty [R] (0x1[4])
			uint32_t TXFIFO_FULL : 1;  //!< Send FIFO full [R] (0x1[5])
			uint32_t INT_MODE : 2;	   //!< FIFO threshold configuration [RW] (0x1[6:7])
			uint32_t ACK : 1;		   //!< ACK generation/detection [RW] (0x1[8])
			uint32_t STO : 1;		   //!< Stop bit generation/detection [RW] (0x1[9])
			uint32_t STA : 1;		   //!< Start/rep.start generation (master mode) [RW] (0x1[10])
			uint32_t ADDR_MATCH : 1;   //!< Slave address matched (slave mode) [R] (0x1[11])
			uint32_t ACKRQ : 1;		   //!< ACK response request (RX mode) [R] (0x1[12])
			uint32_t TXMODE : 1;	   //!< Transmitter mode [R] (0x1[13])
			uint32_t MASTER : 1;	   //!< Master/slave mode [R] (0x1[14])
			uint32_t BUSY : 1;		   //!< Bus busy [R] (0x1[15])
			uint32_t _reserved1 : 16;  //!< (0x1[16:31])
		};

		uint32_t status;
	};

	union {
		struct {
			uint32_t SMB_DAT : 8;	  //!< Data register [RW] (0x2[0:7])
			uint32_t _reserved2 : 24; //!< (0x2[8:31])
		};

		uint32_t data;
	};
} i2c2_hw_t;

#define REG_I2C1 ((i2c1_hw_t *)I2C1_BASE_ADDR)
#define REG_I2C2 ((i2c2_hw_t *)I2C2_BASE_ADDR)
