/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-06 11:10:30
*/


	//vec3 c = rainbowGradient(val);
	//vec3 c = pal( val, PALETTE_8 );
	//vec3 col=spectral_palette(val);
	//vec3 c = pow(col,vec3(1.0/2.2));
	//vec3 c = viridis_quintic(val);
	vec3 c = jet(val);
	frag_color = vec4(c, 0.5);


//TODO: move

float square(float s) { return s * s; }
vec3 square(vec3 s) { return s * s; }

vec3 hueGradient(float t) {
    vec3 p = abs(fract(t + vec3(1.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0);
	return (clamp(p - 1.0, 0.0, 1.0));
}


vec3 techGradient(float t) {
	return pow(vec3(t + 0.01), vec3(120.0, 10.0, 180.0));
}


vec3 fireGradient(float t) {
	return max(pow(vec3(min(t * 1.02, 1.0)), vec3(1.7, 25.0, 100.0)),
			   vec3(0.06 * pow(max(1.0 - abs(t - 0.35), 0.0), 5.0)));
}


vec3 desertGradient(float t) {
	float s = sqrt(clamp(1.0 - (t - 0.4) / 0.6, 0.0, 1.0));
	vec3 sky = sqrt(mix(vec3(1, 1, 1), vec3(0, 0.8, 1.0), smoothstep(0.4, 0.9, t)) * vec3(s, s, 1.0));
	vec3 land = mix(vec3(0.7, 0.3, 0.0), vec3(0.85, 0.75 + max(0.8 - t * 20.0, 0.0), 0.5), square(t / 0.4));
	return clamp((t > 0.4) ? sky : land, 0.0, 1.0) * clamp(1.5 * (1.0 - abs(t - 0.4)), 0.0, 1.0);
}


vec3 electricGradient(float t) {
	return clamp( vec3(t * 8.0 - 6.3, square(smoothstep(0.6, 0.9, t)), pow(t, 3.0) * 1.7), 0.0, 1.0);
}


vec3 neonGradient(float t) {
	return clamp(vec3(t * 1.3 + 0.1, square(abs(0.43 - t) * 1.7), (1.0 - t) * 1.7), 0.0, 1.0);
}


vec3 heatmapGradient(float t) {
	return clamp((pow(t, 1.5) * 0.8 + 0.2) * vec3(smoothstep(0.0, 0.35, t) + t * 0.5, smoothstep(0.5, 1.0, t), max(1.0 - t * 1.7, t * 7.0 - 6.0)), 0.0, 1.0);
}


vec3 rainbowGradient(float t) {
	vec3 c = 1.0 - pow(abs(vec3(t) - vec3(0.65, 0.5, 0.2)) * vec3(3.0, 3.0, 5.0), vec3(1.5, 1.3, 1.7));
	c.r = max((0.15 - square(abs(t - 0.04) * 5.0)), c.r);
	c.g = (t < 0.5) ? smoothstep(0.04, 0.45, t) : c.g;
	return clamp(c, 0.0, 1.0);
}


vec3 brightnessGradient(float t) {
	return vec3(t * t);
}


vec3 grayscaleGradient(float t) {
	return vec3(t);
}


vec3 stripeGradient(float t) {
	return vec3(mod(floor(t * 32.0), 2.0) * 0.2 + 0.8);
}


vec3 ansiGradient(float t) {
	return mod(floor(t * vec3(8.0, 4.0, 2.0)), 2.0);
}

vec3 pal( in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d )
{

    vec3 col = a + b*cos( 6.28318*(c*t+d) );
    return col;
}

//https://www.shadertoy.com/view/lt2GDc
#define PALETTE_0 vec3(0.5,0.5,0.5),vec3(0.5,0.5,0.5),vec3(0.8,0.8,0.8),vec3(0.0,0.33,0.67)+0.21
#define PALETTE_1 vec3(0.55,0.4,0.3),vec3(0.50,0.51,0.35)+0.1,vec3(0.8,0.75,0.8),vec3(0.075,0.33,0.67)+0.21
#define PALETTE_2 vec3(0.55),vec3(0.8),vec3(0.29),vec3(0.00,0.05,0.15) + 0.54
#define PALETTE_3 vec3(0.55),vec3(0.45),vec3(0.00,0.10,0.20) + 0.47
#define PALETTE_4 vec3(0.5),vec3(0.5),vec3(0.9),vec3(0.3,0.20,0.20) + 0.31
#define PALETTE_5 vec3(0.5),vec3(0.5),vec3(0.9),vec3(0.0,0.10,0.20) + 0.47
#define PALETTE_6 vec3(0.5),vec3(0.5),vec3(1.0,1.0,0.5),vec3(0.8,0.90,0.30)
#define PALETTE_7 vec3(0.5),vec3(0.5),vec3(1.0,0.7,0.4),vec3(0.0,0.15,0.20)
#define PALETTE_8 vec3(0.5),vec3(0.5),vec3(2.0,1.0,0.0),vec3(0.5,0.20,0.25)
#define PALETTE_9 vec3(0.5),vec3(0.5),vec3(1),vec3(0.0,0.33,0.67)
#define PALETTE_10 vec3(0.8,0.5,0.4),vec3(0.2,0.4,0.2),vec3(2.0,1.0,1.0),vec3(0.0,0.25,0.25)










// Based on code by Spektre posted at http://stackoverflow.com/questions/3407942/rgb-values-of-visible-spectrum
vec3 spectral_colour(float l) // RGB <0,1> <- lambda l <400,700> [nm]
{
	float r=0.0,g=0.0,b=0.0;
         if ((l>=400.0)&&(l<410.0)) { float t=(l-400.0)/(410.0-400.0); r=    +(0.33*t)-(0.20*t*t); }
    else if ((l>=410.0)&&(l<475.0)) { float t=(l-410.0)/(475.0-410.0); r=0.14         -(0.13*t*t); }
    else if ((l>=545.0)&&(l<595.0)) { float t=(l-545.0)/(595.0-545.0); r=    +(1.98*t)-(     t*t); }
    else if ((l>=595.0)&&(l<650.0)) { float t=(l-595.0)/(650.0-595.0); r=0.98+(0.06*t)-(0.40*t*t); }
    else if ((l>=650.0)&&(l<700.0)) { float t=(l-650.0)/(700.0-650.0); r=0.65-(0.84*t)+(0.20*t*t); }
         if ((l>=415.0)&&(l<475.0)) { float t=(l-415.0)/(475.0-415.0); g=             +(0.80*t*t); }
    else if ((l>=475.0)&&(l<590.0)) { float t=(l-475.0)/(590.0-475.0); g=0.8 +(0.76*t)-(0.80*t*t); }
    else if ((l>=585.0)&&(l<639.0)) { float t=(l-585.0)/(639.0-585.0); g=0.82-(0.80*t)           ; }
         if ((l>=400.0)&&(l<475.0)) { float t=(l-400.0)/(475.0-400.0); b=    +(2.20*t)-(1.50*t*t); }
    else if ((l>=475.0)&&(l<560.0)) { float t=(l-475.0)/(560.0-475.0); b=0.7 -(     t)+(0.30*t*t); }

	return vec3(r,g,b);
}

vec3 spectral_palette(float x) { return spectral_colour(x*300.0+400.0); }



// Viridis approximation, Jerome Liard, August 2016
// https://www.shadertoy.com/view/XtGGzG

// Applied polynomial regression to viridis color palettes so I could easily use them in shaders.
// Degree 5 seems to be a good fit (doesn't capture all details but...)
//
// Some credits/reference links about viridis palettes and why they are good, and some use examples:
//
//  https://bids.github.io/colormap/ (says license is CC0)
//  https://github.com/sjmgarnier/viridis#references @sjmgarnier
//  https://cran.r-project.org/web/packages/viridis/vignettes/intro-to-viridis.html
//
// I learned about the existence of this palette via via https://www.mrao.cam.ac.uk/~dag/CUBEHELIX/ via @kenpex

//#define PLOT_CURVES
#define BLACK_BANDS

float saturate( float x ) { return clamp( x, 0.0, 1.0 ); }

vec3 viridis_quintic( float x )
{
	x = saturate( x );
	vec4 x1 = vec4( 1.0, x, x * x, x * x * x ); // 1 x x2 x3
	vec4 x2 = x1 * x1.w * x; // x4 x5 x6 x7
	return vec3(
		dot( x1.xyzw, vec4( +0.280268003, -0.143510503, +2.225793877, -14.815088879 ) ) + dot( x2.xy, vec2( +25.212752309, -11.772589584 ) ),
		dot( x1.xyzw, vec4( -0.002117546, +1.617109353, -1.909305070, +2.701152864 ) ) + dot( x2.xy, vec2( -1.685288385, +0.178738871 ) ),
		dot( x1.xyzw, vec4( +0.300805501, +2.614650302, -12.019139090, +28.933559110 ) ) + dot( x2.xy, vec2( -33.491294770, +13.762053843 ) ) );
}

vec3 inferno_quintic( float x )
{
	x = saturate( x );
	vec4 x1 = vec4( 1.0, x, x * x, x * x * x ); // 1 x x2 x3
	vec4 x2 = x1 * x1.w * x; // x4 x5 x6 x7
	return vec3(
		dot( x1.xyzw, vec4( -0.027780558, +1.228188385, +0.278906882, +3.892783760 ) ) + dot( x2.xy, vec2( -8.490712758, +4.069046086 ) ),
		dot( x1.xyzw, vec4( +0.014065206, +0.015360518, +1.605395918, -4.821108251 ) ) + dot( x2.xy, vec2( +8.389314011, -4.193858954 ) ),
		dot( x1.xyzw, vec4( -0.019628385, +3.122510347, -5.893222355, +2.798380308 ) ) + dot( x2.xy, vec2( -3.608884658, +4.324996022 ) ) );
}

vec3 magma_quintic( float x )
{
	x = saturate( x );
	vec4 x1 = vec4( 1.0, x, x * x, x * x * x ); // 1 x x2 x3
	vec4 x2 = x1 * x1.w * x; // x4 x5 x6 x7
	return vec3(
		dot( x1.xyzw, vec4( -0.023226960, +1.087154378, -0.109964741, +6.333665763 ) ) + dot( x2.xy, vec2( -11.640596589, +5.337625354 ) ),
		dot( x1.xyzw, vec4( +0.010680993, +0.176613780, +1.638227448, -6.743522237 ) ) + dot( x2.xy, vec2( +11.426396979, -5.523236379 ) ),
		dot( x1.xyzw, vec4( -0.008260782, +2.244286052, +3.005587601, -24.279769818 ) ) + dot( x2.xy, vec2( +32.484310068, -12.688259703 ) ) );
}

vec3 plasma_quintic( float x )
{
	x = saturate( x );
	vec4 x1 = vec4( 1.0, x, x * x, x * x * x ); // 1 x x2 x3
	vec4 x2 = x1 * x1.w * x; // x4 x5 x6 x7
	return vec3(
		dot( x1.xyzw, vec4( +0.063861086, +1.992659096, -1.023901152, -0.490832805 ) ) + dot( x2.xy, vec2( +1.308442123, -0.914547012 ) ),
		dot( x1.xyzw, vec4( +0.049718590, -0.791144343, +2.892305078, +0.811726816 ) ) + dot( x2.xy, vec2( -4.686502417, +2.717794514 ) ),
		dot( x1.xyzw, vec4( +0.513275779, +1.580255060, -5.164414457, +4.559573646 ) ) + dot( x2.xy, vec2( -1.916810682, +0.570638854 ) ) );
}

float tri( float x ) { return 1.0 - abs( fract( x * 0.5 ) - 0.5 ) * 2.0; }
vec3 smoothstep_unchecked( vec3 x ) { return ( x * x ) * ( 3.0 - x * 2.0 ); }
vec3 smoothbump( vec3 a, vec3 r, vec3 x ) { return 1.0 - smoothstep_unchecked( min( abs( x - a ), r ) / r ); }



float colormap_red(float x) {
if (x < 0.7) {
return 4.0 * x - 1.5;
} else {
return -4.0 * x + 4.5;
}
}

float colormap_green(float x) {
if (x < 0.5) {
return 4.0 * x - 0.5;
} else {
return -4.0 * x + 3.5;
}
}

float colormap_blue(float x) {
if (x < 0.3) {
return 4.0 * x + 0.5;
} else {
return -4.0 * x + 2.5;
}
}

vec3 jet(float x) {
float r = clamp(colormap_red(x), 0.0, 1.0);
float g = clamp(colormap_green(x), 0.0, 1.0);
float b = clamp(colormap_blue(x), 0.0, 1.0);
return vec3(r, g, b);
}
