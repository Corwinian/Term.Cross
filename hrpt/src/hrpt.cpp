/*-------------------------------------------------------------------------
    hrpt.cpp
-------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include <c_lib.hpp>
#include <hrpt.hpp>
/*  константы кадра HRPT */
//#include <hrptconst.hpp>

unsigned hrptFrameLengths[8] = { LIDN, LTCD, LTLM, LBSC, LSPC, LHSY, LTIP, LSPR };

THRPTUnpacker::THRPTUnpacker( const char * fname, int memory_flag ) throw ( TAccessExc ):
		fBuf( 0 ),
		fCurFrameNum( 0 ),
		fExtBufFlag( 0 )
{

	char buf[BLK0_SZ];

	if( (fFile = fopen( fname, "rb" )) == NULL )
		goto error;
	if( fread( buf, 1, BLK0_SZ, fFile ) != BLK0_SZ )
		goto error;

	init( buf );

	if( memory_flag ) {  // читать файл заранее
		fBuf = new char [BLK0_SZ + fPackedFrameLength * fTotalFrames * 2];
		if( fread( fBuf + BLK0_SZ, 2, fPackedFrameLength * fTotalFrames, fFile ) != fPackedFrameLength * fTotalFrames )
			goto error;
		fclose( fFile );
		fFile = NULL;       // указываем, что файл закрыт
	} else {   // читать файл при распаковке
		fBuf = new char [BLK0_SZ + fPackedFrameLength * 2];
	}

	memcpy( fBuf, buf, BLK0_SZ );   // копирование 0-блока из временного буфера в постоянный
	fBlk0 = (TBlk0 *)fBuf;
	fData = (uint16_t *)(fBuf + BLK0_SZ );
	fUnpackedFrame = new uint16_t [fUnpackedFrameLength];
	fCurPackedFrame = fData;
	return;

error:
	if( fFile != NULL )
		fclose( fFile );
	if( fBuf )
		delete [] fBuf;
	throw TAccessExc( 1, "THRPTUnpacker::THRPTUnpacker: ошибка доступа к файлу" );
}



THRPTUnpacker::THRPTUnpacker( char * file_data ) :
		fFile( NULL ),
		fBuf( file_data ),
		fCurFrameNum( 0 ),
		fExtBufFlag( 1 )	/* буфер предоставлен снаружи, поэтому при уничтожении объекта удалять его не надо */
{
	init( fBuf );

	fBlk0 = (TBlk0 *)fBuf;
	fData = (uint16_t *)(fBuf + BLK0_SZ);
	fUnpackedFrame = new uint16_t [fUnpackedFrameLength];
	fCurPackedFrame = fData;
}



void THRPTUnpacker::init( char * buf ) {
	fFileFormat = (buf[0]&0xff == 0xff) ? 1 : 2;

	if( fFileFormat == 1 ) {
		TBlk0_HRPT *sp = (TBlk0_HRPT *)buf;
		fPackType = sp->packType;
		fTotalFrames = sp->frameNum + sp->lostFrameNum;
		fMask = sp->frameMask;
		fTotalPix = sp->totalPixNum;
		fPixPassed = sp->pixGap;
		fPix = sp->pixNum;
	} else {
		//TODO: n33d big endian fix
		fPackType = buf[257];
		fTotalFrames = *((uint16_t *)(buf + 258));
		fMask = *((uint16_t *)(buf + 264));
		fTotalPix = 2048;
		fPixPassed = *((uint16_t *)(buf + 266));
		fPix = *((uint16_t *)(buf + 268));
	}

	// наличие каналов AVHRR
	fNChannels = 0;
	for( int i = 0; i < 5; i++ ) {
		if( fMask & ( 0x8000 >> i ) ) {
			fChannels[i] = 1;
			fNChannels++;
		} else
			fChannels[i] = 0;
	}

	fAVHRROffset = 0;
	for( int i = 0; i < 8; i++ ) {
		if( fMask & ( 1 << i ) )
			fAVHRROffset += hrptFrameLengths[i];
		if( i == 1 )
			fTLMOffset = fAVHRROffset;
		if( i == 6 )
			fTIPOffset = fAVHRROffset;
	}
	/*	Если в кадре присутствует информация TIP,
		отнимаем длину синхросерий TIP,
		так как синхросерии TIP в кадр не записываются. */
	if( fMask & 0x40 )
		fAVHRROffset -= LTNO;	

	fUnpackedFrameLength = buildMask( 0, fMask, fPixPassed, fPix );
	if( fPackType == 0 )
		fPackedFrameLength = fUnpackedFrameLength; /* Упаковка: 1*10->2b */
	else {  /* Упаковка: 8*10->10b */
		fPackedFrameLength = (fUnpackedFrameLength * 10) >> 4;
		if( (fUnpackedFrameLength * 10) & 16 )
			fPackedFrameLength++;
		fPackedFrameLength++;
	}
}



