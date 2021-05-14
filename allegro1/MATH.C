/*
		  //  /     /     ,----  ,----.  ,----.  ,----.
		/ /  /     /     /      /    /  /    /  /    /
	      /  /  /     /     /___   /       /____/  /    /
	    /---/  /     /     /      /  __   /\      /    /
	  /    /  /     /     /      /    /  /  \    /    /
	/     /  /____ /____ /____  /____/  /    \  /____/

	Low Level Game Routines (version 1.0)

	Fixed point (16.16) math routines.

	See allegro.txt for instructions and copyright conditions.
   
	By Shawn Hargreaves,
	1 Salisbury Road,
	Market Drayton,
	Shropshire,
	England TF9 1AJ
	email slh100@tower.york.ac.uk (until 1996)
*/


#include "errno.h"
#include "allegro.h"


#ifdef BORLAND    /* djgpp 386 versions are in misc.s */


fixed fmul(x, y)
fixed x;
fixed y;
{
   short neg = 0;
   fixed res;

   if (x < 0) {
      x = -x;
      neg = 1;
   }

   if (y < 0) {
      y = -y;
      neg = 1 - neg;
   }

#define xl      word ptr x
#define xh      word ptr x + 2
#define yl      word ptr y
#define yh      word ptr y + 2

   asm {
      mov bx, xl
      mov cx, xh
      mov ax, yl
      mul bx            // dx.ax = xl * yl
      mov si, dx        // res.low word
      mov di, 0         // res.high word

      mov ax, yh
      mul bx            // dx.ax = xl * yh
      add si, ax
      adc di, dx

      mov ax, yl
      mul cx            // dx.ax = xh * yl
      add si, ax
      adc di, dx

      mov ax, yh
      mul cx            // dx.ax = xh * yh
      jc overflow
      add di, ax
      js overflow

      mov word ptr res, si
      mov word ptr res + 2, di
      jmp done
   }

   overflow:
   errno = ERANGE;
   res = 0x7fffffffL;

   done:

   if (neg)
      res = -res;

   return res;
}



fixed fdiv(x, y)
fixed x;
fixed y;
{
   fixed res = 0;
   short neg = 0;

   if (x < 0) {
      x = -x;
      neg = 1;
   }

   if (y < 0) {
      y = -y;
      neg = 1 - neg;
   }

   x <<= 1;

   asm {
      mov ax, word ptr x + 2
      mov bx, 0                  // ax.bx = temp = x >> 16

      mov si, word ptr y
      mov di, word ptr y + 2     // si.di = y

      mov cx, 31                 // counter

   div_loop:
      sub ax, si
      sbb bx, di                 // temp -= y
      jl result_neg              // if temp >= 0, set bit cx of res

      mov dx, 1
      sub cx, 16                 // which word of res are we in?
      jl bit_in_low_word
      
      shl dx, cl                 // if we are in the high word,
      or word ptr res + 2, dx    // set the bit
      add cx, 16                 // put cx back
      jmp result_set
      
   bit_in_low_word:
      add cx, 16                 // put cx back
      shl dx, cl
      or word ptr res, dx        // set the bit
      jmp result_set

   result_neg:
      add ax, si
      adc bx, di                 // restore temp
      
   result_set:
      shl word ptr x, 1          // shift x, only need low word
      rcl ax, 1                  // move a bit from x into temp
      rcl bx, 1 

      dec cx
      jge div_loop
   }

   if (res < 0) {
      errno = ERANGE;
      res = 0x7fffffffL;
   }

   if (neg)
      res = -res;

   return res;
}



void install_divzero() {};    /* fillers */
void remove_divzero() {};



#endif   /* ifdef BORLAND */


#ifdef GCC     /* fmul() and fdiv() are in misc.s */

#include "go32.h"
#include "dpmi.h"

#define DIV_ZERO     0

_go32_dpmi_seginfo _divzero_oldint;   /* original prot-mode /0 IRQ */
_go32_dpmi_seginfo _divzero_int;      /* prot-mode interrupt segment info */


int _fdiv_flag = 0;


void _my_divzero(_go32_dpmi_registers *regs)
{
   if (_fdiv_flag) {          /* we can trap this division by zero */
      errno = ERANGE;         /* make fdiv() produce reasonable results */
      regs->d.eax = 0x7fffffff;
      regs->d.ebx = 1;
      regs->d.edx = 0;
   }
   else
      remove_divzero();       /* oops, not one of ours */
}



