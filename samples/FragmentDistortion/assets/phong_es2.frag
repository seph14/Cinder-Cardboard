#version 100

precision highp float;

uniform mediump vec4 uLightPosition;
uniform mediump vec4 uSkyDirection;

varying highp vec4 vWorldPosition;
varying highp vec3 vNormal;

void main( void )
{		
	const mediump vec3 matDiffuse = vec3( 1.0 );
	const mediump vec3 lightDiffuse = vec3( 1.0 );

	vec3 V = vWorldPosition.xyz;
	vec3 N = normalize( vNormal );
	vec3 L = normalize( uLightPosition.xyz - V );
	vec3 R = normalize( -reflect( L, N ) );

	// hemispherical ambient lighting
	float hemi = dot( N, uSkyDirection.xyz ) * 0.5 + 0.5;
	gl_FragColor.rgb = mix( vec3(0.0), vec3(0.1), hemi );

	// diffuse lighting
	float lambert = max( 0.0, dot( N, L ) );
	gl_FragColor.rgb += lambert * matDiffuse * lightDiffuse;

	// output gamma corrected color
	gl_FragColor.rgb = sqrt( gl_FragColor.rgb );
	gl_FragColor.a = 1.0;
}