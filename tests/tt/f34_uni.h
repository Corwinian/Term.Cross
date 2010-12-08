/*---------------------------------------------------------------------
	������� �ଠ� 0-����� ��室��� ������ HRPT/CHRPT/GMS/MTSAT.
	Date: 13 july 2005
---------------------------------------------------------------------*/
#pragma pack(1)

struct TBlk0_uni {
/*---------------------------------------------------------------------
	����ﭭ�� ���� �ଠ�
---------------------------------------------------------------------*/
	unsigned char formatType;	/* ⨯ �ଠ� (FFh)		0 */
	char satName[13];		/* ASCII ��� ���		1 */
	unsigned long satId;		/* ������ ���			14 */
	unsigned long revNum;		/* ����� ��⪠ (HRPT)
					��� GMS �� �ᯮ������ (0)	18 */

	/* �६� �ਥ�� UTC ( �� ��ࢮ�� ���� 䠩�� ) */

	unsigned short s_year;		/* ��� ( ����� )		22 */
	unsigned short s_day;		/* ���� ���� (1-based)		24 */
	unsigned long s_time;		/* �६� ( �ᥪ �� ��砫� ��� )	26 */

	/* �६� �ਥ�� UTC ( �� ���譥�� ���筨�� ) */
	/* ��� GMS ���� �� �ᯮ������ */

	unsigned short o_year;		/* ��� ( ����� )		30 */
	unsigned short o_day;		/* ���� ���� (1-based)		32 */
	unsigned long o_time;		/* �६� ( �ᥪ �� ��砫� ��� )	34 */

   	unsigned char reserved1[23];	/* ==== १�� ====		37 */