void install_divzero()
{
   _divzero_int.pm_offset = (int)_my_divzero;   /* div by zero handler */
   _go32_dpmi_allocate_iret_wrapper(&_divzero_int);
   _divzero_int.pm_selector = _go32_my_cs();
   _go32_dpmi_get_protected_mode_interrupt_vector(DIV_ZERO, &_divzero_oldint);
   _go32_dpmi_set_protected_mode_interrupt_vector(DIV_ZERO, &_divzero_int);
}



void remove_divzero()
{
   _go32_dpmi_set_protected_mode_interrupt_vector(DIV_ZERO, &_divzero_oldint);
   _go32_dpmi_free_iret_wrapper(&_divzero_int);
}



#endif      /* ifdef GCC */



fixed _cos_tbl[512] =
{
   /* precalculated fixed point (16.16) cosines for a full circle (0-255),
      at intervals of 0.5. */

   65536L,  65531L,  65516L,  65492L,  65457L,  65413L,  65358L,  65294L, 
   65220L,  65137L,  65043L,  64940L,  64827L,  64704L,  64571L,  64429L, 
   64277L,  64115L,  63944L,  63763L,  63572L,  63372L,  63162L,  62943L, 
   62714L,  62476L,  62228L,  61971L,  61705L,  61429L,  61145L,  60851L, 
   60547L,  60235L,  59914L,  59583L,  59244L,  58896L,  58538L,  58172L, 
   57798L,  57414L,  57022L,  56621L,  56212L,  55794L,  55368L,  54934L, 
   54491L,  54040L,  53581L,  53114L,  52639L,  52156L,  51665L,  51166L, 
   50660L,  50146L,  49624L,  49095L,  48559L,  48015L,  47464L,  46906L, 
   46341L,  45769L,  45190L,  44604L,  44011L,  43412L,  42806L,  42194L, 
   41576L,  40951L,  40320L,  39683L,  39040L,  38391L,  37736L,  37076L, 
   36410L,  35738L,  35062L,  34380L,  33692L,  33000L,  32303L,  31600L, 
   30893L,  30182L,  29466L,  28745L,  28020L,  27291L,  26558L,  25821L, 
   25080L,  24335L,  23586L,  22834L,  22078L,  21320L,  20557L,  19792L, 
   19024L,  18253L,  17479L,  16703L,  15924L,  15143L,  14359L,  13573L, 
   12785L,  11996L,  11204L,  10411L,  9616L,   8820L,   8022L,   7224L, 
   6424L,   5623L,   4821L,   4019L,   3216L,   2412L,   1608L,   804L, 
   0L,      -803L,   -1607L,  -2411L,  -3215L,  -4018L,  -4820L,  -5622L, 
   -6423L,  -7223L,  -8021L,  -8819L,  -9615L,  -10410L, -11203L, -11995L, 
   -12784L, -13572L, -14358L, -15142L, -15923L, -16702L, -17478L, -18252L, 
   -19023L, -19791L, -20556L, -21319L, -22077L, -22833L, -23585L, -24334L, 
   -25079L, -25820L, -26557L, -27290L, -28019L, -28744L, -29465L, -30181L, 
   -30892L, -31599L, -32302L, -32999L, -33691L, -34379L, -35061L, -35737L, 
   -36409L, -37075L, -37735L, -38390L, -39039L, -39682L, -40319L, -40950L, 
   -41575L, -42193L, -42805L, -43411L, -44010L, -44603L, -45189L, -45768L, 
   -46340L, -46905L, -47463L, -48014L, -48558L, -49094L, -49623L, -50145L, 
   -50659L, -51165L, -51664L, -52155L, -52638L, -53113L, -53580L, -54039L, 
   -54490L, -54933L, -55367L, -55793L, -56211L, -56620L, -57021L, -57413L, 
   -57797L, -58171L, -58537L, -58895L, -59243L, -59582L, -59913L, -60234L, 
   -60546L, -60850L, -61144L, -61428L, -61704L, -61970L, -62227L, -62475L, 
   -62713L, -62942L, -63161L, -63371L, -63571L, -63762L, -63943L, -64114L, 
   -64276L, -64428L, -64570L, -64703L, -64826L, -64939L, -65042L, -65136L, 
   -65219L, -65293L, -65357L, -65412L, -65456L, -65491L, -65515L, -65530L, 
   -65536L, -65530L, -65515L, -65491L, -65456L, -65412L, -65357L, -65293L, 
   -65219L, -65136L, -65042L, -64939L, -64826L, -64703L, -64570L, -64428L, 
   -64276L, -64114L, -63943L, -63762L, -63571L, -63371L, -63161L, -62942L, 
   -62713L, -62475L, -62227L, -61970L, -61704L, -61428L, -61144L, -60850L, 
   -60546L, -60234L, -59913L, -59582L, -59243L, -58895L, -58537L, -58171L, 
   -57797L, -57413L, -57021L, -56620L, -56211L, -55793L, -55367L, -54933L, 
   -54490L, -54039L, -53580L, -53113L, -52638L, -52155L, -51664L, -51165L, 
   -50659L, -50145L, -49623L, -49094L, -48558L, -48014L, -47463L, -46905L, 
   -46340L, -45768L, -45189L, -44603L, -44010L, -43411L, -42805L, -42193L, 
   -41575L, -40950L, -40319L, -39682L, -39039L, -38390L, -37735L, -37075L, 
   -36409L, -35737L, -35061L, -34379L, -33691L, -32999L, -32302L, -31599L, 
   -30892L, -30181L, -29465L, -28744L, -28019L, -27290L, -26557L, -25820L, 
   -25079L, -24334L, -23585L, -22833L, -22077L, -21319L, -20556L, -19791L, 
   -19023L, -18252L, -17478L, -16702L, -15923L, -15142L, -14358L, -13572L, 
   -12784L, -11995L, -11203L, -10410L, -9615L,  -8819L,  -8021L,  -7223L, 
   -6423L,  -5622L,  -4820L,  -4018L,  -3215L,  -2411L,  -1607L,  -803L, 
   0L,      804L,    1608L,   2412L,   3216L,   4019L,   4821L,   5623L, 
   6424L,   7224L,   8022L,   8820L,   9616L,   10411L,  11204L,  11996L, 
   12785L,  13573L,  14359L,  15143L,  15924L,  16703L,  17479L,  18253L, 
   19024L,  19792L,  20557L,  21320L,  22078L,  22834L,  23586L,  24335L, 
   25080L,  25821L,  26558L,  27291L,  28020L,  28745L,  29466L,  30182L, 
   30893L,  31600L,  32303L,  33000L,  33692L,  34380L,  35062L,  35738L, 
   36410L,  37076L,  37736L,  38391L,  39040L,  39683L,  40320L,  40951L, 
   41576L,  42194L,  42806L,  43412L,  44011L,  44604L,  45190L,  45769L, 
   46341L,  46906L,  47464L,  48015L,  48559L,  49095L,  49624L,  50146L, 
   50660L,  51166L,  51665L,  52156L,  52639L,  53114L,  53581L,  54040L, 
   54491L,  54934L,  55368L,  55794L,  56212L,  56621L,  57022L,  57414L, 
   57798L,  58172L,  58538L,  58896L,  59244L,  59583L,  59914L,  60235L, 
   60547L,  60851L,  61145L,  61429L,  61705L,  61971L,  62228L,  62476L, 
   62714L,  62943L,  63162L,  63372L,  63572L,  63763L,  63944L,  64115L, 
   64277L,  64429L,  64571L,  64704L,  64827L,  64940L,  65043L,  65137L, 
   65220L,  65294L,  65358L,  65413L,  65457L,  65492L,  65516L,  65531L
};



