#include "include.h"
#include "sys_ctrl.h"

#include "flash_pub.h"
#include "power_save_pub.h"
#include "gpio_pub.h"
#include "reg_rc.h"
#include "manual_ps_pub.h"

// usage:
// extern void sctrl_enter_rtos_deep_sleep_fix(PS_DEEP_CTRL_PARAM *deep_param);

static void sctrl_mac_ahb_slave_clock_disable(void)
{
    UINT32 reg;
#if (CFG_SOC_NAME == SOC_BK7271)
    reg = REG_READ(SCTRL_CONTROL);
    reg &= ~MAC_HCLK_EN_BIT;
    REG_WRITE(SCTRL_CONTROL, reg);
#else
    reg = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
    reg &= ~MAC_HCLK_EN_BIT;
    REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg);
#endif
}

// This is a patched version, where `gpio_stay_*_map` arguments are actually
// taken into account as intended. The offical doc says that they control
// whether the original voltage should 'stay' (float) instead of configuring
// pins to pullup/pulldown.
//
// https://docs.bekencorp.com/sdk_3.0.x/bk7238/build/en/latest/developer-guide/power_save/sleep_test.html
void sctrl_enter_rtos_deep_sleep_fix(PS_DEEP_CTRL_PARAM *deep_param)
{
    DD_HANDLE flash_hdl;
    UINT32 status;
    UINT32 param;
    UINT32 reg;
    UINT32 i;

    uart_wait_tx_over();

    /* close all peri clock*/
    REG_WRITE(ICU_PERI_CLK_PWD, 0xfffff);  //  icu: 0x2;

#if CFG_USE_UART2
    uart2_exit();
#endif
#if CFG_USE_UART1
    uart1_exit();
#endif

#if (CFG_SOC_NAME == SOC_BK7231U) || (SOC_BK7231N == CFG_SOC_NAME)
    reg = REG_READ(SCTRL_LOW_PWR_CLK);
    reg &=~(LPO_CLK_MUX_MASK);
    reg |=(LPO_SRC_ROSC << LPO_CLK_MUX_POSI);
    REG_WRITE(SCTRL_LOW_PWR_CLK, reg);

    REG_WRITE(SCTRL_ROSC_CAL, 0x75);
    REG_WRITE(SCTRL_ROSC_CAL, 0x77);
#else
    /*ana_reg set*/
    REG_WRITE(SCTRL_ANALOG_CTRL0, 0x7819a59b);
    REG_WRITE(SCTRL_ANALOG_CTRL1, 0x7819a59b);
    REG_WRITE(SCTRL_ANALOG_CTRL2, 0x84036080);
    REG_WRITE(SCTRL_ANALOG_CTRL3, 0x180004a0);
    REG_WRITE(SCTRL_ANALOG_CTRL4, 0x84200e52);
    REG_WRITE(SCTRL_ANALOG_CTRL5, 0x3b13b13b);
#if (CFG_SOC_NAME != SOC_BK7231)
    REG_WRITE(SCTRL_ANALOG_CTRL6, 0xb09350);
#endif
#if (CFG_SOC_NAME == SOC_BK7221U)
    REG_WRITE(SCTRL_ANALOG_CTRL7, 0x441a7f0);
    REG_WRITE(SCTRL_ANALOG_CTRL8, 0x3b187c);
    REG_WRITE(SCTRL_ANALOG_CTRL9, 0x82204007);
    REG_WRITE(SCTRL_ANALOG_CTRL10, 0x80801432);
#endif
#endif
    ps_delay(10);
    /*clear int*/
    REG_WRITE(ICU_INTERRUPT_ENABLE, 0);

    extern void gpio_ops_disable_filter(void);
    gpio_ops_disable_filter();

    /*  disable gpio0~31*/
    REG_WRITE(SCTRL_GPIO_WAKEUP_EN,0x0);  //sys_ctrl : 0x48;
    REG_WRITE(SCTRL_GPIO_WAKEUP_INT_STATUS,0xFFFFFFFF);  //sys_ctrl : 0x4a;

#if (CFG_SOC_NAME != SOC_BK7231N)
    /*	disable gpio32~39*/
    REG_WRITE(SCTRL_GPIO_WAKEUP_EN1,0x0);  //sys_ctrl : 0x51;
    REG_WRITE(SCTRL_GPIO_WAKEUP_INT_STATUS1,0xFF);  //sys_ctrl : 0x53;
#endif

    REG_WRITE(SCTRL_BLOCK_EN_MUX, 0x0);   //sys_ctrl : 0x4F;

    /* ROSC_TIMER_int_clear*/
    reg = REG_READ(SCTRL_ROSC_TIMER);
    reg = reg| ROSC_TIMER_INT_STATUS_BIT ;
    REG_WRITE(SCTRL_ROSC_TIMER,reg);      //sys_ctrl : 0x47;

    /*ROSC_TIMER close */
    reg = REG_READ(SCTRL_ROSC_TIMER);
    reg = reg & (~ROSC_TIMER_ENABLE_BIT);																   //'C'
    REG_WRITE(SCTRL_ROSC_TIMER,reg);

    reg = REG_READ(SCTRL_LOW_PWR_CLK);
    reg &=~(LPO_CLK_MUX_MASK);
    if(deep_param->lpo_32k_src == LPO_SELECT_32K_XTAL)
    {
        reg |=(LPO_SRC_32K_XTAL << LPO_CLK_MUX_POSI);
    }
    else
    {
        reg |=(LPO_SRC_ROSC << LPO_CLK_MUX_POSI);
    }

    REG_WRITE(SCTRL_LOW_PWR_CLK, reg);    //sys_ctrl : 0x40;

    /* close all peri int*/
    //    REG_WRITE(ICU_INTERRUPT_ENABLE, 0);

    /* MAC pwd*/
    REG_WRITE(SCTRL_PWR_MAC_MODEM, MAC_PWD << MAC_PWD_POSI);   //sys_ctrl : 0x43;

    sctrl_mac_ahb_slave_clock_disable();

    /* Mac Subsystem clock 480m disable*/
    reg = REG_READ(SCTRL_CONTROL);
    REG_WRITE(SCTRL_CONTROL, reg | MAC_CLK480M_PWD_BIT);

    /* Modem pwd*/
    REG_WRITE(SCTRL_PWR_MAC_MODEM, MODEM_PWD << MODEM_PWD_POSI);

    /* Modem AHB clock disable*/
    reg = REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK);
    reg &= ~PHY_HCLK_EN_BIT;
    REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, reg);

    /* Modem Subsystem clock 480m disable*/
    reg = REG_READ(SCTRL_CONTROL);
    REG_WRITE(SCTRL_CONTROL, reg | MODEM_CLK480M_PWD_BIT);

    /* Flash 26MHz clock select dco clock*/
    flash_hdl = ddev_open(FLASH_DEV_NAME, &status, 0);
    ASSERT(DD_HANDLE_UNVALID != flash_hdl);
    ddev_control(flash_hdl, CMD_FLASH_SET_DCO, 0);

    /* MCLK(main clock) select:dco*/ /* MCLK division*/
    reg = REG_READ(SCTRL_CONTROL);
    reg &= ~(MCLK_MUX_MASK << MCLK_MUX_POSI);
    reg &= ~(MCLK_DIV_MASK << MCLK_DIV_POSI);
    REG_WRITE(SCTRL_CONTROL, reg);                                     //0x02

    if(deep_param->lpo_32k_src == LPO_SELECT_32K_XTAL)
    {
        reg = REG_READ(SCTRL_CONTROL);
        reg =((reg & (~0xF0)) | (0<<4));
        reg =((reg & (~0x03)) | (0<<MCLK_MUX_POSI));
        reg =((reg & (~0x100)) | FLASH_26M_MUX_BIT);
        REG_WRITE(SCTRL_CONTROL,reg); //sys_ctrl : 0x02;
    }

    ps_delay(10);

    reg = 0x0;
    reg = (reg &(~(BLOCK_EN_WORD_MASK << 20))&(~(0x7FFFUL<<5)) &(~(0x01UL<<1)));
    reg = (reg |(BLOCK_EN_WORD_PWD<< 20 )|BLK_EN_FLASH|BLK_EN_ROSC32K|BLK_EN_DIGITAL_CORE|BLK_EN_ANALOG_SYS_LDO);
    if(deep_param->lpo_32k_src == LPO_SELECT_32K_XTAL)
    {
        reg = (reg |BLK_EN_32K_XTAL|BLK_EN_26M_XTAL);
    }
    REG_WRITE(SCTRL_BLOCK_EN_CFG, reg);                  //sys_ctrl : 0x4B;                   //'E'

