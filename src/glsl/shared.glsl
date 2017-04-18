
float colormap_red ( float x ) {
	if ( x < 0.7 )
		return 4.0 * x - 1.5;
	else
		return -4.0 * x + 4.5;
}

float colormap_green ( float x ) {
	if ( x < 0.5 )
		return 4.0 * x - 0.5;
	else
		return -4.0 * x + 3.5;
}

float colormap_blue ( float x ) {
	if ( x < 0.3 )
		return 4.0 * x + 0.5;
	else
		return -4.0 * x + 2.5;
}

const int haxby_colors = 20;
const int seismic_colors = 20;
const int parula_colors = 10;

vec3 haxby_lut[20] =
	vec3[] (
		vec3 ( 9,     0,   121 ),
		vec3 ( 40,     0,   150 ),
		vec3 ( 0,     9,   200 ),
		vec3 ( 0,    25,   212 ),
		vec3 ( 26,   102,   240 ),
		vec3 ( 25,   175,   255 ),
		vec3 ( 50,   190,   255 ),
		vec3 ( 97,   225,   240 ),
		vec3 ( 106,   236,   225 ),
		vec3 ( 138,   236,   174 ),
		vec3 ( 205,   255,   162 ),
		vec3 ( 223,   246,   141 ),
		vec3 ( 248,   215,   104 ),
		vec3 ( 255,   189,    87 ),
		vec3 ( 244,   117,    75 ),
		vec3 ( 255,    90,    90 ),
		vec3 ( 255,   124,   124 ),
		vec3 ( 246,   179,   174 ),
		vec3 ( 255,   196,   196 ),
		vec3 ( 255,   236,   236 )
	);


vec3 seismic_lut[20] =
	vec3[] (
		vec3 ( 0, 	 36,   227 ),
		vec3 ( 0,    72,   250 ),
		vec3 ( 0,   135,   205 ),
		vec3 ( 0,   208,   139 ),
		vec3 ( 22,   244,    90 ),
		vec3 ( 63,   250,    54 ),
		vec3 ( 115,   255,    26 ),
		vec3 ( 189,   255,    12 ),
		vec3 ( 255,   255,     0 ),
		vec3 ( 255,   255,     0 ),
		vec3 ( 255,   255,     0 ),
		vec3 ( 255,   221,     0 ),
		vec3 ( 255,   183,     0 ),
		vec3 ( 255,   144,     0 ),
		vec3 ( 255,   106,     0 ),
		vec3 ( 255,    68,     0 ),
		vec3 ( 255,    29,     0 ),
		vec3 ( 247,     0,     0 ),
		vec3 ( 208,     0,     0 ),
		vec3 ( 170,     0,     0 )
	);



vec3 parula_lut[10] =
	vec3[] (
		vec3 ( 53, 42, 135 ),
		vec3 ( 15, 92, 221 ),
		vec3 ( 18, 125, 216 ),
		vec3 ( 7, 156, 207 ),
		vec3 ( 21, 177, 180 ),
		vec3 ( 89, 189, 140 ),
		vec3 ( 165, 190, 107 ),
		vec3 ( 225, 185, 82 ),
		vec3 ( 252, 206, 46 ),
		vec3 ( 249, 251, 14 )
	);

float saturate ( float x ) { return clamp ( x, 0.0, 1.0 ); }

vec3 viridis_quintic ( float x ) {
	x = saturate ( x );
	vec4 x1 = vec4 ( 1.0, x, x * x, x * x * x ); // 1 x x2 x3
	vec4 x2 = x1 * x1.w * x; // x4 x5 x6 x7
	return vec3 (
			   dot ( x1.xyzw, vec4 ( +0.280268003, -0.143510503, +2.225793877, -14.815088879 ) ) + dot ( x2.xy, vec2 ( +25.212752309, -11.772589584 ) ),
			   dot ( x1.xyzw, vec4 ( -0.002117546, +1.617109353, -1.909305070, +2.701152864 ) ) + dot ( x2.xy, vec2 ( -1.685288385, +0.178738871 ) ),
			   dot ( x1.xyzw, vec4 ( +0.300805501, +2.614650302, -12.019139090, +28.933559110 ) ) + dot ( x2.xy, vec2 ( -33.491294770, +13.762053843 ) ) );
}

vec3 hue ( float t, int discretize ) {
	if ( discretize == 1 ) t = ( floor ( t * 10 ) / 10 );
	vec3 p = abs ( fract ( t + vec3 ( 1.0, 2.0 / 3.0, 1.0 / 3.0 ) ) * 6.0 - 3.0 );
	return ( clamp ( p - 1.0, 0.0, 1.0 ) );
}

