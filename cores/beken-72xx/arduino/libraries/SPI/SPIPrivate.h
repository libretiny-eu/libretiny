/* Copyright (c) Kuba Szczodrzyński 2024-03-15. */

#pragma once

#include <ArduinoPrivate.h>
#include <SPI.h>

struct SPIData {};

// Register structures based on:
// - https://github.com/bekencorp/armino/blob/main/middleware/soc/bk7231n/soc/spi_struct.h
// - https://wiki.bekencorp.com/pages/viewpage.action?pageId=23692278
// - BDK beken378/driver/spi/*.h

typedef volatile struct {
	union {
		struct {
			uint32_t TX_FIFO_INT_LEVEL : 2; //!< TX FIFO interrupt generation condition (1/16/32/48) [RW] (0x0[0:1])
			uint32_t RX_FIFO_INT_LEVEL : 2; //!< RX FIFO interrupt generation condition (1/16/32/48) [RW] (0x0[2:3])
			uint32_t TX_UDF_INT_EN : 1;		//!< TX FIFO underflow interrupt enable [RW] (0x0[4])
			uint32_t RX_OVF_INT_EN : 1;		//!< RX FIFO overflow interrupt enable [RW] (0x0[5])
			uint32_t TX_FIFO_INT_EN : 1;	//!< TX FIFO interrupt enable [RW] (0x0[6])
			uint32_t RX_FIFO_INT_EN : 1;	//!< RX FIFO interrupt enable [RW] (0x0[7])
			uint32_t SPI_CKR : 8;			//!< Prescaler factor [RW] (0x0[8:15])
#if LT_BK7231N || LT_BK7271
			uint32_t SLV_RELEASE_INT_EN : 1; //!< Slave release interrupt enable (four-wire slave mode) [RW] (0x0[16])
			uint32_t WIRE3_EN : 1;			 //!< Three-wire mode (no CSN signal) [RW] (0x0[17])
#else
			uint32_t NSSMD : 2;		 //!< (0x0[16:17])
#endif
			uint32_t BIT_WIDTH : 1; //!< Data bit width (0: 8-bit, 1: 16-bit) [RW] (0x0[18])
#if LT_BK7231N || LT_BK7271
			uint32_t LSB_FIRST : 1; //!< Data frame format (0: MSB-first, 1: LSB-first) [RW] (0x0[19])
#else
			uint32_t _reserved0 : 1; //!< (0x0[19])
#endif
			uint32_t CKPOL : 1; //!< SPI clock polarity [RW] (0x0[20])
			uint32_t CKPHA : 1; //!< SPI clock phase [RW] (0x0[21])
			uint32_t MSTEN : 1; //!< Master mode enable [RW] (0x0[22])
			uint32_t SPIEN : 1; //!< SPI enable [RW] (0x0[23])
#if LT_BK7231N || LT_BK7271
			uint32_t BYTE_INTLVAL : 8; //!< SCK interval between data units (master) [RW] (0x0[24:31])
#else
			uint32_t _reserved1 : 8; //!< (0x0[24:31])
#endif
		};

		uint32_t ctrl;
	};

#if LT_BK7231N || LT_BK7271
	union {
		struct {
			uint32_t TX_EN : 1;			   //!< TX enable [RW] (0x1[0])
			uint32_t RX_EN : 1;			   //!< RX enable [RW] (0x1[1])
			uint32_t TX_FINISH_INT_EN : 1; //!< TX_TRANS_LEN completed interrupt enabled [RW] (0x1[2])
			uint32_t RX_FINISH_INT_EN : 1; //!< RX_TRANS_LEN completed interrupt enable [RW] (0x1[3])
			uint32_t _reserved2 : 4;	   //!< (0x1[4:7])
			uint32_t TX_TRANS_LEN : 12;	   //!< TX data length (bits), 0: unlimited [RW] (0x1[8:19])
			uint32_t RX_TRANS_LEN : 12;	   //!< RX data length (bits), 0: unlimited [RW] (0x1[20:31])
		};

		uint32_t config;
	};
#endif

#if LT_BK7231N || LT_BK7271
	union {
		struct {
			uint32_t _reserved3 : 1;	   //!< (0x2[0])
			uint32_t TX_FIFO_WR_READY : 1; //!< TX FIFO write allowed [R] (0x2[1])
			uint32_t RX_FIFO_RD_READY : 1; //!< RX FIFO read allowed [R] (0x2[2])
			uint32_t _reserved4 : 1;	   //!< (0x2[3])
			uint32_t _reserved5 : 4;	   //!< (0x2[4:7])
			uint32_t TX_FIFO_INT : 1;	   //!< TX FIFO interrupt (1: clear) [RW] (0x2[8])
			uint32_t RX_FIFO_INT : 1;	   //!< RX FIFO interrupt (1: clear) [RW] (0x2[9])
			uint32_t SLV_RELEASE_INT : 1;  //!< Slave release interrupt (1: clear) [RW] (0x2[10])
			uint32_t TX_UDF_INT : 1;	   //!< TX FIFO underflow interrupt (1: clear) [RW] (0x2[11])
			uint32_t RX_OVF_INT : 1;	   //!< RX FIFO overflow interrupt (1: clear) [RW] (0x2[12])
			uint32_t TX_FINISH_INT : 1;	   //!< TX_TRANS_LEN completed interrupt [R] (0x2[13])
			uint32_t RX_FINISH_INT : 1;	   //!< RX_TRANS_LEN completed interrupt [R] (0x2[14])
			uint32_t _reserved6 : 1;	   //!< (0x2[15])
			uint32_t TX_FIFO_CLR : 1;	   //!< TX FIFO clear enable [W] (0x2[16])
			uint32_t RX_FIFO_CLR : 1;	   //!< RX FIFO clear enable [W] (0x2[17])
			uint32_t _reserved7 : 14;	   //!< (0x2[18:31])
		};

		uint32_t status;
	};
#else
	union {
		struct {
			uint32_t TX_FIFO_EMPTY : 1; //!< (0x1[0])
			uint32_t TX_FIFO_FULL : 1;	//!< (0x1[1])
			uint32_t RX_FIFO_EMPTY : 1; //!< (0x1[2])
			uint32_t RX_FIFO_FULL : 1;	//!< (0x1[3])
			uint32_t _reserved8 : 4;	//!< (0x2[4:7])
			uint32_t TX_FIFO_INT : 1;	//!< TX FIFO interrupt (1: clear) [RW] (0x2[8])
			uint32_t RX_FIFO_INT : 1;	//!< RX FIFO interrupt (1: clear) [RW] (0x2[9])
			uint32_t MODF : 1;			//!< (0x1[10])
			uint32_t TX_UDF_INT : 1;	//!< TX FIFO underflow interrupt (1: clear) [RW] (0x2[11])
			uint32_t RX_OVF_INT : 1;	//!< RX FIFO overflow interrupt (1: clear) [RW] (0x2[12])
			uint32_t _reserved9 : 1;	//!< (0x1[13])
			uint32_t SLVSEL : 1;		//!< (0x1[14])
			uint32_t SPIBUSY : 1;		//!< (0x1[15])
			uint32_t _reserved10 : 16;	//!< (0x1[16:31])
		};

		uint32_t status;
	};
#endif

	union {
		struct {
			uint32_t SPI_DAT : 16;	   //!< FIFO read/write operation [RW] (0x2[0:15])
			uint32_t _reserved11 : 16; //!< (0x2[16:31])
		};

		uint32_t data;
	};

#if !(LT_BK7231N || LT_BK7271)
	union {
		struct {
			uint32_t _reserved12 : 1;		 //!< (0x3[0])
			uint32_t S_CS_UP_INT_EN : 1;	 //!< (0x3[1])
			uint32_t _reserved13 : 2;		 //!< (0x3[2:3])
			uint32_t S_CS_UP_INT_STATUS : 1; //!< (0x3[4])
			uint32_t _reserved14 : 27;		 //!< (0x3[5:31])
		};

		uint32_t slave_ctrl;
	};
#endif
} spi_hw_t;

#if LT_BK7271
#define REG_SPI0 ((spi_hw_t *)0x00802500)
#define REG_SPI1 ((spi_hw_t *)0x00802540)
#define REG_SPI2 ((spi_hw_t *)0x00802580)
#else
#define REG_SPI0 ((spi_hw_t *)0x00802700)
#endif