#if (CFG_SOC_NAME != SOC_BK7231U) && (SOC_BK7231N != CFG_SOC_NAME)
    reg = REG_READ(SCTRL_ROSC_CAL);                           //ROSC Calibration disable
    reg =(reg  & (~0x01));
    REG_WRITE(SCTRL_ROSC_CAL, reg);
#endif

    for(i=0; i<GPIONUM; i++)
    {
#if (CFG_SOC_NAME == SOC_BK7231N)
        if(((i > GPIO1) && (i < GPIO6))
            || ((i > GPIO11) && (i < GPIO14))
            || ((i > GPIO17) && (i < GPIO20))
            || ((i > GPIO24) && (i < GPIO26))
            || ((i > GPIO26) && (i < GPIO28)))
        {
            continue;
        }
#endif
        if(((i < BITS_INT)&&(deep_param->gpio_stay_lo_map & (0x01UL << i)))
        ||((deep_param->gpio_index_map & (0x01UL << i)))
        ||((deep_param->gpio_last_index_map & (0x01UL << i)))
        ||((i >= BITS_INT)&&(deep_param->gpio_stay_hi_map & (0x01UL << (i - BITS_INT)))) )
        {
            continue;
        }
        param = GPIO_CFG_PARAM(i, GMODE_DEEP_PS);	/*set gpio 0~39 as high impendance*/
        sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
    }

    if (((deep_param->wake_up_way & PS_DEEP_WAKEUP_RTC))
        && (deep_param->sleep_time!= 0xffffffff))
    {
    /*ROSC_TIMER  init*/
#if (CFG_SOC_NAME != SOC_BK7231)
        reg = (deep_param->sleep_time >> 16)& 0xffff;                                          //'A'
        REG_WRITE(SCTRL_ROSC_TIMER_H,reg);
#endif

        reg = REG_READ(SCTRL_ROSC_TIMER);
        reg |= ROSC_TIMER_INT_STATUS_BIT;                                                                   //'C'
        REG_WRITE(SCTRL_ROSC_TIMER,reg);  //sys_ctrl : 0x47;

        reg = REG_READ(SCTRL_ROSC_TIMER);
        reg &= ~(ROSC_TIMER_PERIOD_MASK << ROSC_TIMER_PERIOD_POSI);
        reg |= ((deep_param->sleep_time & ROSC_TIMER_PERIOD_MASK) << ROSC_TIMER_PERIOD_POSI);
        REG_WRITE(SCTRL_ROSC_TIMER,reg);   //sys_ctrl : 0x47;                         //'D'

        reg = REG_READ(SCTRL_ROSC_TIMER);
        reg |= ROSC_TIMER_ENABLE_BIT;
        REG_WRITE(SCTRL_ROSC_TIMER,reg);  //sys_ctrl : 0x47;                             //'B'

        if(deep_param->lpo_32k_src == LPO_SELECT_32K_XTAL)
        {
            REG_WRITE(SCTRL_CONTROL, 0x330100);
            REG_WRITE(SCTRL_BLOCK_EN_CFG, (0x15D|(0xA5C<<20)));
            REG_WRITE(SCTRL_ROSC_CAL, 0x30);
            REG_WRITE(SCTRL_LOW_PWR_CLK, 0x01);
            REG_WRITE(SCTRL_MODEM_CORE_RESET_PHY_HCLK, 0x03);
            REG_WRITE(SCTRL_CLK_GATING, 0x1ff);
        }
    }