vec3 spectrum ( float x, int discretize ) { // RGB <0,1> <- lambda l <400,700> [nm]
	float l = 400 + x * 300;
	if ( discretize == 1 ) l = ( floor ( l * 10 ) / 10 );
	
	float r = 0.0, g = 0.0, b = 0.0;
	if ( ( l >= 400.0 ) && ( l < 410.0 ) ) { float t = ( l - 400.0 ) / ( 410.0 - 400.0 ); r =    + ( 0.33 * t ) - ( 0.20 * t * t ); }
	else
		if ( ( l >= 410.0 ) && ( l < 475.0 ) ) { float t = ( l - 410.0 ) / ( 475.0 - 410.0 ); r = 0.14         - ( 0.13 * t * t ); }
		else
			if ( ( l >= 545.0 ) && ( l < 595.0 ) ) { float t = ( l - 545.0 ) / ( 595.0 - 545.0 ); r =    + ( 1.98 * t ) - ( t * t ); }
			else
				if ( ( l >= 595.0 ) && ( l < 650.0 ) ) { float t = ( l - 595.0 ) / ( 650.0 - 595.0 ); r = 0.98 + ( 0.06 * t ) - ( 0.40 * t * t ); }
				else
					if ( ( l >= 650.0 ) && ( l < 700.0 ) ) { float t = ( l - 650.0 ) / ( 700.0 - 650.0 ); r = 0.65 - ( 0.84 * t ) + ( 0.20 * t * t ); }
	if ( ( l >= 415.0 ) && ( l < 475.0 ) ) { float t = ( l - 415.0 ) / ( 475.0 - 415.0 ); g =             + ( 0.80 * t * t ); }
	else
		if ( ( l >= 475.0 ) && ( l < 590.0 ) ) { float t = ( l - 475.0 ) / ( 590.0 - 475.0 ); g = 0.8 + ( 0.76 * t ) - ( 0.80 * t * t ); }
		else
			if ( ( l >= 585.0 ) && ( l < 639.0 ) ) { float t = ( l - 585.0 ) / ( 639.0 - 585.0 ); g = 0.82 - ( 0.80 * t )           ; }
	if ( ( l >= 400.0 ) && ( l < 475.0 ) ) { float t = ( l - 400.0 ) / ( 475.0 - 400.0 ); b =    + ( 2.20 * t ) - ( 1.50 * t * t ); }
	else
		if ( ( l >= 475.0 ) && ( l < 560.0 ) ) { float t = ( l - 475.0 ) / ( 560.0 - 475.0 ); b = 0.7 - ( t ) + ( 0.30 * t * t ); }
		
	return vec3 ( r, g, b );
}

#define COLORMAP_GRAY 0
#define COLORMAP_JET 1
#define COLORMAP_PARULA 2
#define COLORMAP_VIRIDIS 3
#define COLORMAP_HSV 4
#define COLORMAP_RGB 5
#define COLORMAP_HAXBY 6
#define COLORMAP_SEISMIC 7

//colormaps
vec3 gray ( float x, int discretize ) {

	if ( discretize == 1 ) x = ( floor ( x * 10 ) / 10 );
	return vec3 ( x, x, x );
}

vec3 jet ( float x, int discretize ) {

	if ( discretize == 1 ) x = ( floor ( x * 10 ) / 10 );
	float r = clamp ( colormap_red ( x ), 0.0, 1.0 );
	float g = clamp ( colormap_green ( x ), 0.0, 1.0 );
	float b = clamp ( colormap_blue ( x ), 0.0, 1.0 );
	return vec3 ( r, g, b );
}

vec3 haxby ( float x, int discretize ) {

	int a = int ( floor ( x * ( haxby_colors - 1 ) ) );
	
	if ( discretize == 1 )
		return haxby_lut[a] / 255.0f;
		
	int b = int ( ceil ( x * ( haxby_colors - 1 ) ) );
	float t = fract ( x * ( haxby_colors - 1 ) );
	
	return mix ( haxby_lut[a] / 255.0f, haxby_lut[b] / 255.0f, t );
}

vec3 seismic ( float x, int discretize ) {

	int a = int ( floor ( x * ( seismic_colors - 1 ) ) );
	
	if ( discretize == 1 )
		return seismic_lut[a] / 255.0f;
		
	int b = int ( ceil ( x * ( seismic_colors - 1 ) ) );
	float t = fract ( x * ( seismic_colors - 1 ) );
	
	return mix ( seismic_lut[a] / 255.0f, seismic_lut[b] / 255.0f, t );
}

vec3 parula ( float x, int discretize ) {

	int a = int ( floor ( x * ( parula_colors - 1 ) ) );
	
	if ( discretize == 1 )
		return parula_lut[a] / 255.0f;
		
	int b = int ( ceil ( x * ( parula_colors - 1 ) ) );
	float t = fract ( x * ( parula_colors - 1 ) );
	return mix ( parula_lut[a] / 255.0f, parula_lut[b] / 255.0f, t );
}

vec3 viridis ( float x, int discretize ) {

	if ( discretize == 1 ) x = ( floor ( x * 10 ) / 10 );
	return viridis_quintic ( x );
}

vec4 apply_colormap ( vec3 val, int colormap, float alpha ) {

	int discretize = 1;
	
	if ( colormap == COLORMAP_GRAY )
		return vec4 ( gray ( val.r, discretize ), alpha );
	else
		if ( colormap == COLORMAP_JET )
			return vec4 ( jet ( val.r, discretize ), alpha );
		else
			if ( colormap == COLORMAP_PARULA )
				return vec4 ( parula ( val.r, discretize ), alpha );
			else
				if ( colormap == COLORMAP_VIRIDIS )
					return vec4 ( viridis ( val.r, discretize ), alpha );
				else
					if ( colormap == COLORMAP_HSV )
						return vec4 ( hue ( val.r, discretize ), alpha );
					else
						if ( colormap == COLORMAP_RGB )
							return vec4 ( spectrum ( val.r, discretize ), alpha );
						else
							if ( colormap == COLORMAP_HAXBY )
								return vec4 ( haxby ( val.r, discretize ), alpha );
							else
								if ( colormap == COLORMAP_SEISMIC )
									return vec4 ( seismic ( val.r, discretize ), alpha );
								else
									return vec4 ( val, alpha );
}
