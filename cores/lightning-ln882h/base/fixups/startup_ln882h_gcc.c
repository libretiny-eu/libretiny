
#include "ln882h.h"
#include "ln_compiler.h"

/*----------------------------------------------------------------------------
  Linker generated Symbols
 *----------------------------------------------------------------------------*/
extern uint32_t __copysection_ram0_load;
extern uint32_t __copysection_ram0_start;
extern uint32_t __copysection_ram0_end;
extern uint32_t __etext;
extern uint32_t __bss_ram0_start__;
extern uint32_t __bss_ram0_end__;
extern uint32_t __StackTop;
extern uint32_t __retention_start__;
extern uint32_t __retention_end__;


/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void( *pFunc )( void );


/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
//extern void _start     (void) __attribute__((noreturn)); /* PreeMain (C library entry point) */
extern void lt_main (void);


/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
void Reset_Handler  (void) ;
__WEAK__ void NMI_Handler            (void);
__WEAK__ void HardFault_Handler      (void);
__WEAK__ void MemManage_Handler      (void);
__WEAK__ void BusFault_Handler       (void);
__WEAK__ void UsageFault_Handler     (void);
__WEAK__ void SVC_Handler            (void);
__WEAK__ void DebugMon_Handler       (void);
__WEAK__ void PendSV_Handler         (void);
__WEAK__ void SysTick_Handler        (void);
__WEAK__ void WDT_IRQHandler         (void);
__WEAK__ void EXT_IRQHandler         (void);
__WEAK__ void RTC_IRQHandler         (void);
__WEAK__ void RFSLP_IRQHandler       (void);
__WEAK__ void MAC_IRQHandler         (void);
__WEAK__ void BLE_WAKE_IRQHandler    (void);
__WEAK__ void BLE_ERR_IRQHandler     (void);
__WEAK__ void BLE_MAC_IRQHandler     (void);
__WEAK__ void DMA_IRQHandler         (void);
__WEAK__ void QSPI_IRQHandler        (void);
__WEAK__ void SDIO_F1_IRQHandler     (void);
__WEAK__ void SDIO_F2_IRQHandler     (void);
__WEAK__ void SDIO_F3_IRQHandler     (void);
__WEAK__ void CM4_FPIXC_IRQHandler   (void);
__WEAK__ void CM4_FPOFC_IRQHandler   (void);
__WEAK__ void CM4_FPUFC_IRQHandler   (void);
__WEAK__ void CM4_FPIOC_IRQHandler   (void);
__WEAK__ void CM4_FPDZC_IRQHandler   (void);
__WEAK__ void CM4_FPIDC_IRQHandler   (void);
__WEAK__ void I2C_IRQHandler         (void);
__WEAK__ void SPI0_IRQHandler        (void);
__WEAK__ void SPI1_IRQHandler        (void);
__WEAK__ void UART0_IRQHandler       (void);
__WEAK__ void UART1_IRQHandler       (void);
__WEAK__ void UART2_IRQHandler       (void);
__WEAK__ void ADC_IRQHandler         (void);
__WEAK__ void WS_IRQHandler          (void);
__WEAK__ void I2S_IRQHandler         (void);
__WEAK__ void GPIOA_IRQHandler       (void);
__WEAK__ void GPIOB_IRQHandler       (void);
__WEAK__ void TIMER0_IRQHandler      (void);
__WEAK__ void TIMER1_IRQHandler      (void);
__WEAK__ void TIMER2_IRQHandler      (void);
__WEAK__ void TIMER3_IRQHandler      (void);
__WEAK__ void ADV_TIMER_IRQHandler   (void);
__WEAK__ void AES_IRQHandler         (void);
__WEAK__ void TRNG_IRQHandler        (void);
__WEAK__ void PAOTD_IRQHandler       (void);


/*----------------------------------------------------------------------------
  User Initial Stack & Heap
 *----------------------------------------------------------------------------*/
//<h> Stack Configuration
//  <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
//</h>
#define  __STACK_SIZE  0x00000600
static uint8_t stack[__STACK_SIZE] __attribute__ ((aligned(8), used, section(".stack")));

#if 0
//<h> Heap Configuration
//  <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
//</h>
#define  __HEAP_SIZE   0x00004000
#if __HEAP_SIZE > 0
static uint8_t heap[__HEAP_SIZE]   __attribute__ ((aligned(8), used, section(".heap")));
#endif
#endif