#if ((CFG_SOC_NAME != SOC_BK7231N) && (CFG_SOC_NAME != SOC_BK7236))
    if ((deep_param->wake_up_way & PS_DEEP_WAKEUP_GPIO))
    {
        for (i = 0; i < BITS_INT; i++)
        {
#if(BITS_INT > GPIONUM)
            if(i >= GPIONUM)
            {
                break;
            }
#endif
            if (deep_param->gpio_index_map & (0x01UL << i))			/*set gpio 0~31 mode*/
            {
                if( deep_param->gpio_edge_map & (0x01UL << i))      //0:high,1:low.
                {
                    if (deep_param->gpio_stay_lo_map & (0x01UL << i)) {
                        param = GPIO_CFG_PARAM(i, GMODE_INPUT);
                    } else {
                        param = GPIO_CFG_PARAM(i, GMODE_INPUT_PULLUP);
                    }
                    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
                    if(0x1 != (UINT32)gpio_ctrl( CMD_GPIO_INPUT, &i))
                    {   /*check gpio really input value,to correct wrong edge setting*/
                        param = GPIO_CFG_PARAM(i, GMODE_INPUT);
                        sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
                        deep_param->gpio_edge_map &= ~(0x01UL << i);
                    }
                }
                else
                {
                    if (deep_param->gpio_stay_lo_map & (0x01UL << i)) {
                        param = GPIO_CFG_PARAM(i, GMODE_INPUT);
                    } else {
                        param = GPIO_CFG_PARAM(i, GMODE_INPUT_PULLDOWN);
                    }
                    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
                    if(0x0 != (UINT32)gpio_ctrl( CMD_GPIO_INPUT, &i))
                    {   /*check gpio really input value,to correct wrong edge setting*/
                        param = GPIO_CFG_PARAM(i, GMODE_INPUT);
                        sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
                        deep_param->gpio_edge_map |= (0x01UL << i);
                    }
                }
            }
        }

        for (i = 0; i < (GPIONUM - BITS_INT); i++)
        {
            if (deep_param->gpio_last_index_map & (0x01UL << i))				/*set gpio 32~39 mode*/
            {
                if( deep_param->gpio_last_edge_map  & (0x01UL << i))
                {
                    if (deep_param->gpio_stay_hi_map & (0x01UL << (i - BITS_INT))) {
                        param = GPIO_CFG_PARAM(i + BITS_INT, GMODE_INPUT);
                    } else {
                        param = GPIO_CFG_PARAM(i + BITS_INT, GMODE_INPUT_PULLUP);
                    }
                    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
                    reg = i + BITS_INT;
                    if(0x1 != (UINT32)gpio_ctrl( CMD_GPIO_INPUT, &reg))
                    {   /*check gpio really input value,to correct wrong edge setting*/
                        param = GPIO_CFG_PARAM(i + BITS_INT, GMODE_INPUT);
                        sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
                        deep_param->gpio_last_edge_map &= ~(0x01UL << i);
                    }
                }
                else
                {
                    if (deep_param->gpio_stay_hi_map & (0x01UL << (i - BITS_INT))) {
                        param = GPIO_CFG_PARAM(i + BITS_INT, GMODE_INPUT);
                    } else {
                        param = GPIO_CFG_PARAM(i + BITS_INT, GMODE_INPUT_PULLDOWN);
                    }
                    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
                    reg = i + BITS_INT;
                    if(0x0 != (UINT32)gpio_ctrl( CMD_GPIO_INPUT, &reg))
                    {   /*check gpio really input value,to correct wrong edge setting*/
                        param = GPIO_CFG_PARAM(i + BITS_INT, GMODE_INPUT);
                        sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
                        deep_param->gpio_last_edge_map |= (0x01UL << i);
                    }
                }
            }
        }

        /* set gpio 0~31 mode*/
        reg = 0xFFFFFFFF;
        REG_WRITE(SCTRL_GPIO_WAKEUP_INT_STATUS,reg);
        reg = deep_param->gpio_edge_map;
        REG_WRITE(SCTRL_GPIO_WAKEUP_TYPE,reg);
        reg = deep_param->gpio_index_map;
        REG_WRITE(SCTRL_GPIO_WAKEUP_EN,reg);

        /* set gpio 31~32 mode*/
        reg = 0xFF;
        REG_WRITE(SCTRL_GPIO_WAKEUP_INT_STATUS1,reg);

        reg = deep_param->gpio_last_edge_map;
        REG_WRITE(SCTRL_GPIO_WAKEUP_TYPE1,reg);

        reg = deep_param->gpio_last_index_map;
        REG_WRITE(SCTRL_GPIO_WAKEUP_EN1,reg);
    }
