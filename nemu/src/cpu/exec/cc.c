#include "cpu/rtl.h"

/* Condition Code */

void rtl_setcc(rtlreg_t* dest, uint8_t subcode) {
  bool invert = subcode & 0x1;
  //由于opcode的规律,invert变量处理最后一位,决定是否取相反的情况
  enum {
    CC_O, CC_NO, CC_B,  CC_NB,
    CC_E, CC_NE, CC_BE, CC_NBE,
    CC_S, CC_NS, CC_P,  CC_NP,
    CC_L, CC_NL, CC_LE, CC_NLE
  };

  // TODO: Query EFLAGS to determine whether the condition code is satisfied.
  // dest <- ( cc is satisfied ? 1 : 0)
  switch (subcode & 0xe) {
    case CC_O:
		//Set byte if overflow (OF=1)
		rtl_get_OF(dest);
		break;
    case CC_B:
		//Set byte if below (CF=1)
		rtl_get_CF(dest);
		break;
    case CC_E:
		//Set byte if equal (ZF=1)
		rtl_get_ZF(dest);
		break;
    case CC_BE:
		//Set byte if below or equal (CF=1 or (ZF=1)
		rtl_get_CF(&t1);
		rtl_get_ZF(&t2);
		rtl_or(dest,&t1,&t2);
		break;
    case CC_S:
		//Set byte if sign (SF=1)
		rtl_get_SF(dest);
		break;
    case CC_L:
		//Set byte if less (SF≠OF)
		rtl_get_SF(&t0);
		rtl_get_OF(&t1);
		*dest = (t0!=t1);
		/* rtl_get_ZF(&t0);
		rtl_get_SF(&t1);
		rtl_get_OF(&t2);
		t1=(t1!=t2?1:0);
		if(invert)
		{
			t0=(t0!=1?1:0);
			rtl_and(dest,&t0,&t1);
		}
		else
		{
			*dest=t1;
		}*/
		break;
    case CC_LE:
      //TODO();
	  //Set byte if less or equal (ZF=1 and SF≠OF)
		rtl_get_ZF(&t0);
		rtl_get_SF(&t1);
		rtl_get_OF(&t2);
		*dest = (t0 == 1) || (t1!=t2);
		break;
	case CC_P: panic("n86 does not have PF");
    default: panic("should not reach here"); 
  }

  if (invert) {
    rtl_xori(dest, dest, 0x1);
  }
}