fixed _tan_tbl[256] =
{
   /* precalculated fixed point (16.16) tangents for a half circle (0-127),
      at intervals of 0.5. */

   0L,      804L,    1609L,   2414L,   3220L,   4026L,   4834L,   5644L, 
   6455L,   7268L,   8083L,   8901L,   9721L,   10545L,  11372L,  12202L, 
   13036L,  13874L,  14717L,  15564L,  16416L,  17273L,  18136L,  19005L, 
   19880L,  20762L,  21650L,  22546L,  23449L,  24360L,  25280L,  26208L, 
   27146L,  28093L,  29050L,  30018L,  30996L,  31986L,  32988L,  34002L, 
   35030L,  36071L,  37126L,  38196L,  39281L,  40382L,  41500L,  42636L, 
   43790L,  44963L,  46156L,  47369L,  48605L,  49863L,  51145L,  52451L, 
   53784L,  55144L,  56532L,  57950L,  59398L,  60880L,  62395L,  63947L, 
   65536L,  67165L,  68835L,  70548L,  72308L,  74116L,  75974L,  77887L, 
   79856L,  81885L,  83977L,  86135L,  88365L,  90670L,  93054L,  95523L, 
   98082L,  100736L, 103493L, 106358L, 109340L, 112447L, 115687L, 119071L, 
   122609L, 126314L, 130198L, 134276L, 138564L, 143081L, 147847L, 152884L, 
   158218L, 163878L, 169896L, 176309L, 183161L, 190499L, 198380L, 206870L, 
   216043L, 225990L, 236817L, 248648L, 261634L, 275959L, 291845L, 309568L, 
   329472L, 351993L, 377693L, 407305L, 441808L, 482534L, 531352L, 590958L, 
   665398L, 761030L, 888450L, 1066730L, 1334016L, 1779314L, 2669641L, 5340086L, 
   -2147483647L, -5340085L, -2669640L, -1779313L, -1334015L, -1066729L, -888449L, -761029L, 
   -665397L, -590957L, -531351L, -482533L, -441807L, -407304L, -377692L, -351992L, 
   -329471L, -309567L, -291844L, -275958L, -261633L, -248647L, -236816L, -225989L, 
   -216042L, -206869L, -198379L, -190498L, -183160L, -176308L, -169895L, -163877L, 
   -158217L, -152883L, -147846L, -143080L, -138563L, -134275L, -130197L, -126313L, 
   -122608L, -119070L, -115686L, -112446L, -109339L, -106357L, -103492L, -100735L, 
   -98081L, -95522L, -93053L, -90669L, -88364L, -86134L, -83976L, -81884L, 
   -79855L, -77886L, -75973L, -74115L, -72307L, -70547L, -68834L, -67164L, 
   -65535L, -63946L, -62394L, -60879L, -59397L, -57949L, -56531L, -55143L, 
   -53783L, -52450L, -51144L, -49862L, -48604L, -47368L, -46155L, -44962L, 
   -43789L, -42635L, -41499L, -40381L, -39280L, -38195L, -37125L, -36070L, 
   -35029L, -34001L, -32987L, -31985L, -30995L, -30017L, -29049L, -28092L, 
   -27145L, -26207L, -25279L, -24359L, -23448L, -22545L, -21649L, -20761L, 
   -19879L, -19004L, -18135L, -17272L, -16415L, -15563L, -14716L, -13873L, 
   -13035L, -12201L, -11371L, -10544L, -9720L,  -8900L,  -8082L,  -7267L, 
   -6454L,  -5643L,  -4833L,  -4025L,  -3219L,  -2413L,  -1608L,  -803L
};



