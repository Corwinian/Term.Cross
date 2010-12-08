#include <c_lib/c_types.hpp>

static int steps[2] = {1, -3};
/*
 * Самый хитрый метод. Распаковывает 8 10 битовых значений упакованных в 10 байт.
 * Creation date: (20.11.00 16:32:37)
 * TODO реализовать unpack810 для big endian
 *
 * @param src byte[]
 * @param dst short[]
 * @nWords  Сколько слов распаковывать. Должно быть кратно 8.
 */
void unpack810( void* in, void* out, uint32_t nWords ) {
	uint8_t* src = (uint8_t*)in;
	uint16_t* dst = (uint16_t*)out;
	int t = 0;
	int pSrc = 0;
	int pDst = 0;
	for (uint32_t dstptr = 0, srcptr = 0; dstptr < nWords; dstptr += 8, srcptr += 10) {
		pSrc = srcptr + 8;
		pDst = dstptr + 7;
		for (int i = 10, shift = 0; i > 0; i--, pSrc += steps[pSrc % 2], shift -= 2) {
			if (shift > 0) {
				t |= (0xff & src[pSrc]) << shift;
				dst[pDst] = (uint16_t) (0x3ff & t);
				t >>= 10;
				pDst--;
			} else {
				t = 0xff & src[pSrc];
				shift = 10;
			}
		}
	}
}