/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
__WEAK__ void NMI_Handler            (void) { while(1); }
__WEAK__ void HardFault_Handler      (void) { while(1); }
__WEAK__ void MemManage_Handler      (void) { while(1); }
__WEAK__ void BusFault_Handler       (void) { while(1); }
__WEAK__ void UsageFault_Handler     (void) { while(1); }
__WEAK__ void SVC_Handler            (void) { while(1); }
__WEAK__ void DebugMon_Handler       (void) { while(1); }
__WEAK__ void PendSV_Handler         (void) { while(1); }
__WEAK__ void SysTick_Handler        (void) { while(1); }
__WEAK__ void WDT_IRQHandler         (void) { while(1); }
__WEAK__ void EXT_IRQHandler         (void) { while(1); }
__WEAK__ void RTC_IRQHandler         (void) { while(1); }
__WEAK__ void RFSLP_IRQHandler       (void) { while(1); }
__WEAK__ void MAC_IRQHandler         (void) { while(1); }
__WEAK__ void BLE_WAKE_IRQHandler    (void) { while(1); }
__WEAK__ void BLE_ERR_IRQHandler     (void) { while(1); }
__WEAK__ void BLE_MAC_IRQHandler     (void) { while(1); }
__WEAK__ void DMA_IRQHandler         (void) { while(1); }
__WEAK__ void QSPI_IRQHandler        (void) { while(1); }
__WEAK__ void SDIO_F1_IRQHandler     (void) { while(1); }
__WEAK__ void SDIO_F2_IRQHandler     (void) { while(1); }
__WEAK__ void SDIO_F3_IRQHandler     (void) { while(1); }
__WEAK__ void CM4_FPIXC_IRQHandler   (void) { while(1); }
__WEAK__ void CM4_FPOFC_IRQHandler   (void) { while(1); }
__WEAK__ void CM4_FPUFC_IRQHandler   (void) { while(1); }
__WEAK__ void CM4_FPIOC_IRQHandler   (void) { while(1); }
__WEAK__ void CM4_FPDZC_IRQHandler   (void) { while(1); }
__WEAK__ void CM4_FPIDC_IRQHandler   (void) { while(1); }
__WEAK__ void I2C_IRQHandler         (void) { while(1); }
__WEAK__ void SPI0_IRQHandler        (void) { while(1); }
__WEAK__ void SPI1_IRQHandler        (void) { while(1); }
__WEAK__ void UART0_IRQHandler       (void) { while(1); }
__WEAK__ void UART1_IRQHandler       (void) { while(1); }
__WEAK__ void UART2_IRQHandler       (void) { while(1); }
__WEAK__ void ADC_IRQHandler         (void) { while(1); }
__WEAK__ void WS_IRQHandler          (void) { while(1); }
__WEAK__ void I2S_IRQHandler         (void) { while(1); }
__WEAK__ void GPIOA_IRQHandler       (void) { while(1); }
__WEAK__ void GPIOB_IRQHandler       (void) { while(1); }
__WEAK__ void TIMER0_IRQHandler      (void) { while(1); }
__WEAK__ void TIMER1_IRQHandler      (void) { while(1); }
__WEAK__ void TIMER2_IRQHandler      (void) { while(1); }
__WEAK__ void TIMER3_IRQHandler      (void) { while(1); }
__WEAK__ void ADV_TIMER_IRQHandler   (void) { while(1); }
__WEAK__ void AES_IRQHandler         (void) { while(1); }
__WEAK__ void TRNG_IRQHandler        (void) { while(1); }
__WEAK__ void PAOTD_IRQHandler       (void) { while(1); }

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
extern const pFunc __Vectors[240];
const pFunc __Vectors[240] __attribute__ ((section(".vectors"))) = {
    (pFunc)(&__StackTop),                     /* (0x00)Top of Stack                   */
    Reset_Handler,                            /* (0x04)IRQ -15  Reset Handler         */
    NMI_Handler,                              /* (0x08)IRQ -14  NMI Handler           */
    HardFault_Handler,                        /* (0x0C)IRQ -13  Hard Fault Handler    */
    MemManage_Handler,                        /* (0x10)IRQ -12  MPU Fault Handler     */
    BusFault_Handler,                         /* (0x14)IRQ -11  Bus Fault Handler     */
    UsageFault_Handler,                       /* (0x18)IRQ -10  Usage Fault Handler   */
    0,                                        /* (0x1C)IRQ -9   Reserved              */
    0,                                        /* (0x20)IRQ -8   Reserved              */
    0,                                        /* (0x24)IRQ -7   Reserved              */
    0,                                        /* (0x28)IRQ -6   Reserved              */
    SVC_Handler,                              /* (0x2C)IRQ -5   SVCall Handler        */
    DebugMon_Handler,                         /* (0x30)IRQ -4   Debug Monitor Handler */
    0,                                        /* (0x34)IRQ -3   Reserved              */
    PendSV_Handler,                           /* (0x38)IRQ -2   PendSV Handler        */
    SysTick_Handler,                          /* (0x3C)IRQ -1   SysTick Handler       */

    /* Interrupts */
    WDT_IRQHandler,                           /* (0x40)IRQ0  */
    EXT_IRQHandler,                           /* (0x44)IRQ1  */
    RTC_IRQHandler,                           /* (0x48)IRQ2  */
    RFSLP_IRQHandler,                         /* (0x4C)IRQ3  */
    MAC_IRQHandler,                           /* (0x50)IRQ4  */
    BLE_WAKE_IRQHandler,                      /* (0x54)IRQ5  */
    BLE_ERR_IRQHandler,                       /* (0x58)IRQ6  */
    BLE_MAC_IRQHandler,                       /* (0x5C)IRQ7  */
    DMA_IRQHandler,                           /* (0x60)IRQ8  */
    QSPI_IRQHandler,                          /* (0x64)IRQ9  */
    SDIO_F1_IRQHandler,                       /* (0x68)IRQ10 */
    SDIO_F2_IRQHandler,                       /* (0x6C)IRQ11 */
    SDIO_F3_IRQHandler,                       /* (0x70)IRQ12 */
    CM4_FPIXC_IRQHandler,                     /* (0x74)IRQ13 */
    CM4_FPOFC_IRQHandler,                     /* (0x78)IRQ14 */
    CM4_FPUFC_IRQHandler,                     /* (0x7C)IRQ15 */
    CM4_FPIOC_IRQHandler,                     /* (0x80)IRQ16 */
    CM4_FPDZC_IRQHandler,                     /* (0x84)IRQ17 */
    CM4_FPIDC_IRQHandler,                     /* (0x88)IRQ18 */
    I2C_IRQHandler,                           /* (0x8C)IRQ19 */
    SPI0_IRQHandler,                          /* (0x90)IRQ20 */
    SPI1_IRQHandler,                          /* (0x94)IRQ21 */
    UART0_IRQHandler,                         /* (0x98)IRQ22 */
    UART1_IRQHandler,                         /* (0x9C)IRQ23 */
    UART2_IRQHandler,                         /* (0xA0)IRQ24 */
    ADC_IRQHandler,                           /* (0xA4)IRQ25 */
    WS_IRQHandler,                            /* (0xA8)IRQ26 */
    I2S_IRQHandler,                           /* (0xAC)IRQ27 */
    GPIOA_IRQHandler,                         /* (0xB0)IRQ28 */
    GPIOB_IRQHandler,                         /* (0xB4)IRQ29 */
    TIMER0_IRQHandler,                        /* (0xB8)IRQ30 */
    TIMER1_IRQHandler,                        /* (0xBC)IRQ31 */
    TIMER2_IRQHandler,                        /* (0xC0)IRQ32 */
    TIMER3_IRQHandler,                        /* (0xC4)IRQ33 */
    ADV_TIMER_IRQHandler,                     /* (0xC8)IRQ34 */
    AES_IRQHandler,                           /* (0xCC)IRQ35 */
    TRNG_IRQHandler,                          /* (0xD0)IRQ36 */
    PAOTD_IRQHandler,                         /* (0xD4)IRQ37 */
};

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void) {
    uint32_t *pSrc, *pDest;
    uint32_t *pTable __attribute__((unused));

    /* Firstly it copies data from read only memory to RAM.
     * There are two schemes to copy. One can copy more than one sections.
     * Another can copy only one section. The former scheme needs more
     * instructions and read-only data to implement than the latter.
     * Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.
     */
    pSrc  = &__copysection_ram0_load;
    pDest = &__copysection_ram0_start;

    for ( ; (pDest < &__copysection_ram0_end); ) {
        *pDest++ = *pSrc++;
    }



    /* Single BSS section scheme.
     *
     * The BSS section is specified by following symbols
     *   __bss_start__: start of the BSS section.
     *   __bss_end__: end of the BSS section.
     *
     * Both addresses must be aligned to 4 bytes boundary.
     */
    pDest = &__bss_ram0_start__;
    for ( ; pDest < &__bss_ram0_end__ ; ) {
        *pDest++ = 0UL;
    }

    pDest = &__retention_start__;
    for ( ; pDest < &__retention_end__ ; ) {
        *pDest++ = 0UL;
    }

    SystemInit();                             /* CMSIS System Initialization */
    lt_main();
}