#elif ((CFG_SOC_NAME == SOC_BK7231N) || (CFG_SOC_NAME == SOC_BK7236))
    if(( deep_param->wake_up_way & PS_DEEP_WAKEUP_GPIO ))
    {
        for ( i = 0; i < BITS_INT; i++ )
        {
#if(BITS_INT > GPIONUM)
            if( i >= GPIONUM )
            {
                break;
            }
#endif
            if((( i > GPIO1 ) && ( i < GPIO6 ))
            || (( i > GPIO11 ) && ( i < GPIO14 ))
            || (( i > GPIO17 ) && ( i < GPIO20 ))
            || (( i > GPIO24 ) && ( i < GPIO26 ))
            || (( i > GPIO26 ) && ( i < GPIO28 )))
            {
                continue;
            }

            if ( deep_param->gpio_index_map & ( 0x01UL << i ))
            {
                int type_h,type_l;
                type_l = deep_param->gpio_edge_map;
                type_h = 0x0;

                /* low level or negedge wakeup */
                if(( type_h & ( 0x01UL << i )) == ( type_l & ( 0x01UL << i )))
                {
                    if (deep_param->gpio_stay_lo_map & (0x01UL << i)) {
                        param = GPIO_CFG_PARAM(i, GMODE_INPUT);
                    } else {
                        param = GPIO_CFG_PARAM(i, GMODE_INPUT_PULLUP);
                    }
                    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
                }
                else    /* high level or posedge wakeup */
                {
                    if (deep_param->gpio_stay_lo_map & (0x01UL << i)) {
                        param = GPIO_CFG_PARAM(i, GMODE_INPUT);
                    } else {
                        param = GPIO_CFG_PARAM(i, GMODE_INPUT_PULLDOWN);
                    }
                    sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);
                }

                REG_WRITE(SCTRL_GPIO_WAKEUP_TYPE, type_l);
                REG_WRITE(SCTRL_GPIO_WAKEUP_TYPE_SELECT, type_h);
            }
        }

        reg = deep_param->gpio_index_map;
        REG_WRITE(SCTRL_GPIO_WAKEUP_EN,reg);
    }
