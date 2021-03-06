/*---------------------------------------------------------------------
	�� �㭪樨, ࠡ���騥 � ����ࠬ�, �।��������, �� ���
	�।�⠢���� �� ᥡ� ���ᨢ� �� 3-�� ������⮢ ⨯� double.
--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------
	VEC_MUL
	���⨯:
		double* vec_mul( double* a, double* b, double* c )
	�㭪�� �ந������ ����୮� 㬭������ ����� a �� ����� b,
	������ १���� � c, ����� ����� � ���� ᮢ������.
	��ࠬ����:
	a	���� ᮬ����⥫�.
	b	�ࠢ� ᮬ����⥫�.
	c	�����⥫� �� �����, �㤠 �㤥� ����饭 १����.
--------------------------------------------------------------------------*/
extern	double* vec_mul( double*, double*, double* );

/*--------------------------------------------------------------------------
	SCAL_MUL
	���⨯:
		double scal_mul( double* a, double* b )
	�㭪�� �ந������ ᪠��୮� 㬭������ ���� ����஢.
	��ࠬ����:
	a, b	�����-ᮬ����⥫�.
--------------------------------------------------------------------------*/
extern	double scal_mul( double* , double* );

/*--------------------------------------------------------------------------
	VEC_LEN
	���⨯:
		double vec_len( double *a )
	�㭪�� ������ ����� �����.
--------------------------------------------------------------------------*/
extern	double vec_len( double* );

/*--------------------------------------------------------------------------
	VEC_NORM
	���⨯:
		void vec_norm( double* a )
	�㭪�� ��ନ��� �����.
--------------------------------------------------------------------------*/
extern	void vec_norm( double* );

/*--------------------------------------------------------------------------
	VEC_COPY
	���⨯:
		void vec_copy( double *dst, double *src )
	�㭪�� ������� ����� src � ����� dst.
--------------------------------------------------------------------------*/
extern	void vec_copy( double*, double* );

/*--------------------------------------------------------------------------
	VEC_ADD
	���⨯:
		double* vec_add( double* a, double* b, double* r )
	�㭪�� �㬬���� ����� a � b, ������ १���� � r.
--------------------------------------------------------------------------*/
extern	double* vec_add( double*, double*, double* );

/*--------------------------------------------------------------------------
	VEC_SUB
	���⨯:
		double* vec_sub( double* a, double* b, double* r )
	�㭪�� ���⠥� ����� a � b, ������ १���� � r.
--------------------------------------------------------------------------*/
extern	double* vec_sub( double*, double*, double* );

/*--------------------------------------------------------------------------
	VEC_NUM_MUL
	���⨯:
		double* vec_num_mul( double* a, double num, double* r )
	�㭪�� 㬭����� ����� a �� �᫮ num, ������ १���� � r.
--------------------------------------------------------------------------*/
extern	double* vec_num_mul( double*, double, double* );