	unsigned char sourceType;	/* ���筨� ������, �.�. ������
								�ਥ���� �⠭樨:
		0	���� �ਥ���� �⠭��	( ��� ᮢ���⨬��� );
		1	�⠭�� �ਥ�� Time Step;
		2	�⠭�� �ਥ�� UKW Technik ( ����୮-�ࡨ⠫쭠� );
		*/
    unsigned char dataType1;	/* 62   ��� ������, ���� 1 */
    unsigned char dataType2;	/* 63   ��� ������, ���� 2 */
		/* ��� ������ ����� ᫥���騥 ���祭��:
		dataType1:				dataType2:
		1	��室�� �����		1	HRPT NOAA
		2	����������� �����	2	HRPT SeaStar
						3	CHRPT FENGYUN
						4...15	१��
						16	GMS LRFAX
						17	GMS S-VISSR
						18	MTSAT HiRIT
						19...	१��
		3	�஥�樨		��� 0 - ⨯
							0 - ��ઠ��᪠�
							1 - ࠢ���஬����筠�
						��� 1 - �����஢��
							0 - �� �����஢���
		4...	१��
		*/

/*---------------------------------------------------------------------
	��६����� ���� �ଠ�
---------------------------------------------------------------------*/
	unsigned short frame_c;		/* ������⢮ ���஢ ��� ᡮ�
					ᨭ�஭���樨, � ⮬ �᫥
					���஢, ����� ���������
					㤠����	����⠭�����		64 */
	unsigned short frame_lost;	/* ������⢮ ����ﭭ��
					���஢,	� ⮬ �᫥ ���஢ �
					ᡮ�� ᨭ�஭���樨, �����
					�� 㤠���� ����⠭�����		66 */
	unsigned short frame_uni1;	/* ��� HRPT - ������⢮ ���஢ ���
					ᡮ�� �� ���� �६��� � ����
					��� GMS -  ������⢮ ���஢, �����
					��������� 㤠���� ����⠭�����	68 */
	unsigned short frame_uni2;	/* ��� HRPT - ������⢮ ���஢ �
					ᡮ�� �� ���� �६��� � ���� �
					�⪮�४�஢����� 
					��� GMS - ������⢮ ���஢ � ᡮ��,
					� ������ ����ﭠ ����
					ᥪ�஢			70 */
	unsigned short frame_gaps;	/* ������⢮ �ய�᪮� (gaps)	72 */

/* �ਬ�砭��:
	��� HRPT
	1. �㬬� frame_c ( 64 ) + frame_lost ( 66 ) ���� ��饥 ������⢮
		���஢ � 䠩�� hrpt ������ �ଠ�;
	2. �㬬� frame_uni1 ( 68 ) + frame_uni2 ( 70 ) ࠢ��
		�������� ���஢ ��� ᡮ� ᨭ�஭���樨 frame_c ( 64 );
	3. ���� frame_gaps ( 72 ) ���� ������⢮ ���, �� ����� ��।�����
		������⢮ ���஢ � ᡮ�� ᨭ�஭���樨 frame_lost ( 66 );
	4. ���� frame_uni1 ( 68 ), frame_uni2 ( 70 ) �
		frame_lost ( 66 ) ��⠢���� ������ � ⠪�� ���浪�
		���ᨢ �� 3 楫�� �ᥫ, ������砥�� � �ଠ� 1b
		��� ����⢮ ������ ( DACS Quality );
	5. ��� ���४⭮�� �८�ࠧ������ �ଠ⮢ �� ��ண� � ���� �
		���⭮ ��������� ���� ��������� � ���� �ଠ�:
		����				����
		totalScans ( numberOfFrames )	frame_c + frame_lost
		frame_lost ( 476 )		frame_lost ( 66 )
		frame_corrected ( 478 )		frame_uni2 ( 70 )
		frame_gaps ( 480 )		frame_gaps ( 72 )
	6. ���� totalScans � numberOfFrames ��� ��娢��� 䠩��� ��ண�
		�ଠ�, ��� �ࠢ���, ���������. ��⠫�� ���� ᮤ�ঠ� 0.
		���� frame_corrected 䨪������ �ணࠬ��� CHECK_HT.EXE.
		��� �� ���४���� ���� totalScans � numberOfFrames,
		�᫨ �����㦥�� ᡮ� �� �६��� � ���� 䠩��. �ணࠬ��
		��⠭�������� ���� new_flag � �� ࠡ�⠥�, �᫨ ��� ࠢ�� 1.
		�ணࠬ�� H_CONC.EXE ᪫������ ࠧ�ࢠ��� � १���� ����
		ᨭ�஭���樨 䠩�� � ���� � ᮮ⢥�����騬 ��ࠧ��
		���४���� ���� totalScans, frame_lost � frame_gaps.
		�ணࠬ�� OLD2NEW.EXE ��९��뢠�� � ���� �ଠ� ⥪�騥
		���祭�� ����� frame_lost, frame_corrected � frame_gaps,
		� ���祭�� ��⠢���� ���� ����� ������ ���
			frame_c = totalScans - frame_lost;
			frame_uni1 = frame_c - frame_uni2;
		�� ���⭮� �८�ࠧ������ NEW2OLD.EXE ��९��뢠�� �
		���� �ଠ� ⥪�騥 ���祭�� ����� frame_lost,
		frame_uni2 � frame_gaps, � ��⠢訥�� ����
		������ ���
			totalScans = numberOfFrames = frame_c + frame_lost;
		�஬� ⮣�, ��� ��⠭�������� ���� new_flag ��� �����
		�� ����୮� ���४�஢�� �६���.
	��� GMS
	1. �㬬� frame_c ( 64 ) + frame_lost ( 66 ) + frame_uni2 ( 70 )
		���� ��饥 ������⢮ ���஢ � 䠩�� gms ������� �ଠ�;
	2. �㬬� frame_lost ( 66 ) + frame_uni2 ( 70 ) ࠢ�� ��饬�
		�������� ᡮ���� ���஢;
	3. ���� frame_gaps ( 72 ) ���� ������⢮ ���, �� ����� ��।�����
		��饥 ������⢮ ᡮ���� ���஢ frame_lost ( 66 ) +
		frame_uni2 ( 70 );
	4. ���� �� ����⭮ ��� ������஢��� �� ����稭� � �ଠ� 1b,
		��� ����⢮ ������ ( DACS Quality );
*/

/*---------------------------------------------------------------------
	���ᠭ�� ����
---------------------------------------------------------------------*/
	unsigned short pack_type;	/* ⨯ 㯠�����			74 */
		/*
			0	������⢨� 㯠�����	(GMS,HRPT)
			1	���㯫�⭠�		(HRPT)
			2	���⭠�			(HRPT)
		*/
	unsigned short frame_length;	/* ��� GMS - ����� ��ப� IR
					��� HRPT - ����� ��ப� AVHRR	76 */
	unsigned long frame_mask;	/* ��᪠ ᥣ���⮢ ����
					��� HRPT �� ��� �������� ॠ���
					ࠧ��� ���� HRPT
					��� GMS ����� ���ଠ樮��� ��� 78 */
	unsigned short pix_gap;		/* ������⢮ �ய�饭��� ���ᥫ��
					�� ��砫� ��ப� (IR,AVHRR)	82 */
	unsigned short pix_len;		/* ������⢮ �ਭ���� ���ᥫ��
					��ப� (IR,AVHRR)		84 */
	unsigned short uni3;		/* ��� HRPT ��� ��⪠:
						1 - ���室�騩,
						0 - ���室�騩
					��� GMS - ����� ���� GMS	86 */

