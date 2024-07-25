#version 430

in vec3 vs_out_pos;
in vec2 vs_out_tex;

out vec4 fs_out_col;

uniform vec3 eye;
uniform float aspect;

uniform mat4 inverseProjection;
uniform mat4 inverseView;

uniform vec4 lightPos;

uniform vec4 lightPos1;
uniform vec4 lightPos2;

uniform vec3 La = vec3(0.0, 0.0, 0.0 );
uniform vec3 Ld = vec3(1.0, 1.0, 1.0 );
uniform vec3 Ls = vec3(1.0, 1.0, 1.0 );

uniform float lightConstantAttenuation    = 1.0;
uniform float lightLinearAttenuation      = 0.0;
uniform float lightQuadraticAttenuation   = 0.0;

uniform vec3 Ka = vec3( 1.0 );
uniform vec3 Kd = vec3( 1.0 );
uniform vec3 Ks = vec3( 1.0 );

uniform float Shininess = 1.0;

uniform float threshold;
uniform vec4 metaBalls[4];

vec3 surfaceNormal;
vec3 cameraDirection;

const float DELTA_TIME = 0.001f;
const float MAX_DISTANCE = 100;
const float T_VALUE = 0.1;
const int MAX_BINARY_STEPS = 20;

float EvaluateMetaBallFunction(vec4 metaball, vec3 position)
{
	float d = distance(metaball.xyz, position) / metaball.w;

	if(d < 0.0)
	{
		return 1.0;
	}
	else if(d > 1.0)
	{
		return 0.0;
	}
	else
	{
		return 2 * pow(d, 3) - 3 * pow(d, 2) + 1.0;
	}
}

float EvaluateColorSpaceFunction(vec3 position)
{
	float t = 0;

	for(int i = 0; i < metaBalls.length(); ++i)
	{
		t += EvaluateMetaBallFunction(metaBalls[i], position);
	}
	t-= threshold;

	return t;
}

vec3 CalculateSurfaceNormal(vec3 position)
{
	float normalX = EvaluateColorSpaceFunction(vec3(position.x + DELTA_TIME, position.y, position.z)) -
					EvaluateColorSpaceFunction(vec3(position.x - DELTA_TIME, position.y, position.z));
	float normalY = EvaluateColorSpaceFunction(vec3(position.x, position.y + DELTA_TIME, position.z)) -
					EvaluateColorSpaceFunction(vec3(position.x, position.y - DELTA_TIME, position.z));
	float normalZ = EvaluateColorSpaceFunction(vec3(position.x, position.y, position.z + DELTA_TIME)) -
					EvaluateColorSpaceFunction(vec3(position.x, position.y, position.z - DELTA_TIME));
	vec3 normal = vec3(normalX,normalY,normalZ);
	return normalize(normal / (2 * DELTA_TIME));
}

void CalculateCameraCoordinateSystem()
{
	vec2 normalizedDeviceCoordinates = vs_out_tex * 2.0 - 1.0;
	normalizedDeviceCoordinates.x *= aspect;

	vec4 clipSpacePosition = vec4(normalizedDeviceCoordinates, -1.0, 1.0);
	vec4 viewSpacePosition = inverseProjection * clipSpacePosition;
	viewSpacePosition /= viewSpacePosition.w;

	vec4 worldSpacePosition = inverseView * viewSpacePosition;

	cameraDirection = normalize(worldSpacePosition.xyz - eye);
}

vec3 CalculateIllumination(vec4 lightPos)
{	
	vec3 ToLight;
	float LightDistance=0.0;
	
	if ( lightPos.w == 0.0 )
	{
		ToLight	= lightPos.xyz;
	}
	else
	{
		ToLight	= lightPos.xyz - vs_out_pos;
		LightDistance = length(ToLight);
	}
	ToLight = normalize(ToLight);
	
	float Attenuation = 1.0 / ( lightConstantAttenuation + lightLinearAttenuation * LightDistance + lightQuadraticAttenuation * LightDistance * LightDistance);
	
	vec3 Ambient = La * Ka;

	float DiffuseFactor = max(dot(ToLight,surfaceNormal), 0.0) * Attenuation;
	vec3 Diffuse = DiffuseFactor * Ld * Kd;
	
	vec3 viewDir = normalize( eye - vs_out_pos );
	vec3 halfDir = normalize(viewDir + ToLight);
	
	float SpecularFactor = pow(max( dot( surfaceNormal, halfDir) ,0.0), Shininess) * Attenuation;
	vec3 Specular = SpecularFactor * Ls * Ks;

	return Ambient + Diffuse + Specular;
}

vec3 RayMarch()
{
	int i = 0;
	float t = T_VALUE;
	float prevFunctionValue = EvaluateColorSpaceFunction(eye);
	float functionValue;
	bool intersect = false;

	while(!intersect && i < MAX_DISTANCE)
	{
		vec3 currentPoint = eye + t * cameraDirection;
		functionValue = EvaluateColorSpaceFunction(currentPoint);

		intersect = prevFunctionValue * functionValue < 0;

		prevFunctionValue = functionValue;
		t += T_VALUE;
		++i;
	}

	if(intersect)
	{
		float minT = t - T_VALUE;
		float maxT = t;

		for(int i = 0; i < MAX_BINARY_STEPS; ++i)
		{
			float midT = (minT + maxT) / 2.0;

			vec3 midPoint = eye + midT * cameraDirection;
			float midValue = EvaluateColorSpaceFunction(midPoint);

			if(prevFunctionValue * midValue < 0)
			{
				maxT = midT;
			}
			else
			{
				minT = midT;
				prevFunctionValue = midValue;
			}
		}

		t = (minT + maxT) / 2.0;
		return vec3(t);
	}
	
	return vec3(1,1,1);
}

void main()
{
	CalculateCameraCoordinateSystem();
	surfaceNormal = CalculateSurfaceNormal(vs_out_pos);

	vec3 lightIllumination = CalculateIllumination(lightPos);

	vec3 lightIllumination1 = CalculateIllumination(lightPos1);
	vec3 lightIllumination2 = CalculateIllumination(lightPos2);
	
	vec3 grayScaleColor = RayMarch();

	fs_out_col = vec4((lightIllumination1 + lightIllumination2) * grayScaleColor, 1 );
}