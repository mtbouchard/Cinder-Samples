
uniform vec3	colorizeColor;
uniform float	mixFactor;
uniform vec3	keyColor; 
uniform float	thresh;

// the texture 
uniform sampler2D image; 

void main(void) 
{ 
    // sample from the texture  
   vec3 sample = texture2D(image, gl_TexCoord[0].xy).rgb; 

   // calculate difference with keying color
   float difference = length(keyColor.rgb - sample.rgb);

   // 
   float alpha = (difference - thresh) / (1.0 - thresh);
   if(alpha < 0.5) alpha = 0.0; else alpha = 1.0;

   //
   sample = mix(sample, colorizeColor, mixFactor);

   gl_FragColor = vec4(sample, alpha);  
} 