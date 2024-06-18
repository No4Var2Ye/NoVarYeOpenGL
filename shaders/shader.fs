
#version 450 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;

void main()
{
	FragColor = texture(texture1, TexCoord);
}

// out vec4 FragColor;
// // in vec3 ourColor;
// in vec3 ourPosition;

// void main() {
//     FragColor = vec4(ourPosition, 1.0);    // note how the position value is linearly interpolated to get all the different colors
// }
