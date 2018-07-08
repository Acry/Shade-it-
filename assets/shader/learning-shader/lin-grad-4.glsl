void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	//Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord/iResolution.xy;

	
	vec3 col = vec3(0.6, 0.6, 0.6);
	col *= length(uv-vec2(0.5,0.5));
	// Output to screen
	fragColor = vec4(col,1.0);
}
