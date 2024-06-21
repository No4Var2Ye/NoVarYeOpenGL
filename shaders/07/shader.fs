
#version 450 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform float mixValue;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
    // FragColor = texture(texture1, TexCoord) * vec4(ourColor, 1.0);
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	// FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.1);
	FragColor = mix(texture(texture1, TexCoord), texture(texture2, vec2(1.0 -TexCoord.x, TexCoord.y)), mixValue);
}

// out vec4 FragColor;
// // in vec3 ourColor;
// in vec3 ourPosition;

// void main() {
//     FragColor = vec4(ourPosition, 1.0);    // note how the position value is linearly interpolated to get all the different colors
// }
