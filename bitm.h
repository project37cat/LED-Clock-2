// bitm.h


#ifndef BITM_H_
#define BITM_H_

/////////////////////////////////////////////////

#ifndef SET_BIT
#define SET_BIT(reg, bit) (reg |= (1<<bit))
#endif

#ifndef CLR_BIT
#define CLR_BIT(reg, bit) (reg &= (~(1<<bit)))
#endif

#ifndef BIT_IS_SET
#define BIT_IS_SET(reg, bit) ((reg>>bit)&1)
#endif

#ifndef BIT_IS_CLR
#define BIT_IS_CLR(reg, bit) (!((reg>>bit)&1))
#endif

/////////////////////////////////////////////////

#endif

