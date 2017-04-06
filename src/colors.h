_float4 haxby_colormap(float value) {

    _float4 color;

    unsigned low = (unsigned)(float)floor(value * (haxby_colors - 1));
    unsigned high = (unsigned)(float)ceil(value * (haxby_colors - 1));

    float v = (float)(haxby_colors - 1) * (value - (float)floor(value * (haxby_colors - 1)) / (haxby_colors - 1));

    float r = (float)linear_interpolation( v, haxby[low][0], haxby[high][0] );
    float g = (float)linear_interpolation( v, haxby[low][1], haxby[high][1] );
    float b = (float)linear_interpolation( v, haxby[low][2], haxby[high][2] );

    color.r = r / 255.0f;
    color.g = g / 255.0f;
    color.b = b / 255.0f;
    color.a = 1.0f;

    return color;

}

_float4 seismic_colormap(float value) {

    _float4 color;

    unsigned low = (unsigned)(float)floor(value * (seismic_colors - 1));
    unsigned high = (unsigned)(float)ceil(value * (seismic_colors - 1));

    float v = (float)(seismic_colors - 1) * (value - (float)floor(value * (seismic_colors - 1)) / (seismic_colors - 1));

    float r = (float)linear_interpolation( v, seismic[low][0], seismic[high][0] );
    float g = (float)linear_interpolation( v, seismic[low][1], seismic[high][1] );
    float b = (float)linear_interpolation( v, seismic[low][2], seismic[high][2] );

    color.r = r / 255.0f;
    color.g = g / 255.0f;
    color.b = b / 255.0f;
    color.a = 1.0f;

    return color;

}