	/* ᫥���騥 ���� �ᯮ������� ⮫쪮 ��� GMS */
	unsigned short vis_length;	/* ����� ��ப� VIS		88 */
	unsigned short vis_pix_gap;	/* ������⢮ �ய�饭��� ���ᥫ��
					�� ��砫� ��ப� VIS		90 */
	unsigned short vis_pix_len;	/* ������⢮ �ਭ���� ���ᥫ��
					��ப� VIS			92 */
	unsigned short first_frame_number;	/* ����� ��ࢮ� ��ப� VISSR � 䠩��
									( �� ��ࢮ� �� ��ப� ! )		94 */
	unsigned char unused2[32];	/* ==== १�� ====		96 */

/*---------------------------------------------------------------------
	���� �ࡨ⠫��� ������ NORAD
---------------------------------------------------------------------*/
	unsigned long ref_num;	/* ����� ��⪠				128 */
	unsigned short set_num;	/* ����� ����� ����⮢		132 */
	unsigned short ephem;	/* ⨯ �䥬�ਤ				134 */
	unsigned short ep_year;	/* ��� ( ����� )			136 */
	double ep_day;		/* ���� ���� ( 1-based )		138 */
	double n0;		/* �।��� �������� ( ࠤ/��� )		146 */
	double bstar;		/* BSTAR Drag term			154 */
	double i0;		/* ���������� �ࡨ�� ( ࠤ )		162 */
	double raan;		/* ��אַ� ���宦����� ���室�饣� 㧫� ( ࠤ ) 170 */
	double e0;		/* ��業����� �ࡨ��		178 */
	double w0;		/* ��㬥�� ��ਣ�� ( ࠤ )		186 */
	double m0;		/* �।��� �������� ( ࠤ )		194 */
	unsigned char unused3[54];	/* ==== १�� ====		202 */

/*---------------------------------------------------------------------
	���� ���४樨
	�ᯮ������ ���� ⮫쪮 ��� HRPT
---------------------------------------------------------------------*/
	unsigned short c_ver;	/* ����� ���ᨨ ���४樨		256 */
	short s_delt;		/* ���ࠢ�� ���⮢�� �ᮢ (�ᥪ) -
					- �� TBUS			258 */
	short delt;		/* ���ࠢ�� �६��� ( �ᥪ )		260 */
				/* ���४�� �ਥ��樨 ᪠��� (ࠤ) */
	double roll;		/* �७					262 */
	double pitch;		/* ⠭���				270 */
	double yaw;		/* ��᪠��				278 */

	unsigned char unused4[226];	/* ==== १�� ====		286 */
};

#pragma pack()
