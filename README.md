# MY OPENGL ENGINE!
Here been my project OpenGL! It's don't my first project, it's just my repeat about OpenGL, and I'm tried to create(how is possibly) greate the engine on OpenGL! The repository is been update every day, why i don't finish this project! Lets start!

## 20.02.25
There is just simple check my triangle! And I'm decided make the font rendering(Signed distance field font rendering), and to be honest, I wanted to do this for a very long time, but I didn’t have enough experience to implement it, so I decided to do it now (for those who don’t know, I will describe everything in detail so that everyone understands exactly what I want to achieve).
### Result.
![gif](https://github.com/tornado4444/OpenGL/blob/main/Game/murder.gif)

How we can see, there are fragment and vertex shaders on my engine. At the moment shaders are implemented for triangle and text(signed text distance field font rendering). 
```c++
textShader.frag
#version 460 core

out vec4 color;

in vec2 TexCoords;

uniform sampler2D image;
uniform vec3 Color;

void main(){
	vec4 sampled = vec4(1.0f,1.0f,1.0f, texture(image,TexCoords).r);
	color = vec4(Color, 1.0f) * sampled;
}
```
This is the main section of code that defines for the fragment shader to set the texture and a specific color for the text so that it can be rendered, also not as an object (for example, when we want to move the camera away, we see how the text disappears, and is rendered as a pyramid), but namely the specified shader.

```c++
#version 460 core

layout (location = 0) in vec4 vertex;
out vec2 TexCoords;

uniform mat4 projection;

void main() {
	gl_Position = projection * vec4(vertex.xy, 0.0f,1.0f);
	TexCoords = vertex.zw;
}
```
It`s the vertex shader, where we're show, what depth do we want to create.

The main code on Font.cpp
```c++
void Font::loadTextures() {
	string baseFolder = myFilename.substr(0, myFilename.find_last_of('/', myFilename.length()) + 1);

	for (int i = 0; i < Pages.size(); i++) {
		string filename = baseFolder + Pages[i].filename;

		GLenum err = glGetError();
		glGenTextures(1, &Pages[i].textId);
		if (err != GL_NO_ERROR) {
			MyglobalLogger().logMessage(Logger::ERROR, "OpenGL error after glGenTextures: " + std::to_string(err), __FILE__, __LINE__);
		}

		glBindTexture(GL_TEXTURE_2D, Pages[i].textId);

		if (err != GL_NO_ERROR) {
			MyglobalLogger().logMessage(Logger::ERROR, "OpenGL error after glGenTextures: " + std::to_string(err), __FILE__, __LINE__);
		}

		// Parametrs images for rendering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, channels;
		unsigned char* image = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (!image) {
			MyglobalLogger().logMessage(Logger::ERROR, "Failed to load texture: " + filename, __FILE__, __LINE__);
			continue;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		stbi_image_free(image);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	MyglobalLogger().logMessage(Logger::DEBUG, "Loading font texture from: " + myFilename, __FILE__, __LINE__);
}

```
Here is the here we specifically enter and set parameters for the background so that everything will be rendered normally.
## 02.03.25
The next, I decided to implement a camera, of course, and give textures to my pyramide. At the moment, only basic pyramid textures and custom shaders have been implemented. 
```c++
default.frag

#version 460 core

out vec4 FragColor;

in vec3 color;
in vec2 texCoord;
uniform sampler2D tex0;

void main()
{
	FragColor = texture(tex0, texCoord) * vec4(color, 1.0);
}
```
Here a fragment shader is specifically implemented, which tells us what exactly we will set in the buffer, what the color will be, and we can also set the texture.

```c++
default.vert

#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTex;

uniform mat4 model;
uniform mat4 camMatrix;

out vec3 color;
out vec2 texCoord;

void main()
{
	gl_Position = camMatrix * model * vec4(aPos, 1.0);
	color = aColor;
	texCoord = aTex;
}
```
The main feature of the vertex shader is to create uniform variables so that our pyramid is rendered correctly. Also, the most important thing is to set gl_Position correctly so that there are no errors in the logger.

### Result.
![gif](https://github.com/tornado4444/OpenGL/blob/main/Game/had.gif)

23.03.25
# TODO
- [X] Signed Distance Field Font Rendering.
- [X] Fixing camera and textures.
- [ ] Models.
- [ ] Guizmo.
- [ ] AO(Ambient Occlusion).
- [ ] Animation.
- [ ] PBR.
- [ ] Volumetric Clouds.
- [ ] BVH.

# Operating Systems.
- [X] Windows.
- [ ] Linux.
