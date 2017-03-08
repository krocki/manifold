/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-06 13:20:16
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-06 20:40:26
*/

int nvgCreateImageA(NVGcontext* ctx, int w, int h, int imageFlags, const unsigned char* data) {

	return nvgInternalParams(ctx)->renderCreateTexture(nvgInternalParams(ctx)->userPtr, NVG_TEXTURE_ALPHA, w, h, imageFlags, data);

}