fixed facos(x)
register fixed x;
{
   register short a, b, c;   /* for binary search */
   register fixed d;       /* difference value for search */

   if ((x < -65536L) || (x > 65536L)) {
      errno = EDOM;
      return 0L;
   }

   a = 0;
   b = 256;
			   /* binary search on the cos table */
   do {
      c = (a + b) >> 1;
      d = x - _cos_tbl[c];

      if (d < 0)
	 a = c + 1;
      else
	 if (d > 0)
	    b = c - 1;

   } while ((a <= b) && (d));

   return (((long)c) << 15);
}



fixed fatan(x)
register fixed x;
{
   register short a, b, c;   /* for binary search */
   register fixed d;       /* difference value for search */

   if (x >= 0) {           /* search the first part of tan table */
      a = 0;
      b = 127;
   }
   else {                  /* search the second half instead */
      a = 128;
      b = 255;
   } 
   
   do {
      c = (a + b) >> 1;
      d = x - _tan_tbl[c];

      if (d > 0)
	 a = c + 1;
      else
	 if (d < 0)
	    b = c - 1;

   } while ((a <= b) && (d));

   if (x >= 0)
      return ((long)c) << 15;
      
   return (-0x00800000L + (((long)c) << 15));
}



fixed fatan2(y,x)
register fixed y;
register fixed x;
{
   register fixed r;

   if (x==0) {
      if (y==0) {
	 errno = EDOM;
	 return 0L;
      }
      else
	 return ((y < 0) ? -0x00400000L : 0x00400000L);
   } 

   errno = 0;
   r = fdiv(y,x);

   if (errno == ERANGE) {
      errno = 0;
      return ((y < 0) ? -0x00400000L : 0x00400000L);
   }

   r = fatan(r);

   if (x >= 0)
      return r;
   
   if (y >= 0)
      return 0x00800000L + r;

   return r - 0x00800000L;
}



