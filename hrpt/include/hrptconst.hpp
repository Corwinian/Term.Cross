
#ifndef _HRPTCONST_H_
#define _HRPTCONST_H_

/*  константы кадра HRPT */
#define MIDN    1       /* маска для приема IDN */
#define MTCD    2       /* --#-- TCD */
#define MTLM    4       /* --#-- TLM */
#define MBSC    8       /* --#-- BSC */
#define MSPC    16      /* --#-- SPC */
#define MHSY    32      /* --#-- HSY */
#define MTIP    64      /* --#-- TIP */
#define MSPR    128     /* --#-- SPR */
#define MAVH    256     /* --#-- AVHRR */
#define MASY    512     /* --#-- ASY */
#define MFSY    1024    /* --#-- FSY */
#define M5      2048    /* --#-- 5 канала AVHRR */
#define M4      4096    /* --#-- 4 --#-- */
#define M3      8192    /* --#-- 3 --#-- */
#define M2      16384   /* --#-- 2 --#-- */
#define M1      32768   /* --#-- 1 --#-- */

#define SSHRPT  5       /* адрес для кода синхросерии HRPT */
#define SSTIP1  105     /* адрес для кода 1-ой синхросерии TIP */
#define SSTIP2  209     /* --#-- 2-ой --#-- */
#define SSTIP3  313     /* --#-- 3-ей --#-- */
#define SSTIP4  417     /* --#-- 4-ой --#-- */
#define SSTIP5  521     /* --#-- 5-ой --#-- */

#define FSY		0	/* адрес начала синхросерии HRPT */
#define LFSY	6	/* длина --#-- */
#define IDN		6	/* адрес начала идентификатора IDN */
#define LIDN	2	/* длина --#-- */
#define TCD		8	/* адрес TCD */
#define LTCD	4	/* длина TCD */
#define TLM		12	/* адрес TLM */
#define LTLM	10	/* длина TLM */
#define BSC		22	/* адрес BSC */
#define LBSC	30	/* длина BSC */
#define SPC		52	/* адрес SPC */
#define LSPC	50	/* длина SPC */
#define HSY		102	/* адрес HSY */
#define LHSY	1	/* длина HSY */
#define TIP		103	/* адрес TIP */
#define LTIP	520	/* длина TIP */
#define LMTIP	101	/* длина малого кадра TIP без синхросерий ( - 3 ) */
#define LTNO	15	/* 15 слов синхросерий не пишутся */
#define SPR		623	/* адрес SPR */
#define LSPR	127	/* длина SPR */
#define AVH		750	/* адрес AVHRR */
#define LAVH	10240	/* длина AVHRR */
#define ASY		10990	/* адрес ASY */
#define LASY	100	/* длина ASY */
#define LHRPT	11090	/* длина маски (малого кадра HRPT) */
#define LAVHRR	2048	/* длина строки канала AVHRR в элементах */
#define LSTLM	30	/* длина строки файла телеметрии */
#define NTIP	5	/* число полей TIP в кадре HRPT */

/* эти переменные добавлены 27.06.95 для организации пакетного режима */

#define M_HIRS	1024
#define M_MSU	512
#define M_SSU	256
#define M_TIP	128

/* эти определения из функции bld_mask */
#define LSTR	LAVHRR
#define WRMAJF	0	/* запись по главному кадру */
#define WRALLF	2	/* запись по всем кадрам */
#define NOWR	1	/* не писать */
#define SYNHRPT	6	/* код синхросерии HRPT */
#define SYNTIP	4+1	/* код синхросерии TIP-a всегда не писать */

#endif
