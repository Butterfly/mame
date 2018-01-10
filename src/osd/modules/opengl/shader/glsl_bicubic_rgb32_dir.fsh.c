const char glsl_bicubic_rgb32_dir_fsh_src[] =
"// license:BSD-3-Clause\n"
"// copyright-holders:Aaron Stover\n"
"#pragma optimize (on)\n"
"#pragma debug (off)\n"
"\n"
"uniform sampler2D     color_texture;\n"
"uniform vec2          color_texture_pow2_sz; // pow2 tex size\n"
"uniform vec4          vid_attributes;        // gamma, contrast, brightness\n"
"\n"
"#define TEX2D(v) texture2D(color_texture,(v))\n"
"\n"
"void main()\n"
"{\n"
"    vec2 pixel    = gl_TexCoord[0].st * color_texture_pow2_sz - 0.5;\n"
"    vec2 uv_ratio = fract(pixel);\n"
"    vec2 one      = 1.0 / color_texture_pow2_sz;\n"
"    vec2 xy       = (floor(pixel) + 0.5) * one;\n"
"\n"
"    //\n"
"    // Robert G. Keys'\n"
"    //\n"
"    // 'Cubic Convolution Interpolation for Digital Image Processing'\n"
"    // IEEE TRANSACTIONS ON ACOUSTICS,  SPEECH, AND SIGNAL PROCESSING, \n"
"    // VOL. ASSP-29, NO. 6, DECEMBER 1981 \n"
"    //\n"
"    // gives the following equation:\n"
"    //\n"
"    // g(x) = c_k-1(-s^3 + 2s^2 - s)/2 \n"
"    //      + c_k(3s^3 - 5s^2 + 2)/2\n"
"    //      + c_k+1(-3s^3 + 4s^2 + s)/2 \n"
"    //      + c_k+2(s^3 - s^2)/2\n"
"    //\n"
"    // c_* are the sample values, s is our uv_ratio. \n"
"    //\n"
"    // Weight equations are taken from above.\n"
"    //\n"
"    vec2 s  = uv_ratio;\n"
"    vec2 s2 = uv_ratio*uv_ratio;\n"
"    vec2 s3 = uv_ratio*uv_ratio*uv_ratio;\n"
"    vec2 w0 = 0.5 * (-s3 + 2.0*s2 - s);\n"
"    vec2 w1 = 0.5 * (3.0*s3 - 5.0*s2 + 2.0);\n"
"    vec2 w2 = 0.5 * (-3.0*s3 + 4.0*s2 + s);\n"
"    vec2 w3 = 0.5 * (s3 - s2);\n"
"\n"
"    // Compute the coordinates to sample from\n"
"    vec2 pos0 = xy - 1.0*one.xy;\n"
"    vec2 pos1 = xy;\n"
"    vec2 pos2 = xy + 1.0*one.xy;\n"
"    vec2 pos3 = xy + 2.0*one.xy;\n"
"\n"
"    // Finally - take the samples, multiply by weight, and sum\n"
"    vec4 col = vec4(0.0);\n"
"    col += TEX2D(vec2(pos0.x, pos0.y)) *  w0.x * w0.y;\n"
"    col += TEX2D(vec2(pos1.x, pos0.y)) *  w1.x * w0.y;\n"
"    col += TEX2D(vec2(pos2.x, pos0.y)) *  w2.x * w0.y;\n"
"    col += TEX2D(vec2(pos3.x, pos0.y)) *  w3.x * w0.y;\n"
"\n"
"    col += TEX2D(vec2(pos0.x, pos1.y)) *  w0.x * w1.y;\n"
"    col += TEX2D(vec2(pos1.x, pos1.y)) *  w1.x * w1.y;\n"
"    col += TEX2D(vec2(pos2.x, pos1.y)) *  w2.x * w1.y;\n"
"    col += TEX2D(vec2(pos3.x, pos1.y)) *  w3.x * w1.y;\n"
"    \n"
"    col += TEX2D(vec2(pos0.x, pos2.y)) *  w0.x * w2.y;\n"
"    col += TEX2D(vec2(pos1.x, pos2.y)) *  w1.x * w2.y;\n"
"    col += TEX2D(vec2(pos2.x, pos2.y)) *  w2.x * w2.y;\n"
"    col += TEX2D(vec2(pos3.x, pos2.y)) *  w3.x * w2.y;\n"
"\n"
"    col += TEX2D(vec2(pos0.x, pos3.y)) *  w0.x * w3.y;\n"
"    col += TEX2D(vec2(pos1.x, pos3.y)) *  w1.x * w3.y;\n"
"    col += TEX2D(vec2(pos2.x, pos3.y)) *  w2.x * w3.y;\n"
"    col += TEX2D(vec2(pos3.x, pos3.y)) *  w3.x * w3.y;\n"
"\n"
"#ifdef DO_GAMMA\n"
"    // gamma/contrast/brightness\n"
"    vec4 gamma = vec4(1.0 / vid_attributes.r, 1.0 / vid_attributes.r, 1.0 / vid_attributes.r, 0.0);\n"
"    gl_FragColor =  ( pow ( col, gamma ) * vid_attributes.g) + vid_attributes.b - 1.0;\n"
"#else\n"
"    // contrast/brightness\n"
"    gl_FragColor =  ( col                * vid_attributes.g) + vid_attributes.b - 1.0;\n"
"#endif\n"
"}\n";