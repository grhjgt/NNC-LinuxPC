#include "ENS1_BOOST.h"
#include "ENS1_CLOCK.h"



/*--------------------内部BOOST_SET-----------------------*/
uint8_t Boost_Voltage_Sel(uint8_t VOLTAGE_XV)
{   
	PCLK_Enable(ANALOG_PCLK_EN);
	switch (VOLTAGE_XV){
		case VOLTAGE_11V :
			CMSDK_ANAC->BOOST_CTRL =0x71013;//内部boost	
			CMSDK_ANAC->PMU_CTRL = 0x10;
			break;
		
		case VOLTAGE_15V :
			CMSDK_ANAC->BOOST_CTRL =0x71113;//内部boost	
			CMSDK_ANAC->PMU_CTRL = 0x10;
			break;
		case VOLTAGE_26V :
			CMSDK_ANAC->BOOST_CTRL =0x71213;//内部boost	
			CMSDK_ANAC->PMU_CTRL = 0x10;
			break;
		case VOLTAGE_45V :
			CMSDK_ANAC->BOOST_CTRL =0xc1413;//
			CMSDK_ANAC->PMU_CTRL = 0x10;
			break;
		case VOLTAGE_55V :
			CMSDK_ANAC->BOOST_CTRL =0xc1713;//内部boost	
			CMSDK_ANAC->PMU_CTRL = 0x10;
			break;
		default:
			CMSDK_ANAC->BOOST_CTRL =0x71013;//内部boost	
			CMSDK_ANAC->PMU_CTRL = 0x10;
			break;
	}
	return 0;
}