THRPTUnpacker::~THRPTUnpacker() {
	if( !fExtBufFlag )
		delete [] fBuf;
	delete [] fUnpackedFrame;
	if( fFile != NULL )
		fclose( fFile );
}


void THRPTUnpacker::setCurrentFrameNumber( uint32_t frame_num ) throw ( TAccessExc, TRequestExc ) {
	TAccessExc ae1( 1, "THRPTUnpacker::setCurrentFrameNumber: ошибка доступа к файлу" );
	TRequestExc re1( 2, "THRPTUnpacker::setCurrentFrameNumber: кадр с указанным номером в файле отсутствует" );

	if( frame_num >= fTotalFrames )
		throw re1;

	if( frame_num == fCurFrameNum )
		return;

	if( fFile == NULL ) {
		fCurPackedFrame = fData + frame_num * fPackedFrameLength;
	} else {
		if( fseek( fFile, BLK0_SZ + frame_num * fPackedFrameLength * 2, SEEK_SET ) != 0 )
			throw ae1;
	}

	fCurFrameNum = frame_num;
}


void THRPTUnpacker::unpackNextFrame( uint16_t * buffer ) throw( TAccessExc ) {

	if( fCurFrameNum == fTotalFrames )
		return;     /* если распакованы все кадры файла */

	if( fFile != NULL ) {
		if( fread( fData, 2, fPackedFrameLength, fFile ) != fPackedFrameLength )
			throw TAccessExc( 1, "THRPTUnpacker::unpackNextFrame: ошибка доступа к файлу" );
	}

	/* Распаковка кадра. */
	if( fPackType == 0 ) {
		uint16_t* dst = buffer;
		uint16_t* dst_end = dst + fUnpackedFrameLength;
		uint16_t* src = fCurPackedFrame;
		while( dst != dst_end )
			*dst++ = *src++ & 0x3ff;
	} else {
		unpack810( fCurPackedFrame, buffer, fUnpackedFrameLength );
	}

	if( fFile == NULL )
		fCurPackedFrame += fPackedFrameLength;
	fCurFrameNum++;
}


void THRPTUnpacker::unpackNextFrame( uint16_t * avhrrBuffers[5], uint16_t * tlmBuffers[3], uint16_t* tipBuffer ) throw ( TAccessExc, TRequestExc ) {

	unpackNextFrame( fUnpackedFrame );

	/* если кадр со сбоем синхронизации */
	if( (fUnpackedFrame[0] | fUnpackedFrame[1] | fUnpackedFrame[2] | fUnpackedFrame[3] |
			fUnpackedFrame[4] | fUnpackedFrame[5] | fUnpackedFrame[6] | fUnpackedFrame[7]) == 0 )
		throw TRequestExc( 2, "THRPTUnpacker::unpackNextFrame: кадр со сбоем синхронизации" );

	/* Выделение из распакованных данных каналов AVHRR. */
	unsigned channelOffset = fAVHRROffset;
	for( int i=0; i<5; i++ ) {
		if( fChannels[i] ) {
			if( avhrrBuffers[i] ) {
				memset( avhrrBuffers[i], 0, fPixPassed<<1 );
				uint16_t* dst = avhrrBuffers[i] + fPixPassed;
				uint16_t* dst_end = dst + fPix;
				uint16_t* src = fUnpackedFrame + channelOffset;
				while( dst != dst_end ) {
					*dst++ = *src;
					src += fNChannels;
				}
				memset( dst_end, 0, (fTotalPix - fPixPassed - fPix)<<1 );
			}
			channelOffset++;
		}
	}

	/* Выделение из распакованных данных телеметрии. */
	if( tlmBuffers && (fMask & 0x4) ) {	/* Если в файле присутствуют данные телеметрии. */
		for( int i=0; i<3; i++ ) {
			if( tlmBuffers[i] ) {
				uint16_t* dst = tlmBuffers[i];
				/* Telemetry AVHRR - 10 слов */
				uint32_t j = fTLMOffset;
				for( uint32_t k = 0 ; k < LTLM; k++, j++ )
					*dst++ = fUnpackedFrame[j];
				/* AVHRR Internal target data - 10 слов */
				j += i;
				for( uint32_t k = 0; k < LBSC/3; k++, j += 3 )
					*dst++ = fUnpackedFrame[j];
				/* Space data AVHRR - 10 слов */
				j = fTLMOffset + LTLM + LBSC + i + 2;
				for( uint32_t k = 0; k < LSPC/5; k++, j += 5 )
					*dst++ = fUnpackedFrame[j];
			}
		}
	}

	/* Выделение из распакованных данных информации TIP. */
	if( tipBuffer && (fMask & 0x40) ) {	/* Если в файле присутствует информация TIP. */
		memcpy( tipBuffer, fUnpackedFrame + fTIPOffset, 1010 );
	}

}


