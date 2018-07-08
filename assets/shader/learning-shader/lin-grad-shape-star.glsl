void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	// Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord/iResolution.xy;
	
	// Distance to the centre
	vec2 q  = uv-vec2(0.5,0.5);
	
	vec3 col = vec3(0.6, 0.6, 0.6);
	
	// using smoothstep now
	// gradient between smoothstep
	float r= 0.2 + 0.1*cos(atan(q.x,q.y)*10);
	col *= smoothstep ( r, r+0.3, length(q));
	// Output to screen
	fragColor = vec4(col,1.0);
}
