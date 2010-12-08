/*----------------------------------------------------------------------------------------------------------
    contour.hpp
    Класс реализующий процедуры выделения контуров и их анализа для работы со спутниковыми изображениями.
----------------------------------------------------------------------------------------------------------*/
#ifndef _CONTOUR_HPP_
#define _CONTOUR_HPP_


class TContourImage {
public:

    TContourImage();

    virtual ~TContourImage();


/****************************************************************************************************************************************************************************************
    Выделение контурных пикселов береговой линии для бинарного изображения (в частности маска суша/море).
    Если mark_sea = sea или mark_land = mark_land, тогда контурные пикселы пренадлежат суше или морю.

    Параметры:
    b_image                             Бинарное изображение, результат - выделенные контурные пикселы, помечаются в нём
    sea, land                           Значения для моря и суши в b_image
    cloudy_mask                         Маска облачности для b_image (0 - пиксел безоблачный, 1 - пиксел покрыт облачностью)
    width, height                       Количество строк и количество пикселов в строке для b_image и cloudy_mask
    mark_sea, mark_land                 Значения, которыми помечаются контурные пикселы (открытые от облачности), как для моря так и для суши в b_image
    mark_cloudy_sea, mark_cloudy_land   Значения, которыми помечаются контурные пикселы (покрытые облачностью), как для моря так и для суши в b_image
*****************************************************************************************************************************************************************************************/
    void coastal_lineBinaryImage( const char * b_image, char sea, char land, const char * cloudy_mask,
                                               long width, long height, char mark_sea, char mark_land,
                                               char mark_cloudy_sea, char mark_cloudy_land );


    void coastal_lineBinaryImage_withThickness( int number_gcp, uint8_t koff, const char * b_image, char sea, char land, const char * cloudy_mask,
                                                long width, long height, char mark_sea, char mark_land,
                                                char mark_cloudy_sea, char mark_cloudy_land, bool flag );


private:

    char * fCloudyMask;

    char * fBinaryImage;

    long fWidth;
    long fHeight;
};


#endif