#endif

#if (CFG_SOC_NAME != SOC_BK7231N)
    REG_WRITE(SCTRL_USB_PLUG_WAKEUP,USB_PLUG_IN_INT_BIT|USB_PLUG_OUT_INT_BIT);
    if(deep_param->wake_up_way & PS_DEEP_WAKEUP_USB)
    {
        REG_WRITE(SCTRL_USB_PLUG_WAKEUP,USB_PLUG_IN_EN_BIT|USB_PLUG_OUT_EN_BIT);
    }
#endif

#ifdef BK_DEEP_SLEEP_DEBUG
    BK_DEEP_SLEEP_PRT("SCTRL_CONTROL=0x%08X\r\n", REG_READ(SCTRL_CONTROL));
    BK_DEEP_SLEEP_PRT("SCTRL_SLEEP=0x%08X\r\n", REG_READ(SCTRL_SLEEP));
    BK_DEEP_SLEEP_PRT("SCTRL_ROSC_TIMER=0x%08X\r\n", REG_READ(SCTRL_ROSC_TIMER));
    BK_DEEP_SLEEP_PRT("SCTRL_BLOCK_EN_CFG=0x%08X\r\n", REG_READ(SCTRL_BLOCK_EN_CFG));
    BK_DEEP_SLEEP_PRT("SCTRL_ROSC_CAL=0x%08X\r\n", REG_READ(SCTRL_ROSC_CAL));
    BK_DEEP_SLEEP_PRT("SCTRL_BLOCK_EN_MUX=0x%08X\r\n", REG_READ(SCTRL_BLOCK_EN_MUX));
    BK_DEEP_SLEEP_PRT("SCTRL_LOW_PWR_CLK=0x%08X\r\n", REG_READ(SCTRL_LOW_PWR_CLK));
    BK_DEEP_SLEEP_PRT("SCTRL_PWR_MAC_MODEM=0x%08X\r\n", REG_READ(SCTRL_PWR_MAC_MODEM));
    BK_DEEP_SLEEP_PRT("SCTRL_MODEM_CORE_RESET_PHY_HCLK=0x%08X\r\n", REG_READ(SCTRL_MODEM_CORE_RESET_PHY_HCLK));
    BK_DEEP_SLEEP_PRT("SCTRL_CLK_GATING=0x%08X\r\n", REG_READ(SCTRL_CLK_GATING));
    BK_DEEP_SLEEP_PRT("SCTRL_GPIO_WAKEUP_INT_STATUS=0x%08X\r\n", REG_READ(SCTRL_GPIO_WAKEUP_INT_STATUS));
    BK_DEEP_SLEEP_PRT("SCTRL_GPIO_WAKEUP_TYPE=0x%08X\r\n", REG_READ(SCTRL_GPIO_WAKEUP_TYPE));
    BK_DEEP_SLEEP_PRT("SCTRL_GPIO_WAKEUP_EN=0x%08X\r\n", REG_READ(SCTRL_GPIO_WAKEUP_EN));
    BK_DEEP_SLEEP_PRT("SCTRL_GPIO_WAKEUP_INT_STATUS1=0x%08X\r\n", REG_READ(SCTRL_GPIO_WAKEUP_INT_STATUS1));
    BK_DEEP_SLEEP_PRT("SCTRL_GPIO_WAKEUP_TYPE1=0x%08X\r\n", REG_READ(SCTRL_GPIO_WAKEUP_TYPE1));
    BK_DEEP_SLEEP_PRT("SCTRL_GPIO_WAKEUP_EN1=0x%08X\r\n", REG_READ(SCTRL_GPIO_WAKEUP_EN1));
#endif

    /* enter deep_sleep mode */
    reg = REG_READ(SCTRL_SLEEP);
    reg &= ~(SLEEP_MODE_MASK << SLEEP_MODE_POSI);
    reg = reg | SLEEP_MODE_CFG_DEEP_WORD;
    REG_WRITE(SCTRL_SLEEP, reg);

    delay(5);
}
