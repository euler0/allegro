/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *      By Shawn Hargreaves,
 *      1 Salisbury Road,
 *      Market Drayton,
 *      Shropshire,
 *      England, TF9 1AJ.
 *
 *      Fixed point math routines and lookup tables.
 *
 *      See readme.txt for copyright information.
 */


#include <errno.h>
#include <math.h>

#include "allegro.h"



fixed _cos_tbl[512] =
{
   /* precalculated fixed point (16.16) cosines for a full circle (0-255) */

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
   /* precalculated fixed point (16.16) tangents for a half circle (0-127) */

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



fixed _acos_tbl[513] =
{
   /* precalculated fixed point (16.16) inverse cosines (-1 to 1) */

   0x800000L,  0x7C65C7L,  0x7AE75AL,  0x79C19EL,  0x78C9BEL,  0x77EF25L,  0x772953L,  0x76733AL,
   0x75C991L,  0x752A10L,  0x74930CL,  0x740345L,  0x7379C1L,  0x72F5BAL,  0x72768FL,  0x71FBBCL,
   0x7184D3L,  0x711174L,  0x70A152L,  0x703426L,  0x6FC9B5L,  0x6F61C9L,  0x6EFC36L,  0x6E98D1L,
   0x6E3777L,  0x6DD805L,  0x6D7A5EL,  0x6D1E68L,  0x6CC40BL,  0x6C6B2FL,  0x6C13C1L,  0x6BBDAFL,
   0x6B68E6L,  0x6B1558L,  0x6AC2F5L,  0x6A71B1L,  0x6A217EL,  0x69D251L,  0x698420L,  0x6936DFL,
   0x68EA85L,  0x689F0AL,  0x685465L,  0x680A8DL,  0x67C17DL,  0x67792CL,  0x673194L,  0x66EAAFL,
   0x66A476L,  0x665EE5L,  0x6619F5L,  0x65D5A2L,  0x6591E7L,  0x654EBFL,  0x650C26L,  0x64CA18L,
   0x648890L,  0x64478CL,  0x640706L,  0x63C6FCL,  0x63876BL,  0x63484FL,  0x6309A5L,  0x62CB6AL,
   0x628D9CL,  0x625037L,  0x621339L,  0x61D69FL,  0x619A68L,  0x615E90L,  0x612316L,  0x60E7F7L,
   0x60AD31L,  0x6072C3L,  0x6038A9L,  0x5FFEE3L,  0x5FC56EL,  0x5F8C49L,  0x5F5372L,  0x5F1AE7L,
   0x5EE2A7L,  0x5EAAB0L,  0x5E7301L,  0x5E3B98L,  0x5E0473L,  0x5DCD92L,  0x5D96F3L,  0x5D6095L,
   0x5D2A76L,  0x5CF496L,  0x5CBEF2L,  0x5C898BL,  0x5C545EL,  0x5C1F6BL,  0x5BEAB0L,  0x5BB62DL,
   0x5B81E1L,  0x5B4DCAL,  0x5B19E7L,  0x5AE638L,  0x5AB2BCL,  0x5A7F72L,  0x5A4C59L,  0x5A1970L,
   0x59E6B6L,  0x59B42AL,  0x5981CCL,  0x594F9BL,  0x591D96L,  0x58EBBDL,  0x58BA0EL,  0x588889L,
   0x58572DL,  0x5825FAL,  0x57F4EEL,  0x57C40AL,  0x57934DL,  0x5762B5L,  0x573243L,  0x5701F5L,
   0x56D1CCL,  0x56A1C6L,  0x5671E4L,  0x564224L,  0x561285L,  0x55E309L,  0x55B3ADL,  0x558471L,
   0x555555L,  0x552659L,  0x54F77BL,  0x54C8BCL,  0x549A1BL,  0x546B98L,  0x543D31L,  0x540EE7L,
   0x53E0B9L,  0x53B2A7L,  0x5384B0L,  0x5356D4L,  0x532912L,  0x52FB6BL,  0x52CDDDL,  0x52A068L,
   0x52730CL,  0x5245C9L,  0x52189EL,  0x51EB8BL,  0x51BE8FL,  0x5191AAL,  0x5164DCL,  0x513825L,
   0x510B83L,  0x50DEF7L,  0x50B280L,  0x50861FL,  0x5059D2L,  0x502D99L,  0x500175L,  0x4FD564L,
   0x4FA967L,  0x4F7D7DL,  0x4F51A6L,  0x4F25E2L,  0x4EFA30L,  0x4ECE90L,  0x4EA301L,  0x4E7784L,
   0x4E4C19L,  0x4E20BEL,  0x4DF574L,  0x4DCA3AL,  0x4D9F10L,  0x4D73F6L,  0x4D48ECL,  0x4D1DF1L,
   0x4CF305L,  0x4CC829L,  0x4C9D5AL,  0x4C729AL,  0x4C47E9L,  0x4C1D45L,  0x4BF2AEL,  0x4BC826L,
   0x4B9DAAL,  0x4B733BL,  0x4B48D9L,  0x4B1E84L,  0x4AF43BL,  0x4AC9FEL,  0x4A9FCDL,  0x4A75A7L,
   0x4A4B8DL,  0x4A217EL,  0x49F77AL,  0x49CD81L,  0x49A393L,  0x4979AFL,  0x494FD5L,  0x492605L,
   0x48FC3FL,  0x48D282L,  0x48A8CFL,  0x487F25L,  0x485584L,  0x482BECL,  0x48025DL,  0x47D8D6L,
   0x47AF57L,  0x4785E0L,  0x475C72L,  0x47330AL,  0x4709ABL,  0x46E052L,  0x46B701L,  0x468DB7L,
   0x466474L,  0x463B37L,  0x461201L,  0x45E8D0L,  0x45BFA6L,  0x459682L,  0x456D64L,  0x45444BL,
   0x451B37L,  0x44F229L,  0x44C920L,  0x44A01CL,  0x44771CL,  0x444E21L,  0x44252AL,  0x43FC38L,
   0x43D349L,  0x43AA5FL,  0x438178L,  0x435894L,  0x432FB4L,  0x4306D8L,  0x42DDFEL,  0x42B527L,
   0x428C53L,  0x426381L,  0x423AB2L,  0x4211E5L,  0x41E91AL,  0x41C051L,  0x41978AL,  0x416EC5L,
   0x414601L,  0x411D3EL,  0x40F47CL,  0x40CBBBL,  0x40A2FBL,  0x407A3CL,  0x40517DL,  0x4028BEL,
   0x400000L,  0x3FD742L,  0x3FAE83L,  0x3F85C4L,  0x3F5D05L,  0x3F3445L,  0x3F0B84L,  0x3EE2C2L,
   0x3EB9FFL,  0x3E913BL,  0x3E6876L,  0x3E3FAFL,  0x3E16E6L,  0x3DEE1BL,  0x3DC54EL,  0x3D9C7FL,
   0x3D73ADL,  0x3D4AD9L,  0x3D2202L,  0x3CF928L,  0x3CD04CL,  0x3CA76CL,  0x3C7E88L,  0x3C55A1L,
   0x3C2CB7L,  0x3C03C8L,  0x3BDAD6L,  0x3BB1DFL,  0x3B88E4L,  0x3B5FE4L,  0x3B36E0L,  0x3B0DD7L,
   0x3AE4C9L,  0x3ABBB5L,  0x3A929CL,  0x3A697EL,  0x3A405AL,  0x3A1730L,  0x39EDFFL,  0x39C4C9L,
   0x399B8CL,  0x397249L,  0x3948FFL,  0x391FAEL,  0x38F655L,  0x38CCF6L,  0x38A38EL,  0x387A20L,
   0x3850A9L,  0x38272AL,  0x37FDA3L,  0x37D414L,  0x37AA7CL,  0x3780DBL,  0x375731L,  0x372D7EL,
   0x3703C1L,  0x36D9FBL,  0x36B02BL,  0x368651L,  0x365C6DL,  0x36327FL,  0x360886L,  0x35DE82L,
   0x35B473L,  0x358A59L,  0x356033L,  0x353602L,  0x350BC5L,  0x34E17CL,  0x34B727L,  0x348CC5L,
   0x346256L,  0x3437DAL,  0x340D52L,  0x33E2BBL,  0x33B817L,  0x338D66L,  0x3362A6L,  0x3337D7L,
   0x330CFBL,  0x32E20FL,  0x32B714L,  0x328C0AL,  0x3260F0L,  0x3235C6L,  0x320A8CL,  0x31DF42L,
   0x31B3E7L,  0x31887CL,  0x315CFFL,  0x313170L,  0x3105D0L,  0x30DA1EL,  0x30AE5AL,  0x308283L,
   0x305699L,  0x302A9CL,  0x2FFE8BL,  0x2FD267L,  0x2FA62EL,  0x2F79E1L,  0x2F4D80L,  0x2F2109L,
   0x2EF47DL,  0x2EC7DBL,  0x2E9B24L,  0x2E6E56L,  0x2E4171L,  0x2E1475L,  0x2DE762L,  0x2DBA37L,
   0x2D8CF4L,  0x2D5F98L,  0x2D3223L,  0x2D0495L,  0x2CD6EEL,  0x2CA92CL,  0x2C7B50L,  0x2C4D59L,
   0x2C1F47L,  0x2BF119L,  0x2BC2CFL,  0x2B9468L,  0x2B65E5L,  0x2B3744L,  0x2B0885L,  0x2AD9A7L,
   0x2AAAABL,  0x2A7B8FL,  0x2A4C53L,  0x2A1CF7L,  0x29ED7BL,  0x29BDDCL,  0x298E1CL,  0x295E3AL,
   0x292E34L,  0x28FE0BL,  0x28CDBDL,  0x289D4BL,  0x286CB3L,  0x283BF6L,  0x280B12L,  0x27DA06L,
   0x27A8D3L,  0x277777L,  0x2745F2L,  0x271443L,  0x26E26AL,  0x26B065L,  0x267E34L,  0x264BD6L,
   0x26194AL,  0x25E690L,  0x25B3A7L,  0x25808EL,  0x254D44L,  0x2519C8L,  0x24E619L,  0x24B236L,
   0x247E1FL,  0x2449D3L,  0x241550L,  0x23E095L,  0x23ABA2L,  0x237675L,  0x23410EL,  0x230B6AL,
   0x22D58AL,  0x229F6BL,  0x22690DL,  0x22326EL,  0x21FB8DL,  0x21C468L,  0x218CFFL,  0x215550L,
   0x211D59L,  0x20E519L,  0x20AC8EL,  0x2073B7L,  0x203A92L,  0x20011DL,  0x1FC757L,  0x1F8D3DL,
   0x1F52CFL,  0x1F1809L,  0x1EDCEAL,  0x1EA170L,  0x1E6598L,  0x1E2961L,  0x1DECC7L,  0x1DAFC9L,
   0x1D7264L,  0x1D3496L,  0x1CF65BL,  0x1CB7B1L,  0x1C7895L,  0x1C3904L,  0x1BF8FAL,  0x1BB874L,
   0x1B7770L,  0x1B35E8L,  0x1AF3DAL,  0x1AB141L,  0x1A6E19L,  0x1A2A5EL,  0x19E60BL,  0x19A11BL,
   0x195B8AL,  0x191551L,  0x18CE6CL,  0x1886D4L,  0x183E83L,  0x17F573L,  0x17AB9BL,  0x1760F6L,
   0x17157BL,  0x16C921L,  0x167BE0L,  0x162DAFL,  0x15DE82L,  0x158E4FL,  0x153D0BL,  0x14EAA8L,
   0x14971AL,  0x144251L,  0x13EC3FL,  0x1394D1L,  0x133BF5L,  0x12E198L,  0x1285A2L,  0x1227FBL,
   0x11C889L,  0x11672FL,  0x1103CAL,  0x109E37L,  0x10364BL,  0xFCBDAL,   0xF5EAEL,   0xEEE8CL,
   0xE7B2DL,   0xE0444L,   0xD8971L,   0xD0A46L,   0xC863FL,   0xBFCBBL,   0xB6CF4L,   0xAD5F0L,
   0xA366FL,   0x98CC6L,   0x8D6ADL,   0x810DBL,   0x73642L,   0x63E62L,   0x518A6L,   0x39A39L,
   0x0L
};



static short sqrt_table[256] =
{
   /* this table is used by the fsqrt() function */

   0x2D4,   0x103F,  0x16CD,  0x1BDB,  0x201F,  0x23E3,  0x274B,  0x2A6D, 
   0x2D57,  0x3015,  0x32AC,  0x3524,  0x377F,  0x39C2,  0x3BEE,  0x3E08, 
   0x400F,  0x4207,  0x43F0,  0x45CC,  0x479C,  0x4960,  0x4B19,  0x4CC9, 
   0x4E6F,  0x500C,  0x51A2,  0x532F,  0x54B6,  0x5635,  0x57AE,  0x5921, 
   0x5A8D,  0x5BF4,  0x5D56,  0x5EB3,  0x600A,  0x615D,  0x62AB,  0x63F5, 
   0x653B,  0x667D,  0x67BA,  0x68F5,  0x6A2B,  0x6B5E,  0x6C8D,  0x6DBA, 
   0x6EE3,  0x7009,  0x712C,  0x724C,  0x7369,  0x7484,  0x759C,  0x76B1, 
   0x77C4,  0x78D4,  0x79E2,  0x7AEE,  0x7BF7,  0x7CFE,  0x7E04,  0x7F07, 
   0x8007,  0x8106,  0x8203,  0x82FF,  0x83F8,  0x84EF,  0x85E5,  0x86D9, 
   0x87CB,  0x88BB,  0x89AA,  0x8A97,  0x8B83,  0x8C6D,  0x8D56,  0x8E3D, 
   0x8F22,  0x9007,  0x90E9,  0x91CB,  0x92AB,  0x938A,  0x9467,  0x9543, 
   0x961E,  0x96F8,  0x97D0,  0x98A8,  0x997E,  0x9A53,  0x9B26,  0x9BF9, 
   0x9CCA,  0x9D9B,  0x9E6A,  0x9F39,  0xA006,  0xA0D2,  0xA19D,  0xA268, 
   0xA331,  0xA3F9,  0xA4C1,  0xA587,  0xA64D,  0xA711,  0xA7D5,  0xA898, 
   0xA95A,  0xAA1B,  0xAADB,  0xAB9A,  0xAC59,  0xAD16,  0xADD3,  0xAE8F, 
   0xAF4B,  0xB005,  0xB0BF,  0xB178,  0xB230,  0xB2E8,  0xB39F,  0xB455, 
   0xB50A,  0xB5BF,  0xB673,  0xB726,  0xB7D9,  0xB88A,  0xB93C,  0xB9EC, 
   0xBA9C,  0xBB4B,  0xBBFA,  0xBCA8,  0xBD55,  0xBE02,  0xBEAE,  0xBF5A, 
   0xC005,  0xC0AF,  0xC159,  0xC202,  0xC2AB,  0xC353,  0xC3FA,  0xC4A1, 
   0xC548,  0xC5ED,  0xC693,  0xC737,  0xC7DC,  0xC87F,  0xC923,  0xC9C5, 
   0xCA67,  0xCB09,  0xCBAA,  0xCC4B,  0xCCEB,  0xCD8B,  0xCE2A,  0xCEC8, 
   0xCF67,  0xD004,  0xD0A2,  0xD13F,  0xD1DB,  0xD277,  0xD312,  0xD3AD, 
   0xD448,  0xD4E2,  0xD57C,  0xD615,  0xD6AE,  0xD746,  0xD7DE,  0xD876, 
   0xD90D,  0xD9A4,  0xDA3A,  0xDAD0,  0xDB66,  0xDBFB,  0xDC90,  0xDD24, 
   0xDDB8,  0xDE4C,  0xDEDF,  0xDF72,  0xE004,  0xE096,  0xE128,  0xE1B9, 
   0xE24A,  0xE2DB,  0xE36B,  0xE3FB,  0xE48B,  0xE51A,  0xE5A9,  0xE637, 
   0xE6C5,  0xE753,  0xE7E1,  0xE86E,  0xE8FB,  0xE987,  0xEA13,  0xEA9F, 
   0xEB2B,  0xEBB6,  0xEC41,  0xECCB,  0xED55,  0xEDDF,  0xEE69,  0xEEF2, 
   0xEF7B,  0xF004,  0xF08C,  0xF114,  0xF19C,  0xF223,  0xF2AB,  0xF332, 
   0xF3B8,  0xF43E,  0xF4C4,  0xF54A,  0xF5D0,  0xF655,  0xF6DA,  0xF75E, 
   0xF7E3,  0xF867,  0xF8EA,  0xF96E,  0xF9F1,  0xFA74,  0xFAF7,  0xFB79, 
   0xFBFB,  0xFC7D,  0xFCFF,  0xFD80,  0xFE02,  0xFE82,  0xFF03,  0xFF83
};



/* fatan:
 *  Fixed point inverse tangent. Does a binary search on the tan table.
 */
fixed fatan(fixed x)
{
   int a, b, c;            /* for binary search */
   fixed d;                /* difference value for search */

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



/* fatan2:
 *  Like the libc atan2, but for fixed point numbers.
 */
fixed fatan2(fixed y, fixed x)
{
   fixed r;

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

   if (errno) {
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



/* fsqrt:
 *  Fixed point square root routine. This code is taken from the fixfloat
 *  library by Arne Steinarson.
 */
fixed fsqrt(fixed x)
{
   fixed result;

   if (x <= 0) {
      if (x < 0)
	 errno = EDOM;

      return 0;
   }

   asm (
      "  movl %1, %0 ; "

      " fsqrt_not_zero: "
      "  xorl %%ecx, %%ecx ; "
      "  cmpl $0x010000, %0 ; "        /* series of shifts to reduce range */
      "  jb fsqrt_0 ; "
      "  shrl $16, %0 ; "
      "  addb $16, %%cl ; "

      " fsqrt_0: "
      "  cmpl $0x0100, %0 ; "
      "  jb fsqrt_1 ; "
      "  shrl $8, %0 ; "
      "  addb $8, %%cl ; "

      " fsqrt_1: "
      "  cmpl $0x010, %0 ; "
      "  jb fsqrt_2 ; "
      "  shrl $4, %0 ; "
      "  addb $4, %%cl ; "

      " fsqrt_2: "
      "  cmpl $0x04, %0 ; "
      "  jb fsqrt_3 ; "
      "  shrl $2, %0 ; "
      "  addb $2, %%cl ; "

      " fsqrt_3: "
      "  cmpl $2, %0 ; "
      "  jb fsqrt_4 ; "
      "  incl %%ecx ; "

      " fsqrt_4: "
      "  movl %1, %0 ; "
      "  subb $15, %%cl ; "
      "  testb $1, %%cl ; "
      "  je fsqrt_cl_even ; "
      "  incb %%cl ; "

      " fsqrt_cl_even: "
      "  movb %%cl, %%dl ; "
      "  addb $4, %%cl ; "
      "  jns fsqrt_cl_pve ; "

      "  negb %%cl ; "
      "  shll %%cl, %0 ; "
      "  jmp fsqrt_cl_nve_done ; "

      " fsqrt_cl_pve: "
      "  shrl %%cl, %0 ; "

      " fsqrt_cl_nve_done: "
      "  sarb $1, %%dl ; "             /* table lookup */
      "  shrl $4, %0 ; "
      "  movw _sqrt_table(, %0, 2), %w0 ; "
      "  movb %%dl, %%cl ; "
      "  testb %%cl, %%cl ; "
      "  js fsqrt_cl_nve ; "

      "  shll %%cl, %0 ; "             /* shift back into proper range */
      "  jmp fsqrt_done ; "

      " fsqrt_cl_nve: "
      "  negb %%cl ; "
      "  shrl %%cl, %0 ; "

      " fsqrt_done: "

   : "=&a" (result)                    /* result in eax */
   : "rm" (x)                          /* parameter in reg or memory */
   : "%ecx", "%edx"                    /* clobbers ecx and edx */
   );

   return result;
}



