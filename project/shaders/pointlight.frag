#version 330

uniform sampler2D texmap;
uniform sampler2D texmap1;


uniform int texMode;

const int nLights = 9;
const vec3 fogColor = vec3(0.8, 0.6, 0.87);
const float FogDensity = 0.005;
uniform bool enableFog;

out vec4 colorOut;

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	float transparency;
	int texCount;
};

uniform Materials mat;

struct LightProperties {
	bool isEnabled;
	bool isLocal;
	bool isSpot;
	vec3 color;
	vec3 position;
	vec3 halfVector;
	vec3 coneDirection;
	float spotCosCutoff;
	float spotExponent;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
};

uniform LightProperties Lights[nLights];

in Data {
	vec3 normal;
	vec3 eye;
	vec4 position;
	vec2 tex_coord;
	vec4 pos;
} DataIn;

void main() {
/* Old
	vec4 spec = vec4(0.0);
	

	vec3 n = normalize(DataIn.normal);
	vec3 l = normalize(DataIn.lightDir);
	vec3 e = normalize(DataIn.eye);

	float intensity = max(dot(n,l)*0.7, 0.0);

	
	if (intensity > 0.0) {

		vec3 h = normalize(l + e);
		float intSpec = max(dot(h,n), 0.0);
		spec = mat.specular * pow(intSpec, mat.shininess);
	}
	
	colorOut = max(intensity * mat.diffuse + spec, mat.ambient);
*/
	vec4 texel, texel1;

	if(texMode == 2){
		texel = texture(texmap, DataIn.tex_coord);  // texel from texture_id1
		texel1 = texture(texmap1, DataIn.tex_coord);  // texel from texture_id2
	} else if (texMode == 1 || texMode == 3) {
		texel = texture(texmap, DataIn.tex_coord);
	} else if (texMode == 4 ){
		texel = texture(texmap, DataIn.tex_coord);
		if(texel.r == 0){
			colorOut = vec4(0,0,0,0);
			return;
		} else {
			colorOut = vec4(mat.diffuse.rgb, texel.r * mat.transparency);
			return;
		}
	}

	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye - DataIn.position.xyz);
	
	vec3 scatteredLight = vec3(0.0); // or, to a global ambient light
	vec3 reflectedLight = vec3(0.0);
	// loop over all the lights
	for (int light = 0; light < nLights; ++light) {
		if (! Lights[light].isEnabled)
			continue;

		vec3 halfVector;
		vec3 lightDirection = Lights[light].position;
		float attenuation = 1.0;

		// for local lights, compute per-fragment direction,
		// halfVector, and attenuation
		if (Lights[light].isLocal) {
			lightDirection = lightDirection - DataIn.position.xyz;
			float lightDistance = length(lightDirection);
			lightDirection = normalize(lightDirection);
			attenuation = 1.0 / (Lights[light].constantAttenuation + Lights[light].linearAttenuation * lightDistance + Lights[light].quadraticAttenuation * lightDistance * lightDistance);
			if (Lights[light].isSpot) {
				float spotCos = dot(lightDirection,-Lights[light].coneDirection);
				if (spotCos < Lights[light].spotCosCutoff)
					attenuation = 0.0;
				else
					attenuation *= pow(spotCos, Lights[light].spotExponent);
			}
			halfVector = normalize(lightDirection + e);
		} else {
			lightDirection = normalize(lightDirection);
			halfVector = normalize(lightDirection + e);
		}
		float diffuse = max(0.0, dot(n, lightDirection));
		float specular = max(0.0, dot(n, halfVector));
		if (diffuse == 0.0)
			specular = 0.0;
		else
			specular = pow(specular, mat.shininess);
		// Accumulate all the lights’ effects
		if(texMode == 0){
			scatteredLight += Lights[light].color * mat.diffuse.rgb * diffuse * attenuation;
			reflectedLight += Lights[light].color * mat.specular.rgb * specular * attenuation;
			
		}else if(texMode == 2) {
			scatteredLight += Lights[light].color * texel.rgb * texel1.rgb * diffuse * attenuation;
			reflectedLight += Lights[light].color * mat.specular.rgb * specular * attenuation;
		}else if (texMode == 1) {
			scatteredLight += Lights[light].color * texel.rgb * diffuse * attenuation;
			reflectedLight += Lights[light].color * mat.specular.rgb * specular * attenuation;
		}
	}

	vec3 rgb = min(scatteredLight + reflectedLight, vec3(1.0));
	if(texMode == 0){
		colorOut = vec4(max(rgb, mat.ambient.rgb), mat.transparency);

	}else if(texMode == 2){
		colorOut = vec4(max(rgb,texel.rgb*texel1.rgb*0.1), mat.transparency);
		
	}else if (texMode == 1) {
		colorOut = vec4(max(rgb,texel.rgb*0.1), texel.a);
	}else if (texMode == 3){

		colorOut = vec4(texel.rgb*mat.diffuse.rgb, min(mat.diffuse.a, texel.a));
	}

	// FOG	
	if (enableFog) {
		float dist = length(DataIn.pos);
		float fogFactor = 0;

		//fogFactor = (200 - dist)/(200 - 50); // linear
		dist = dist - 35; // Fog start distance
		fogFactor = 1.0 / exp(dist * FogDensity); // exponential

		fogFactor = clamp(fogFactor, 0.0,1.0);
		//colorOut = vec4(fogColor * (1-fogFactor) + colorOut.xyz * fogFactor, mat.transparency);

		if(texMode == 0){
		colorOut = vec4(fogColor * (1-fogFactor) + colorOut.xyz * fogFactor, mat.transparency);;

		}else if(texMode == 2){
			colorOut = vec4(fogColor * (1-fogFactor) + colorOut.xyz * fogFactor, mat.transparency);
		
		}else if (texMode == 1 || texMode == 3) {
			colorOut = vec4(fogColor * (1-fogFactor) + colorOut.xyz * fogFactor, texel.a);
		}
	}
}