unsigned THRPTUnpacker::buildMask( uint8_t * frameMask, unsigned code, unsigned pass_pix, unsigned n_pix ) {
	unsigned i, j, k, n;
	unsigned result = 0;
	uint8_t maskValue;
	uint8_t* p = NULL;
	uint8_t* end = NULL;

	/* синхросерия начала кадра */
	if( frameMask ) {
		p = frameMask;
		end = frameMask + LFSY;
		maskValue = ( code & MFSY ) ? WRALLF : NOWR;
		while( p < end )
			*p++ = maskValue;
	}
	/* заносим обязательный код синхросерии HRPT */
	if( frameMask )
		frameMask[SSHRPT] |= SYNHRPT;

	if( code & MFSY )
		result += LFSY;
	else
		result++; /* учитываем байт по смещению SSHRPT */

	/* идентификатор спутника */
	/* код времени */
	/* телеметрия AVHRR */
	/* сканирование черного тела */
	/* сканирование космоса (SPACE) */
	/* синхросерия AVHRR */
	/* TIP */
	/* резервные слова SPARE */
	for( i = 0; i < 8; i++ ) {
		if( frameMask ) {
			end += hrptFrameLengths[i];
			maskValue = ( code & (1 << i) ) ? WRALLF : NOWR;
			while( p < end )
				*p++ = maskValue;
		}
		if( code & (1 << i) )
			result += hrptFrameLengths[i];
	}

	/* заносим обязательные коды синхросерий TIP */
	/* три слова синхросерии TIP-а не писать никогда */
	if( frameMask ) {
		for( i = TIP; i < SSTIP5; i += LTIP/5 ) {
			frameMask[i] |= NOWR;
			frameMask[i+1] |= NOWR;
			frameMask[i+2] = SYNTIP;
		}
	}
	result -= LTNO;

	/* каналы AVHRR */
	if( frameMask ) {
		end += LAVH;
		while( p < end )
			*p++ = NOWR;
	}
	k = AVH + pass_pix * 5; /* адрес 1-го требуемого пиксела AVHRR */
	for( j = 0; j < 5; j++, k++ ) {
		if( code & (0x8000 >> j) ) {
			if( frameMask ) {
				for( i = k, n = 0; n < n_pix; i += 5, n++ )
					frameMask[i] = WRALLF;
			}
			result += n_pix;
		}
	}

	/* синхросерия конца кадра */
	if( frameMask ) {
		end += LASY;
		maskValue = ( code & MASY ) ? WRALLF : NOWR;
		while( p < end )
			*p++ = maskValue;
	}
	if( code & MASY )
		result += LASY;

	return result;
}


void THRPTUnpacker::channelsAvailability( int availabilityArray[5] ) {
	memcpy( availabilityArray, fChannels, 5 * sizeof( int ) );
}


int THRPTUnpacker::channelAvailable( int i ) {
	return fChannels[i];
}


uint32_t THRPTUnpacker::totalFrames() {
	return fTotalFrames;
}


uint32_t THRPTUnpacker::currentFrameNumber() {
	return fCurFrameNum;
}

TBlk0* THRPTUnpacker::blk0() {
	return fBlk0;
}


uint32_t THRPTUnpacker::mask() {
	return fMask;
}


uint32_t THRPTUnpacker::frameWordLength() {
	return fUnpackedFrameLength;
}


uint32_t THRPTUnpacker::packedFrameWordLength() {
	return fPackedFrameLength;
}