#define SQRT_TBL_SIZE           182

fixed _sqrt_tbl[SQRT_TBL_SIZE] =
{
   /* a bunch of precalculated values for the square root routine */

  0L,          65536L,      262144L,     589824L,     1048576L,    1638400L, 
  2359296L,    3211264L,    4194304L,    5308416L,    6553600L,    7929856L, 
  9437184L,    11075584L,   12845056L,   14745600L,   16777216L,   18939904L, 
  21233664L,   23658496L,   26214400L,   28901376L,   31719424L,   34668544L, 
  37748736L,   40960000L,   44302336L,   47775744L,   51380224L,   55115776L, 
  58982400L,   62980096L,   67108864L,   71368704L,   75759616L,   80281600L, 
  84934656L,   89718784L,   94633984L,   99680256L,   104857600L,  110166016L, 
  115605504L,  121176064L,  126877696L,  132710400L,  138674176L,  144769024L, 
  150994944L,  157351936L,  163840000L,  170459136L,  177209344L,  184090624L, 
  191102976L,  198246400L,  205520896L,  212926464L,  220463104L,  228130816L, 
  235929600L,  243859456L,  251920384L,  260112384L,  268435456L,  276889600L, 
  285474816L,  294191104L,  303038464L,  312016896L,  321126400L,  330366976L, 
  339738624L,  349241344L,  358875136L,  368640000L,  378535936L,  388562944L, 
  398721024L,  409010176L,  419430400L,  429981696L,  440664064L,  451477504L, 
  462422016L,  473497600L,  484704256L,  496041984L,  507510784L,  519110656L, 
  530841600L,  542703616L,  554696704L,  566820864L,  579076096L,  591462400L, 
  603979776L,  616628224L,  629407744L,  642318336L,  655360000L,  668532736L, 
  681836544L,  695271424L,  708837376L,  722534400L,  736362496L,  750321664L, 
  764411904L,  778633216L,  792985600L,  807469056L,  822083584L,  836829184L, 
  851705856L,  866713600L,  881852416L,  897122304L,  912523264L,  928055296L, 
  943718400L,  959512576L,  975437824L,  991494144L,  1007681536L, 1024000000L, 
  1040449536L, 1057030144L, 1073741824L, 1090584576L, 1107558400L, 1124663296L, 
  1141899264L, 1159266304L, 1176764416L, 1194393600L, 1212153856L, 1230045184L, 
  1248067584L, 1266221056L, 1284505600L, 1302921216L, 1321467904L, 1340145664L, 
  1358954496L, 1377894400L, 1396965376L, 1416167424L, 1435500544L, 1454964736L, 
  1474560000L, 1494286336L, 1514143744L, 1534132224L, 1554251776L, 1574502400L, 
  1594884096L, 1615396864L, 1636040704L, 1656815616L, 1677721600L, 1698758656L, 
  1719926784L, 1741225984L, 1762656256L, 1784217600L, 1805910016L, 1827733504L, 
  1849688064L, 1871773696L, 1893990400L, 1916338176L, 1938817024L, 1961426944L, 
  1984167936L, 2007040000L, 2030043136L, 2053177344L, 2076442624L, 2099838976L, 
  2123366400L, 2147024896L
};



fixed fsqrt(x)
fixed x;
{
   register fixed y = x;
   register short a = 0;
   register short b = SQRT_TBL_SIZE - 1;
   register long c;
   register long d;

   if (y <= 0) {
      if (y < 0)
	 errno = EDOM;
      return 0;
   }
   
   do {                         /* binary search on the sqrt table */
      c = (a + b) >> 1;
      d = y - _sqrt_tbl[(short)c];

      if (d > 0)
	 a = (short)c + 1;
      else
	 if (d < 0)
	    b = (short)c - 1;

   } while ((a <= b) && (d));

   if (d < 0)
      if (c > 0)
	 c--;

   y -= _sqrt_tbl[(short)c];
   c <<= 9;
   
   while (y >= 0) {             /* work out the exact value */
      y -= c;
      c += 2;
   }

   y = c >> 1;

   if ((y * (y-1) + 1) > x)
      y--;

   return y << 8;
}

