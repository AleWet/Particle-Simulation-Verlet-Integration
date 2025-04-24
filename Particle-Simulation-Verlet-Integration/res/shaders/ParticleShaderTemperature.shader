#shader vertex
#version 330 core

// Quad vertex attributes
layout(location = 0) in vec2 a_Position;     // Quad vertex positions
layout(location = 1) in vec2 a_TexCoord;     // Texture coordinates

// Instance attributes
layout(location = 2) in vec2 a_ParticlePos;  // Particle center position
layout(location = 3) in float a_Temperature; // Particle temperature
layout(location = 4) in float a_Size;        // Particle size

// Outputs to fragment shader
out vec2 v_TexCoord;
out float v_Temperature;

uniform mat4 u_MVP;

void main()
{
    // Calculate the position of this vertex
    // a_Position is in [-1,1] range, scale by particle size and add to particle position
    vec2 vertexPos = a_ParticlePos + a_Position * a_Size;
    
    // Transform vertex to clip space
    gl_Position = u_MVP * vec4(vertexPos, 0.0, 1.0);
    
    // Pass texture coordinates to fragment shader
    v_TexCoord = a_TexCoord;
    
    // Pass temperature to fragment shader
    v_Temperature = a_Temperature;
}

#shader fragment
#version 330 core

in vec2 v_TexCoord;
in float v_Temperature;  
out vec4 FragColor;

void main()
{
    // Calculate distance from center (0.5, 0.5) in texture space
    vec2 center = vec2(0.5, 0.5);
    float distance = length(v_TexCoord - center) * 2.0; // *2 to normalize to [0,1] range
    
    // Create a soft circle shape with smooth edges
    float circleShape = 1.0 - smoothstep(0.9, 1.0, distance);

    // max temperature per particle = 400

    // Temperature thresholds
    float minTemp = 0.0;        // Cold (black)
    float lowTemp = 50.0;       // Starting temperature (red)
    float medTemp = 175.0;      // Medium temperature (orange)
    float highTemp = 300.0;     // High temperature (yellow)
    float veryHighTemp = 400.0; // Very high temperature (white)
    
    vec3 colorRGB;
    
    if (v_Temperature <= minTemp) {
        // Black for no heat or negative temperature (it shouldn't be but still why not)
        colorRGB = vec3(0.0, 0.0, 0.0);
    }
    else if (v_Temperature < lowTemp) {
        float t = (v_Temperature - minTemp) / (lowTemp - minTemp);
        colorRGB = vec3(t, 0.0, 0.0);
    }
    else if (v_Temperature < medTemp) {
        float t = (v_Temperature - lowTemp) / (medTemp - lowTemp);
        // Orange is roughly (1.0, 0.5, 0.0)
        colorRGB = mix(vec3(1.0, 0.0, 0.0), vec3(1.0, 0.5, 0.0), t);
    }
    else if (v_Temperature < highTemp) {
        float t = (v_Temperature - medTemp) / (highTemp - medTemp);
        colorRGB = mix(vec3(1.0, 0.5, 0.0), vec3(1.0, 1.0, 0.0), t);
    }
    else {
        float t = min((v_Temperature - highTemp) / (veryHighTemp - highTemp), 1.0);
        colorRGB = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), t);
    }
    
    // Create the final color with alpha from the circle 
    vec4 finalColor = vec4(colorRGB, circleShape);
    
    // Discard pixels outside the circle for clean edge
    if (circleShape < 0.1) discard;
    
    FragColor = finalColor;
}